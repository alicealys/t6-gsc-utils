#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"

#include "scheduler.hpp"
#include "scripting.hpp"
#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace gsc
{
    namespace
    {
        std::unordered_map<std::string, game::BuiltinMethodDef> registered_methods;
        std::unordered_map<std::string, game::BuiltinFunctionDef> registered_functions;

        std::unordered_map<unsigned int, std::pair<std::string, script_function>> functions;
        std::unordered_map<unsigned int, std::pair<std::string, script_method>> methods;

        utils::hook::detour scr_get_common_function_hook;
        utils::hook::detour player_get_method_hook;

        auto field_offset_start = 0xA000;

        struct entity_field
        {
            std::string name;
            std::function<scripting::script_value(unsigned int entnum)> getter;
            std::function<void(unsigned int entnum, scripting::script_value)> setter;
        };

        std::vector<std::function<void()>> post_load_callbacks;
        std::unordered_map<unsigned int, std::unordered_map<unsigned int, entity_field>> custom_fields;

        unsigned int find_function(const std::string& name)
        {
            for (const auto& function : functions)
            {
                if (function.second.first == name)
                {
                    return function.first;
                }
            }

            return 0;
        }

        unsigned int find_method(const std::string& name)
        {
            for (const auto& method : methods)
            {
                if (method.second.first == name)
                {
                    return method.first;
                }
            }

            return 0;
        }

        builtin_function scr_get_common_function(const char** pName, int* type, int* min_args, int* max_args)
        {
            const auto func = reinterpret_cast<builtin_function>(find_function(*pName));

            if (func == nullptr)
            {
                return scr_get_common_function_hook.invoke<builtin_function>(pName, type, min_args, max_args);
            }

            *type = 1;
            *min_args = 0;
            *max_args = 512;

            return func;
        }

        builtin_method player_get_method(const char** pName, int* min_args, int* max_args)
        {
            const auto method = reinterpret_cast<builtin_method>(find_method(*pName));

            if (method == nullptr)
            {
                return player_get_method_hook.invoke<builtin_method>(pName, min_args, max_args);
            }
            
            *min_args = 0;
            *max_args = 512;

            return method;
        }

        bool call_function(unsigned int ptr)
        {
            if (functions.find(ptr) == functions.end())
            {
                return false;
            }

            const auto function = functions[ptr];

            try
            {
                const auto result = function.second(get_arguments());
                return_value(result);
            }
            catch (const std::exception& e)
            {
                printf("************** Script execution error **************\n");
                printf("Error executing function '%s':\n", function.first.data());
                printf("    %s\n", e.what());
                printf("****************************************************\n");
            }

            return true;
        }

        bool call_method(unsigned int ptr, game::scr_entref_t entref)
        {
            if (methods.find(ptr) == methods.end())
            {
                return false;
            }

            const auto method = methods[ptr];

            try
            {
                const auto result = method.second(entref, get_arguments());
                return_value(result);

            }
            catch (const std::exception& e)
            {
                printf("************** Script execution error **************\n");
                printf("Error executing method '%s':\n", method.first.data());
                printf("    %s\n", e.what());
                printf("****************************************************\n");
            }

            return true;
        }

        __declspec(naked) void call_builtin_stub_mp()
        {
            __asm
            {
                pushad
                push ecx
                call call_function

                cmp al, 0
                jne loc_1

                pop ecx
                call ecx
                popad

                push 0x8F6498
                retn
            loc_1:
                pop ecx
                popad

                push 0x8F6498
                retn
            }
        }

        __declspec(naked) void call_builtin_stub_zm()
        {
            __asm
            {
                pushad
                push ecx
                call call_function

                cmp al, 0
                jne loc_1

                pop ecx
                call ecx
                popad

                push 0x8F51F8
                retn
            loc_1:
                pop ecx
                popad

                push 0x8F51F8
                retn
            }
        }

        __declspec(naked) void call_builtin_method_stub_mp()
        {
            __asm
            {
                pushad
                push esi
                push [ebp + 0x2C]
                call call_method

                cmp al, 0
                jne loc_1

                add esp, 0x8
                popad

                call[ebp + 0x2C]

                jmp end
            loc_1:
                add esp, 0x8
                popad

                jmp end
            end:
                add esp, 0x18

                push 0x8F6498
                retn
            }
        }

        __declspec(naked) void call_builtin_method_stub_zm()
        {
            __asm
            {
                pushad
                push esi
                push[ebp + 0x2C]
                call call_method

                cmp al, 0
                jne loc_1

                add esp, 0x8
                popad

                call[ebp + 0x2C]

                jmp end
            loc_1:
                add esp, 0x8
                popad

                jmp end
            end:
                add esp, 0x18

                push 0x8F51F8
                retn
            }
        }

        utils::hook::detour scr_get_object_field_hook;
        void scr_get_object_field_stub(unsigned int classnum, int entnum, unsigned int offset)
        {
            if (custom_fields[classnum].find(offset) == custom_fields[classnum].end())
            {
                return scr_get_object_field_hook.invoke<void>(classnum, entnum, offset);
            }

            const auto field = custom_fields[classnum][offset];

            try
            {
                const auto result = field.getter(entnum);
                return_value(result);
            }
            catch (const std::exception& e)
            {
                printf("************** Script execution error **************\n");
                printf("Error getting field '%s':\n", field.name.data());
                printf("    %s\n", e.what());
                printf("****************************************************\n");
            }
        }

        utils::hook::detour scr_set_object_field_hook;
        void scr_set_object_field_stub(unsigned int classnum, int entnum, unsigned int offset)
        {
            if (custom_fields[classnum].find(offset) == custom_fields[classnum].end())
            {
                return scr_set_object_field_hook.invoke<void>(classnum, entnum, offset);
            }

            const auto args = get_arguments();
            const auto field = custom_fields[classnum][offset];

            try
            {
                field.setter(entnum, args[0]);
            }
            catch (const std::exception& e)
            {
                printf("************** Script execution error **************\n");
                printf("Error setting field '%s':\n", field.name.data());
                printf("    %s\n", e.what());
                printf("****************************************************\n");
            }
        }

        utils::hook::detour scr_post_load_scripts_hook;
        void scr_post_load_scripts_stub()
        {
            for (const auto& callback : post_load_callbacks)
            {
                callback();
            }

            return scr_post_load_scripts_hook.invoke<void>();
        }
    }

    namespace function
    {
        void add(const std::string& name, const script_function& function)
        {
            const auto id = functions.size() + 1;
            functions[id] = std::make_pair(name, function);
        }
    }

    namespace method
    {
        void add(const std::string& name, const script_method& function)
        {
            const auto id = methods.size() + 1;
            methods[id] = std::make_pair(name, function);
        }
    }

    namespace field
    {
        void add(const classid classnum, const std::string& name,
            const std::function<scripting::script_value(unsigned int entnum)>& getter,
            const std::function<void(unsigned int entnum, const scripting::script_value&)>& setter)
        {
            const auto offset = field_offset_start++;
            custom_fields[classnum][offset] = {name, getter, setter};

            post_load_callbacks.push_back([classnum, name, offset]()
            {
                game::Scr_AddClassField(game::SCRIPTINSTANCE_SERVER, classnum, name.data(), offset);
            });
        }
    }

    void return_value(const scripting::script_value& value)
    {
        if (game::scr_VmPub->outparamcount)
        {
            game::Scr_ClearOutParams(game::SCRIPTINSTANCE_SERVER);
        }

        scripting::push_value(value);
    }

    std::vector<scripting::script_value> get_arguments()
    {
        std::vector<scripting::script_value> args;

        const auto top = game::scr_VmPub->top;

        for (auto i = 0; i < game::scr_VmPub->outparamcount; i++)
        {
            const auto value = game::scr_VmPub->top[-i];
            args.push_back(value);
        }

        return args;
    }

    function_args::function_args(std::vector<scripting::script_value> values)
        : values_(values)
    {
    }

    unsigned int function_args::size() const
    {
        return this->values_.size();
    }

    std::vector<scripting::script_value> function_args::get_raw() const
    {
        return this->values_;
    }

    scripting::value_wrap function_args::get(const int index) const
    {
        if (index >= this->values_.size())
        {
            throw std::runtime_error(utils::string::va("parameter %d does not exist", index));
        }

        return {this->values_[index], index};
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            scr_get_common_function_hook.create(SELECT(0x691110, 0x4EB070), scr_get_common_function);
            player_get_method_hook.create(SELECT(0x432480, 0x6F2DB0), player_get_method);

            utils::hook::jump(SELECT(0x8F63FF, 0x8F515F), SELECT(call_builtin_stub_mp, call_builtin_stub_zm));
            utils::hook::jump(SELECT(0x8F6492, 0x8F51F2), SELECT(call_builtin_method_stub_mp, call_builtin_method_stub_zm));

            scr_get_object_field_hook.create(SELECT(0x573160, 0x4224E0), scr_get_object_field_stub);
            scr_set_object_field_hook.create(SELECT(0x5B9820, 0x43F2A0), scr_set_object_field_stub);
            scr_post_load_scripts_hook.create(SELECT(0x6B75B0, 0x492440), scr_post_load_scripts_stub);

            field::add(classid::entity, "flags",
                [](unsigned int entnum) -> scripting::script_value
                {
                    const auto entity = &game::g_entities[entnum];
                    return entity->client->eflags;
                },
                [](unsigned int entnum, const scripting::script_value& value)
                {
                    const auto entity = &game::g_entities[entnum];
                    entity->client->eflags = value.as<int>();
                }
            );

            field::add(classid::entity, "clientflags",
                [](unsigned int entnum) -> scripting::script_value
                {
                    const auto entity = &game::g_entities[entnum];
                    return entity->client->flags;
                },
                [](unsigned int entnum, const scripting::script_value& value)
                {
                    const auto entity = &game::g_entities[entnum];
                    entity->client->flags = value.as<int>();
                }
            );

            function::add("getfunction", [](const function_args&) -> scripting::script_value
            {
                const auto filename = game::get<const char*>(0);
                const auto function = game::get<const char*>(1);
                const auto handle = game::Scr_GetFunctionHandle(game::SCRIPTINSTANCE_SERVER, filename, function, 0, 0);

                game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, handle);
                const auto stack = *reinterpret_cast<int**>(SELECT(0x2E1A5E0, 0x2DEA8E0));
                *stack = 10;

                return {};
            });

            function::add("meow", [](const function_args& args) -> scripting::script_value
            {
                printf("meow\n");
                return {};
            });

            function::add("arrayremovekey", [](const function_args& args) -> scripting::script_value
            {
                const auto array = args[0].as<scripting::array>();
                const auto key = args[1].as<std::string>();
                array.erase(key);
            });

            function::add("xor", [](const function_args& args)
            {
                const auto a = args[0].as<int>();
                const auto b = args[1].as<int>();
                return a ^ b;
            });

            function::add("not", [](const function_args& args)
            {
                const auto a = args[0].as<int>();
                return ~a;
            });

            function::add("and", [](const function_args& args)
            {
                const auto a = args[0].as<int>();
                const auto b = args[1].as<int>();
                return a & b;
            });

            function::add("or", [](const function_args& args)
            {
                const auto a = args[0].as<int>();
                const auto b = args[1].as<int>();
                return a | b;
            });

            function::add("structget", [](const function_args& args)
            {
                const auto obj = args[0].as<scripting::object>();
                const auto key = args[1].as<std::string>();
                return obj[key];
            });

            function::add("structset", [](const function_args& args) -> scripting::script_value
            {
                const auto obj = args[0].as<scripting::object>();
                const auto key = args[1].as<std::string>();
                obj[key] = args[2];
                return {};
            });

            function::add("structremove", [](const function_args& args) -> scripting::script_value
            {
                const auto obj = args[0].as<scripting::object>();
                const auto key = args[1].as<std::string>();
                obj.erase(key);
                return {};
            });

            function::add("_print", [](const function_args& args) -> scripting::script_value
            {
                const auto args_ = args.get_raw();

                for (const auto& arg : args_)
                {
                    const auto str = arg.as<std::string>();
                    printf("%s\t", str.data());
                }

                printf("\n");

                return {};
            });
        }
    };
}

REGISTER_COMPONENT(gsc::component)
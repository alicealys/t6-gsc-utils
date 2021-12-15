#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"
#include "json.hpp"
#include "debug.hpp"

#include "scheduler.hpp"
#include "scripting.hpp"
#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace gsc
{
    namespace
    {
        std::unordered_map<std::string, std::pair<script_function, builtin_function>> functions;
        std::unordered_map<std::string, std::pair<script_method, builtin_method>> methods;

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

        builtin_function find_function(const std::string& name)
        {
            for (const auto& function : functions)
            {
                if (function.first == name)
                {
                    return function.second.second;
                }
            }

            return 0;
        }

        builtin_method find_method(const std::string& name)
        {
            for (const auto& method : methods)
            {
                if (method.first == name)
                {
                    return method.second.second;
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

        bool call_function(const char* name)
        {
            if (functions.find(name) == functions.end())
            {
                return false;
            }

            const auto function = functions[name];

            try
            {
                const auto result = function.first(get_arguments());
                return_value(result);
            }
            catch (const std::exception& e)
            {
                printf("******* script runtime error *******\n");
                printf("in call to builtin function \"%s\": %s\n", name, e.what());
                printf(debug::get_call_stack().data());
                printf("************************************\n");
            }

            return true;
        }

        bool call_method(const char* name, game::scr_entref_t entref)
        {
            if (methods.find(name) == methods.end())
            {
                return false;
            }

            const auto method = methods[name];

            try
            {
                const scripting::entity entity = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, entref.entnum, entref.classnum, 0);
                const auto result = method.first(entity, get_arguments());
                return_value(result);

            }
            catch (const std::exception& e)
            {
                printf("******* script runtime error *******\n");
                printf("in call to builtin method \"%s\": %s\n", name, e.what());
                printf(debug::get_call_stack().data());
                printf("************************************\n");
            }

            return true;
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
                printf("******* script runtime error *******\n");
                printf("while getting field \"%s\": %s\n", field.name.data(), e.what());
                printf(debug::get_call_stack().data());
                printf("************************************\n");
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
                printf("******* script runtime error *******\n");
                printf("while setting field \"%s\": %s\n", field.name.data(), e.what());
                printf(debug::get_call_stack().data());
                printf("************************************\n");
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
            const auto string = utils::memory::duplicate_string(name);
            const auto ptr = reinterpret_cast<builtin_function>(
                utils::hook::assemble([string](utils::hook::assembler& a)
                {
                    a.pushad();
                    a.push(string);
                    a.call(call_function);
                    a.add(esp, 0x4);
                    a.popad();

                    a.ret();
                })
            );

            functions[name] = std::make_pair(function, ptr);
        }
    }

    namespace method
    {
        void add(const std::string& name, const script_method& function)
        {
            const auto string = utils::memory::duplicate_string(name);
            const auto ptr = reinterpret_cast<builtin_method>(
                utils::hook::assemble([string](utils::hook::assembler& a)
                {
                    a.pushad();
                    a.push(dword_ptr(esp, 0x24));
                    a.push(string);
                    a.call(call_method);
                    a.add(esp, 0x8);
                    a.popad();

                    a.ret();
                })
            );

            methods[name] = std::make_pair(function, ptr);
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

    std::string find_builtin_name(void* function)
    {
        for (auto i = scripting::function_map.begin(); i != scripting::function_map.end(); ++i)
        {
            if (i->second.actionFunc == function)
            {
                return i->second.actionString;
            }
        }

        for (auto i = functions.begin(); i != functions.end(); ++i)
        {
            if (i->second.second == function)
            {
                return i->first;
            }
        }

        return {};
    }

    std::string find_builtin_method_name(void* function)
    {
        for (auto i = scripting::method_map.begin(); i != scripting::method_map.end(); ++i)
        {
            if (i->second.actionFunc == function)
            {
                return i->second.actionString;
            }
        }

        for (auto i = methods.begin(); i != methods.end(); ++i)
        {
            if (i->second.second == function)
            {
                return i->first;
            }
        }

        return {};
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            scr_get_common_function_hook.create(SELECT(0x4B57B0, 0x4AD040), scr_get_common_function);
            player_get_method_hook.create(SELECT(0x432480, 0x6F2DB0), player_get_method);

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

            function::add("getfunction", [](const function_args& args) -> scripting::script_value
            {
                const auto filename = args[0].as<std::string>();
                const auto function = args[1].as<std::string>();

                if (scripting::script_function_table[filename].find(function) != scripting::script_function_table[filename].end())
                {
                    return scripting::function{scripting::script_function_table[filename][function]};
                }

                return {};
            });

            function::add("getfunctionname", [](const function_args& args)
            {
                const auto function = args[0].as<scripting::function>();
                return function.get_name();
            });

            function::add("arrayremovekey", [](const function_args& args) -> scripting::script_value
            {
                const auto array = args[0].as<scripting::array>();
                const auto key = args[1].as<std::string>();
                array.erase(key);
                return {};
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

            function::add("getstructkeys", [](const function_args& args) -> scripting::script_value
            {
                const auto obj = args[0].as<scripting::object>();
                const auto keys = obj.get_keys();
                scripting::array result;

                for (const auto& key : keys)
                {
                    result.push(key);
                }

                return result;
            });

            function::add("isfunctionptr", [](const function_args& args) -> scripting::script_value
            {
                return args[0].is<scripting::function>();
            });

            function::add("isentity", [](const function_args& args) -> scripting::script_value
            {
                const auto value = args[0].get_raw();
                const auto type = game::scr_VarGlob->objectVariableValue[value.u.uintValue].w.type & 0x7F;
                return value.type == game::SCRIPT_OBJECT && type == game::SCRIPT_ENTITY;
            });

            function::add("isstruct", [](const function_args& args)
            {
                return args[0].is<scripting::object>();
            });

            function::add("typeof", [](const function_args& args)
            {
                return args[0].type_name();
            });

            method::add("get", [](const scripting::entity& entity, const function_args& args)
            {
                const auto field = args[0].as<std::string>();
                return entity.get(field);
            });

            method::add("set", [](const scripting::entity& entity, const function_args& args) -> scripting::script_value
            {
                const auto field = args[0].as<std::string>();
                entity.set(field, args[1]);
                return {};
            });
        }
    };
}

REGISTER_COMPONENT(gsc::component)
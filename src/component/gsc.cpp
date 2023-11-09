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
    std::unordered_map<std::string, function_t> functions;
    std::unordered_map<std::string, function_t> methods;

    std::unordered_map<std::string, void*> function_wraps;
    std::unordered_map<std::string, void*> method_wraps;

    namespace
    {
        utils::hook::detour scr_get_common_function_hook;
        utils::hook::detour player_get_method_hook;

        auto field_offset_start = 0xA000;

        std::string current_namespace;

        struct entity_field
        {
            std::string name;
            std::function<scripting::script_value(unsigned int entnum)> getter;
            std::function<void(unsigned int entnum, scripting::script_value)> setter;
        };

        std::vector<std::function<void()>> post_load_callbacks;
        std::unordered_map<unsigned int, std::unordered_map<unsigned int, entity_field>> custom_fields;

        std::unordered_map<std::string, scripting::script_value> world;

        std::unordered_map<void*, const char*> hooked_builtins;

        utils::hook::detour scr_error_internal_hook;

        std::atomic_bool disable_longjmp_error = false;

        std::string get_full_name(const std::string& name)
        {
            if (current_namespace.empty())
            {
                return name;
            }

            return std::format("{}::{}", current_namespace, name);
        }

        void* find_function(const std::string& name)
        {
            const auto full_name = get_full_name(name);
            const auto iter = function_wraps.find(full_name);
            if (iter == function_wraps.end())
            {
                return nullptr;
            }

            return iter->second;
        }

        void* find_method(const std::string& name)
        {
            const auto full_name = get_full_name(name);
            const auto iter = method_wraps.find(full_name);
            if (iter == method_wraps.end())
            {
                return nullptr;
            }

            return iter->second;
        }

        builtin_function scr_get_common_function(const char** name, int* type, int* min_args, int* max_args)
        {
            const auto func = reinterpret_cast<builtin_function>(find_function(*name));

            if (func == nullptr)
            {
                return scr_get_common_function_hook.invoke<builtin_function>(name, type, min_args, max_args);
            }

            *type = 1;
            *min_args = 0;
            *max_args = 512;

            return func;
        }

        builtin_method player_get_method(const char** name, int* min_args, int* max_args)
        {
            const auto method = reinterpret_cast<builtin_method>(find_method(*name));

            if (method == nullptr)
            {
                return player_get_method_hook.invoke<builtin_method>(name, min_args, max_args);
            }

            *min_args = 0;
            *max_args = 512;

            return method;
        }

        utils::hook::detour scr_get_object_field_hook;
        void scr_get_object_field_stub(unsigned int classnum, int entnum, unsigned int offset)
        {
            const auto class_iter = custom_fields.find(classnum);
            if (class_iter == custom_fields.end())
            {
                return scr_get_object_field_hook.invoke<void>(classnum, entnum, offset);
            }

            const auto iter = class_iter->second.find(offset);
            if (iter == class_iter->second.end())
            {
                return scr_get_object_field_hook.invoke<void>(classnum, entnum, offset);
            }

            const auto& field = iter->second;

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
            const auto class_iter = custom_fields.find(classnum);
            if (class_iter == custom_fields.end())
            {
                return scr_set_object_field_hook.invoke<void>(classnum, entnum, offset);
            }

            const auto iter = class_iter->second.find(offset);
            if (iter == class_iter->second.end())
            {
                return scr_set_object_field_hook.invoke<void>(classnum, entnum, offset);
            }

            const auto args = get_arguments();
            const auto& field = iter->second;

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

        void scr_error_internal_stub()
        {
            if (disable_longjmp_error)
            {
                throw;
            }
            else
            {
                scr_error_internal_hook.invoke<void>();
            }
        }

        __declspec(naked) void scr_error_internal_stub_1_mp()
        {
            __asm
            {
                mov eax, 0
                mov edx, eax
                imul edx, 0x54

                push 0x8F3F65
                retn
            }
        }

        __declspec(naked) void scr_error_internal_stub_1_zm()
        {
            __asm
            {
                mov eax, 0
                mov edx, eax
                imul edx, 0x54

                push 0x8F2CC5
                retn
            }
        }

        void gsc_obj_resolve_stub_1(game::GSC_OBJ* obj, game::GSC_IMPORT_ITEM* item)
        {
            const auto name = &obj->magic[item->name];
            current_namespace = &obj->magic[item->name_space];

            if (current_namespace[0] == 0)
            {
                return;
            }

            const auto is_function = find_function(name);
            const auto is_method = !is_function && find_method(name);

            if (is_function || is_method)
            {
                item->name_space += static_cast<unsigned short>(current_namespace.size());
            }
        }

        __declspec(naked) void gsc_obj_resolve_stub_mp()
        {
            __asm
            {
                pushad
                push edi
                push esi
                call gsc_obj_resolve_stub_1
                pop esi
                pop edi
                popad

                movzx eax, byte ptr[edi + 7]
                and eax, 0xF
                dec eax

                push 0x6CD7F1
                ret
            }
        }

        __declspec(naked) void gsc_obj_resolve_stub_zm()
        {
            __asm
            {
                pushad
                push edi
                push esi
                call gsc_obj_resolve_stub_1
                pop esi
                pop edi
                popad

                movzx eax, byte ptr[edi + 7]
                and eax, 0xF
                dec eax

                push 0x4F0441
                ret
            }
        }
    }

    bool execute_hook(const void* ptr)
    {
        const auto iter = hooked_builtins.find(reinterpret_cast<void*>(
            reinterpret_cast<size_t>(ptr)));
        if (iter == hooked_builtins.end())
        {
            return false;
        }

        scripting::function function(iter->second);
        function.call(*game::levelEntityId, get_arguments());

        return true;
    }

    void call_function(const function_t& function, const std::string& name)
    {
        const auto args = get_arguments();

        try
        {
            const auto value = function(args);
            return_value(value);
        }
        catch (const std::exception& e)
        {
            printf("******* script runtime error *******\n");
            printf("in call to builtin function \"%s\": %s\n", name.data(), e.what());
            printf(debug::get_call_stack().data());
            printf("************************************\n");
        }
    }

    void call_method(const function_t& method, const std::string& name, const game::scr_entref_t entref)
    {
        const auto args = get_arguments();

        try
        {
            const scripting::entity entity = game::Scr_GetEntityId(
                game::SCRIPTINSTANCE_SERVER, entref.entnum, entref.classnum, 0);

            std::vector<scripting::script_value> args_{};
            args_.push_back(entity);
            for (const auto& arg : args)
            {
                args_.push_back(arg);
            }

            const auto value = method(args_);
            return_value(value);
        }
        catch (const std::exception& e)
        {
            printf("******* script runtime error *******\n");
            printf("in call to builtin method \"%s\": %s\n", name.data(), e.what());
            printf(debug::get_call_stack().data());
            printf("************************************\n");
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

        for (auto i = 0; static_cast<unsigned int>(i) < game::scr_VmPub->outparamcount; i++)
        {
            const auto value = game::scr_VmPub->top[-i];
            args.push_back(value);
        }

        return args;
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

        for (auto i = function_wraps.begin(); i != function_wraps.end(); ++i)
        {
            if (i->second == function)
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

        for (auto i = method_wraps.begin(); i != method_wraps.end(); ++i)
        {
            if (i->second == function)
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
            utils::hook::jump(SELECT(0x6CD7E9, 0x4F0439), SELECT(gsc_obj_resolve_stub_mp, gsc_obj_resolve_stub_zm));

            scr_get_common_function_hook.create(SELECT(0x4B57B0, 0x4AD040), scr_get_common_function);
            player_get_method_hook.create(SELECT(0x432480, 0x6F2DB0), player_get_method);

            scr_get_object_field_hook.create(SELECT(0x573160, 0x4224E0), scr_get_object_field_stub);
            scr_set_object_field_hook.create(SELECT(0x5B9820, 0x43F2A0), scr_set_object_field_stub);
            scr_post_load_scripts_hook.create(SELECT(0x6B75B0, 0x492440), scr_post_load_scripts_stub);

            utils::hook::jump(SELECT(0x8F3F60, 0x8F2CC0), SELECT(scr_error_internal_stub_1_mp, scr_error_internal_stub_1_zm));
            scr_error_internal_hook.create(SELECT(0x8F3F60, 0x8F2CC0), scr_error_internal_stub);

            scripting::on_shutdown([&]
            {
                hooked_builtins.clear();
            });

            field::add(classid::entity, "eflags",
                [](unsigned int entnum) -> scripting::script_value
                {
                    const auto entity = &game::g_entities[entnum];
                    return entity->flags;
                },
                [](unsigned int entnum, const scripting::script_value& value)
                {
                    const auto entity = &game::g_entities[entnum];
                    entity->flags = value.as<int>();
                }
            );

            field::add(classid::entity, "eflags2",
                [](unsigned int entnum) -> scripting::script_value
                {
                    const auto entity = &game::g_entities[entnum];
                    return entity->eFlags2;
                },
                    [](unsigned int entnum, const scripting::script_value& value)
                {
                    const auto entity = &game::g_entities[entnum];
                    entity->eFlags2 = value.as<int>();
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

            field::add(classid::entity, "address",
                [](unsigned int entnum) -> scripting::script_value
                {
                    if (entnum >= 18)
                    {
                        throw std::runtime_error("Not a player entity");
                    }

                    const auto clients = *game::svs_clients;
                    const auto client = &clients[entnum];
                    const auto address = client->header.netchan.remoteAddress.ip;

                    const std::string address_str = utils::string::va("%i.%i.%i.%i",
                        address[0],
                        address[1],
                        address[2],
                        address[3]
                    );

                    return address_str;
                },
                [](unsigned int, const scripting::script_value&)
                {
                    throw std::runtime_error("cannot set address");
                }
            );

            function::add("getfunction", [](const std::string& filename, const std::string& function)
                -> scripting::script_value
            {
                const auto file_iter = scripting::script_function_table.find(filename);
                if (file_iter == scripting::script_function_table.end())
                {
                    throw std::runtime_error(std::format("script {} not found", filename));
                }

                const auto iter = file_iter->second.find(function);
                if (iter == file_iter->second.end())
                {
                    throw std::runtime_error(std::format("function {} not found in script", function, filename));
                }

                return scripting::function{iter->second};
            });

            function::add("getfunctionname", [](const scripting::variadic_args& args)
            {
                const auto function = args[0].as<scripting::function>();
                return function.get_name();
            });
            
            function::add("getfunctionargcount", [](const scripting::function& function)
            {
                const auto pos = function.get_pos();
                if (*pos != 0x17) // OP_SafeCreateLocalVariables
                {
                    return 0;
                }

                return static_cast<int>(pos[1]);
            });

            function::add("arrayremovekey", [](const scripting::array& array, const std::string& key)
            {
                array.erase(key);
            });

            function::add("xor", [](const int a, const int b)
            {
                return a ^ b;
            });

            function::add("not", [](const int a)
            {
                return ~a;
            });

            function::add("and", [](const int a, const int b)
            {
                return a & b;
            });

            function::add("or", [](const int a, const int b)
            {
                return a | b;
            });

            function::add("structget", [](const scripting::object& obj, const std::string& key)
            {
                return obj[key];
            });

            function::add("structset", [](const scripting::object& obj, const std::string& key, 
                const scripting::script_value& value)
            {
                obj[key] = value;
            });

            function::add("structremove", [](const scripting::object& obj, const std::string& key)
            {
                obj.erase(key);
            });

            function::add("getstructkeys", [](const scripting::object& obj)
            {
                return obj.get_keys();
            });

            function::add("isfunctionptr", [](const scripting::script_value& value)
            {
                return value.is<scripting::function>();
            });

            function::add("isentity", [](const scripting::script_value& value)
            {
                const auto& raw = value.get_raw();
                const auto type = game::scr_VarGlob->objectVariableValue[raw.u.uintValue].w.type & 0x7F;
                return raw.type == game::SCRIPT_OBJECT && type == game::SCRIPT_ENTITY;
            });

            function::add("isstruct", [](const scripting::script_value& value)
            {
                return value.is<scripting::object>();
            });

            function::add("typeof", [](const scripting::script_value& value)
            {
                return value.type_name();
            });

            method::add("get", [](const scripting::entity& entity, const std::string& field)
            {
                return entity.get(field);
            });

            method::add("set", [](const scripting::entity& entity, const std::string& field,
                const scripting::script_value& value)
            {
                entity.set(field, value);
            });

            function::add("worldget", [](const std::string& key)
            {
                return world[key];
            });

            function::add("worldset", [](const std::string& key, const scripting::script_value& value)
            {
                world[key] = value;
            });

            function::add("invokefunc", [](const std::string& name, const scripting::variadic_args& args)
            {
                const auto lower = utils::string::to_lower(name);

                std::vector<scripting::script_value> arguments;
                for (auto i = args.begin() + 1, end = args.end(); i < end; ++i)
                {
                    arguments.emplace_back(i->get_raw());
                }

                disable_longjmp_error = true;

                const auto _0 = gsl::finally([&]
                {
                    *reinterpret_cast<const char**>(SELECT(0x2E27C70, 0x2DF7F70)) = nullptr;
                    disable_longjmp_error = false;
                });

                return scripting::call(lower, arguments);
            });

            function::add("detourfunc", [](const std::string& name, const scripting::function& stub)
            {
                const auto iter = scripting::function_map.find(name);
                if (iter == scripting::function_map.end())
                {
                    throw std::runtime_error("function not found");
                }

                const auto func = iter->second.actionFunc;
                hooked_builtins[func] = stub.get_pos();
            });

            function::add("disabledetour", [](const std::string& name)
            {
                const auto iter = scripting::function_map.find(name);
                if (iter == scripting::function_map.end())
                {
                    throw std::runtime_error("function not found");
                }

                const auto func = iter->second.actionFunc;
                hooked_builtins.erase(func);
            });

            function::add("scripting::test", [](const scripting::object& object)
            {
                std::unordered_map<std::string, std::string> map;

                for (auto i = map.begin(); i != map.end(); i++)
                {
                    const auto b = *i;
                }

                auto iter = object.find("value");
                if (iter != object.end())
                {
                    iter->second = "balls";
                    auto t = iter->second.type_name();
                    printf("aaaaa %s\n", t.data());
                }

                auto begin = object.begin();
                auto end = object.end();
                printf("begin == end %i\n", begin == end);

                for (auto i = object.begin(); i != object.end(); ++i)
                {
                    printf("key: %s\n", i->first.data());
                }

                for (const auto& [key, value] : object)
                {
                    auto str = value.to_string();
                    printf("%s = %s\n", key.data(), str.data());
                }
            });

            function::add("scripting::test2", [](const scripting::array& object)
            {
                auto iter = object.find("value");
                if (iter != object.end())
                {
                    iter->second = "balls";
                    auto t = iter->second.type_name();
                    printf("aaaaa %s\n", t.data());
                }

                auto begin = object.begin();
                auto end = object.end();
                printf("begin == end %i\n", begin == end);

                for (auto i = object.begin(); i != object.end(); ++i)
                {
                    printf("key: %s\n", i->first.to_string().data());
                }

                for (const auto& [key, value] : object)
                {
                    auto str = value.to_string();
                    auto key_str = key.to_string();
                    printf("%s = %s\n", key_str.data(), str.data());
                }
            });
        }
    };
}

REGISTER_COMPONENT(gsc::component)

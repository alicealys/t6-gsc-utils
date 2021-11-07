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

    value_wrap::value_wrap(const scripting::script_value& value, int argument_index)
        : value_(value)
        , argument_index_(argument_index)
    {
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

    value_wrap function_args::get(const int index) const
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
        }
    };
}

REGISTER_COMPONENT(gsc::component)
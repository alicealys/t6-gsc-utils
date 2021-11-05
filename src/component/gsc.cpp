#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"

#include <utils/hook.hpp>

namespace gsc
{
    namespace
    {
        std::unordered_map<std::string, game::BuiltinMethodDef> registered_methods;
        std::unordered_map<std::string, game::BuiltinFunctionDef> registered_functions;

        utils::hook::detour scr_get_common_function_hook;
        utils::hook::detour player_get_method_hook;

        script_function scr_get_common_function(const char** pName, int* type, int* min_args, int* max_args)
        {
            auto func = function::find(*pName);

            if (func == nullptr)
            {
                return scr_get_common_function_hook.invoke<script_function>(pName, type, min_args, max_args);
            }

            *pName = func->actionString;
            *type = func->type;
            *min_args = func->min_args;
            *max_args = func->max_args;

            return func->actionFunc;
        }

        script_method player_get_method(const char** pName, int* min_args, int* max_args)
        {
            auto method = method::find(*pName);

            if (method == nullptr)
            {
                return player_get_method_hook.invoke<script_method>(pName, min_args, max_args);
            }

            *pName = method->actionString;
            *min_args = method->min_args;
            *max_args = method->max_args;

            return method->actionFunc;
        }
    }

    namespace function
    {
        void add(const char* name, int min_args, int max_args, script_function f)
        {
            if (find(name) != nullptr)
            {
                return;
            }

            registered_functions[name] = {};
            auto itr = registered_functions.find(name);

            itr->second.actionString = name;
            itr->second.type = 1;
            itr->second.min_args = min_args;
            itr->second.max_args = max_args;
            itr->second.actionFunc = f;
        }

        game::BuiltinFunctionDef* find(const std::string& name)
        {
            auto itr = registered_functions.find(name);

            if (itr != registered_functions.end())
            {
                return &itr->second;
            }

            return nullptr;
        }
    }

    namespace method
    {
        void add(const char* name, int min_args, int max_args, script_method f)
        {
            if (find(name) != nullptr)
            {
                return;
            }

            registered_methods[name] = {};
            auto itr = registered_methods.find(name);

            itr->second.actionString = name;
            itr->second.type = 1;
            itr->second.min_args = min_args;
            itr->second.max_args = max_args;
            itr->second.actionFunc = f;
        }

        game::BuiltinMethodDef* find(const std::string& name)
        {
            auto itr = registered_methods.find(name);

            if (itr != registered_methods.end())
            {
                return &itr->second;
            }

            return nullptr;
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            scr_get_common_function_hook.create(SELECT(0x691110, 0x4EB070), scr_get_common_function);
            player_get_method_hook.create(SELECT(0x432480, 0x6F2DB0), player_get_method);

            function::add("getfunction", 2, 2, []()
            {
                const auto filename = game::get<const char*>(0);
                const auto function = game::get<const char*>(1);
                const auto handle = game::Scr_GetFunctionHandle(game::SCRIPTINSTANCE_SERVER, filename, function, 0, 0);

                game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, handle);
                const auto stack = *reinterpret_cast<int**>(SELECT(0x2E1A5E0, 0x2DEA8E0));
                *stack = 10;
            });
        }
    };
}

REGISTER_COMPONENT(gsc::component)
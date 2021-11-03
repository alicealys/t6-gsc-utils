#include <stdafx.hpp>
#include "loader/component_loader.hpp"
#include "functions.hpp"
#include "methods.hpp"

namespace gsc
{
    namespace
    {
        utils::hook::detour scr_get_common_function_hook;
        utils::hook::detour player_get_method_hook;

        void (*scr_get_common_function(const char** pName, int* type, int* min_args, int* max_args))()
        {
            auto func = function::find(*pName);

            if (func == nullptr)
            {
                return scr_get_common_function_hook.invoke<void(__cdecl*)()>(pName, type, min_args, max_args);
            }

            *pName = func->actionString;
            *type = func->type;
            *min_args = func->min_args;
            *max_args = func->max_args;

            return func->actionFunc;
        }

        void (*player_get_method(const char** pName, int* min_args, int* max_args))(game::scr_entref_t)
        {
            auto method = method::find(*pName);

            if (method == nullptr)
            {
                return player_get_method_hook.invoke<void(__cdecl*)(game::scr_entref_t)>(pName, min_args, max_args);
            }

            *pName = method->actionString;
            *min_args = method->min_args;
            *max_args = method->max_args;

            return method->actionFunc;
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            scr_get_common_function_hook.create(SELECT(0x691110, 0x4EB070), scr_get_common_function);
            player_get_method_hook.create(SELECT(0x432480, 0x6F2DB0), player_get_method);
        }
    };
}

REGISTER_COMPONENT(gsc::component)

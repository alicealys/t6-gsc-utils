#include <stdafx.hpp>

namespace gsc
{
    namespace
    {
        void (*scr_get_common_function(const char** pName, int* type, int* min_args, int* max_args))()
        {
            auto func = function::find(*pName);

            if (func == nullptr)
            {
                auto value = game::Scr_GetCommonFunction(pName, type, min_args, max_args);
                return reinterpret_cast<void(__cdecl*)()>(value);
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
                auto value = game::Player_GetMethod(pName, min_args, max_args);
                return reinterpret_cast<void(__cdecl*)(game::scr_entref_t)>(value);
            }

            *pName = method->actionString;
            *min_args = method->min_args;
            *max_args = method->max_args;

            return method->actionFunc;
        }
    }

    void setup()
    {
        utils::hook::call(SELECT(0x4B580D, 0x4AD09D), scr_get_common_function);
        utils::hook::call(SELECT(0x59D0AE, 0x48613E), player_get_method);
    }
}
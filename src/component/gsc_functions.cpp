#include <stdafx.hpp>

namespace gsc_functions
{
    void init()
    {
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
}
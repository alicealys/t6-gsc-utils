#include <stdafx.hpp>
#include "loader/component_loader.hpp"
#include "gsc/functions.hpp"

namespace gsc_functions
{
    class component final : public component_interface
    {
    public:
		void post_unpack() override
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
	};
}

REGISTER_COMPONENT(gsc_functions::component)

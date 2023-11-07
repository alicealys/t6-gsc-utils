#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

namespace bots
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			add_gsc_functions();
		}

	private:
		static void add_gsc_functions()
		{
			gsc::function::add("dropallbots", []
			{
				const auto* dvar = game::Dvar_FindVar("com_maxclients");
				const auto client_count = game::Dvar_GetInt(dvar);

				for (auto i = 0; i < client_count; ++i)
				{
					if (game::SV_IsTestClient(i))
					{
						game::SV_GameDropClient(i, "GAME_DROPPEDFORINACTIVITY");
					}
				}
			});
		}
	};
}

REGISTER_COMPONENT(bots::component)

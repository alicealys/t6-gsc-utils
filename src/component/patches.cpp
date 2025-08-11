#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/scripting/safe_execution.hpp"
#include "component/gsc.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace patches
{
	namespace
	{
		const game::dvar_t* sv_display_clan_tag;

		char* cs_display_name_stub(const void* client_state, int type)
		{
			if (!sv_display_clan_tag->current.enabled)
			{
				type = 1;
			}

			return utils::hook::invoke<char*>(SELECT(0x59AE90, 0x563AD0), client_state, type);
		}

		void* bg_get_client_field_stub(int a1, const char* a2)
		{
			const auto result = utils::hook::invoke<void*>(0x5B4220, a1, a2);
			if (!result)
			{
				const auto err = utils::string::va("no client field named %s found", a2);
				game::Scr_Error(game::SCRIPTINSTANCE_SERVER, err, false);
			}

			return result;
		}
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			patch();

			if (game::environment::t6zm())
			{
				patch_zm();
			}
		}

		void patch()
		{
			sv_display_clan_tag = game::Dvar_RegisterBool("sv_display_clan_tag", false, 0, "Display the clan tag in the game log");
			utils::hook::call(SELECT(0x820F1C, 0x81F91C), cs_display_name_stub);
		}

		void patch_zm()
		{
			// Fix some common crashes, not necessary because of `patch()` but just to be sure

			// sets an always null pointer :/
			utils::hook::nop(0x6FF36E, 0x2);

			// related to custom perks and tombstone/electric chair on motd and possibly other contexts
			utils::hook::call(0x85842A, bg_get_client_field_stub);
		}
	};
}

REGISTER_COMPONENT(patches::component)

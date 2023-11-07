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
				const auto err = utils::string::va("No client field named %s found", a2);
				game::Scr_Error(game::SCRIPTINSTANCE_SERVER, err, false);
			}

			return result;
		}

		void execute_with_seh_wrap(const scripting::safe_execution::script_function function, 
			const game::scr_entref_t entref)
		{
			if (gsc::execute_hook(function))
			{
				return;
			}

			if (!scripting::safe_execution::execute_with_seh(function, entref))
			{
				game::Scr_Error(game::SCRIPTINSTANCE_SERVER, "exception handled", false);
			}
		}

		void call_builtin_stub(utils::hook::assembler& a)
		{
			a.mov(dword_ptr(SELECT(0x2E1A5E0, 0x2DEA8E0), eax), esi);

			a.pushad();
			a.push(0);
			a.push(0);
			a.push(ecx);
			a.call(execute_with_seh_wrap);
			a.add(esp, 0xC);
			a.popad();

			a.jmp(SELECT(0x8F6401, 0x8F5161));
		}

		void call_builtin_method_stub(utils::hook::assembler& a)
		{
			a.mov(dword_ptr(edx), ebx);

			a.pushad();
			a.push(eax);
			a.push(esi);
			a.push(dword_ptr(ebp, 0x2C));
			a.call(execute_with_seh_wrap);
			a.add(esp, 0xC);
			a.popad();

			a.add(esp, 0x18);

			a.jmp(SELECT(0x8F6498, 0x8F51F8));
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			patch();

			if (game::environment::t6zm())
			{
				patch_zm();
			}
		}

		void patch()
		{
			utils::hook::jump(SELECT(0x8F63F9, 0x8F5159), utils::hook::assemble(call_builtin_stub));
			utils::hook::jump(SELECT(0x8F6490, 0x8F51F0), utils::hook::assemble(call_builtin_method_stub));

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

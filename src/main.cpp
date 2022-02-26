#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

BOOL APIENTRY DllMain(HMODULE /*module_*/, DWORD ul_reason_for_call, LPVOID /*reserved_*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		if (game::plutonium::is_up_to_date())
        {
			utils::hook::jump(reinterpret_cast<uintptr_t>(&printf), game::plutonium::printf);
		}

		component_loader::post_unpack();
	}

	return TRUE;
}
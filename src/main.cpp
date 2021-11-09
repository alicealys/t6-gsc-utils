#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

BOOL APIENTRY DllMain(HMODULE module_, DWORD ul_reason_for_call, LPVOID reserved_)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
        const auto value = *reinterpret_cast<DWORD*>(0x20800000);
        if (value == 0x2A6784ED)
        {
			utils::hook::jump(reinterpret_cast<uintptr_t>(&printf), game::plutonium::printf);
        }

		component_loader::post_unpack();
	}

	return TRUE;
}
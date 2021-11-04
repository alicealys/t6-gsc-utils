#include <stdafx.hpp>
#include "loader/component_loader.hpp"

BOOL APIENTRY DllMain(HMODULE module_, DWORD ul_reason_for_call, LPVOID reserved_)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// set current gamemode
		game::current = reinterpret_cast<const char*>(0xC2F028) == "multiplayer"s
			? game::gamemode::multiplayer
			: game::gamemode::zombies;

		component_loader::post_unpack();
	}

	return TRUE;
}
#include <stdinc.hpp>
#include "loader/component_loader.hpp"

BOOL APIENTRY DllMain(HMODULE /*module_*/, DWORD ul_reason_for_call, LPVOID /*reserved_*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		component_loader::post_unpack();
	}

	return TRUE;
}
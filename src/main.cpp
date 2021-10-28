#include <stdafx.hpp>

BOOL APIENTRY DllMain(HMODULE module_, DWORD ul_reason_for_call, LPVOID reserved_)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		game::init();
		gsc::setup();

		http::init();
		scheduler::init();
		command::init();
		chat::init();
		io::init();
		gsc_functions::init();
	}

	return TRUE;
}
#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "component/signatures.hpp"

#include <utils/hook.hpp>
#include <utils/binary_resource.hpp>
#include <utils/nt.hpp>

namespace
{
	utils::hook::detour load_library_hook;
	HMODULE __stdcall load_library_stub(LPCSTR lib_name, HANDLE file, DWORD flags)
	{
		if (lib_name == "libmysql.dll"s)
		{
			static auto dll = utils::binary_resource{LIBMYSQL_DLL, lib_name};
			const auto path = dll.get_extracted_file();
			const auto handle = load_library_hook.invoke_pascal<HMODULE>(path.data(), file, flags);

			if (handle != nullptr)
			{
				return handle;
			}
		}

		return load_library_hook.invoke_pascal<HMODULE>(lib_name, file, flags);
	}
}

BOOL APIENTRY DllMain(HMODULE module, DWORD ul_reason_for_call, LPVOID /*reserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		utils::nt::library::set_current_handle(module);

		load_library_hook.create(LoadLibraryExA, load_library_stub);

		if (!signatures::process())
		{
			MessageBoxA(NULL,
				"This version of t6-gsc-utils is outdated.\n" \
				"Download the latest dll from here: https://github.com/fedddddd/t6-gsc-utils/releases",
				"ERROR", MB_ICONERROR);

			return FALSE;
		}

		if (game::plutonium::printf.get() != nullptr)
		{
			utils::hook::jump(reinterpret_cast<uintptr_t>(&printf), game::plutonium::printf);
		}

		component_loader::post_unpack();
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		component_loader::pre_destroy();
	}

	return TRUE;
}

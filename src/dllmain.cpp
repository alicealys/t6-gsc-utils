#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "plugin.hpp"

#include <utils/hook.hpp>
#include <utils/binary_resource.hpp>
#include <utils/nt.hpp>
#include <utils/cryptography.hpp>
#include <utils/io.hpp>

namespace
{
	utils::hook::detour load_library_hook;

	std::string extract_resource(const std::string& name, const int resource)
	{
		const auto data = utils::nt::load_resource(resource);
		const auto path = std::filesystem::current_path() / "tmp" / name;
		const auto path_str = path.generic_string();

		if (!utils::io::write_file(path_str, data))
		{
			const auto current = utils::io::read_file(path_str);
			const auto hash_current = utils::cryptography::md5::compute(current);
			const auto hash_target = utils::cryptography::md5::compute(data);
			if (hash_target != hash_current)
			{
				throw std::runtime_error("failed to extract libmysql.dll!");
			}
		}

		return path_str;
	}

	HMODULE __stdcall load_library_stub(LPCSTR lib_name, HANDLE file, DWORD flags)
	{
		if (lib_name == "libmysql.dll"s)
		{
			const auto path = extract_resource(lib_name, LIBMYSQL_DLL);
			const auto handle = load_library_hook.invoke_pascal<HMODULE>(path.data(), file, flags);

			if (handle != nullptr)
			{
				return handle;
			}
			else
			{
				throw std::runtime_error(std::format("failed to load libmysql.dll: {}", GetLastError()));
			}
		}

		return load_library_hook.invoke_pascal<HMODULE>(lib_name, file, flags);
	}
}

PLUTONIUM_API plutonium::sdk::plugin* PLUTONIUM_CALLBACK on_initialize()
{
	return plugin::get();
}

BOOL APIENTRY DllMain(HMODULE module, DWORD ul_reason_for_call, LPVOID /*reserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		utils::nt::library::set_current_handle(module);
		load_library_hook.create(LoadLibraryExA, load_library_stub);
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		component_loader::on_shutdown();
	}

	return TRUE;
}

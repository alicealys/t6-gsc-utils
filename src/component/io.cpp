#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "gsc.hpp"
#include "io.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <curl/curl.h>

namespace io
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			const auto path = game::Dvar_FindVar("fs_homepath")->current.string;
			std::filesystem::current_path(path);

			gsc::function::add("fremove", [](const char* path)
			{
				return std::remove(path);
			});

			gsc::function::add("fopen", [](const char* path, const char* mode)
			{
				FILE* handle = nullptr;
				if (fopen_s(&handle, path, mode) != 0)
				{
					throw std::runtime_error("Invalid handle");
				}

				if (!handle)
				{
					printf("fopen: Invalid path\n");
				}

				return handle;
			});

			gsc::function::add("fclose", [](FILE* handle)
			{
				return fclose(handle);
			});

			gsc::function::add("fwrite", [](FILE* handle, const char* text)
			{
				return fprintf(handle, "%s", text);
			});

			gsc::function::add("fread", [](FILE* handle)
			{
				fseek(handle, 0, SEEK_END);
				const auto length = ftell(handle);

				fseek(handle, 0, SEEK_SET);
				char* buffer = (char*)calloc(length, sizeof(char));

				fread(buffer, sizeof(char), length, handle);

				const std::string result = buffer;

				free(buffer);

				return result;
			});

			gsc::function::add("hashstring", [](const char* str)
			{
				return game::BG_StringHashValue(str);
			});

			gsc::function::add_multiple([](const std::string& file, const std::string& data,
				const scripting::variadic_args& va)
			{
				auto append = false;

				if (va.size() > 0)
				{
					append = va[0];
				}

				return utils::io::write_file(file, data, append);
			}, "writefile", "io::write_file");

			gsc::function::add_multiple([](const std::string& file, const std::string& data)
			{
				return utils::io::write_file(file, data, true);
			}, "appendfile", "io::append_file");

			gsc::function::add_multiple(utils::io::file_exists, "fileexists", "io::file_exists");
			gsc::function::add_multiple(utils::io::move_file, "movefile", "io::move_file");
			gsc::function::add_multiple(utils::io::file_size, "filesize", "io::file_size");
			gsc::function::add_multiple(utils::io::create_directory, "createdirectory", "io::create_directory");
			gsc::function::add_multiple(utils::io::directory_exists, "directoryexists", "io::directory_exists");
			gsc::function::add_multiple(utils::io::directory_is_empty, "directoryisempty", "io::directory_is_empty");
			gsc::function::add_multiple(utils::io::list_files, "listfiles", "io::list_files");
			gsc::function::add_multiple(utils::io::remove_file, "removefile", "io::remove_file");

			gsc::function::add_multiple([](const std::string& directory, const scripting::variadic_args& va)
			{
				auto recursive = false;
				if (va.size() > 0)
				{
					recursive = va[0];
				}

				utils::io::remove_directory(directory, recursive);
			}, "removedirectory", "io::remove_directory");

			gsc::function::add_multiple(utils::io::copy_folder, "copyfolder", "io::copy_folder");
			gsc::function::add_multiple(utils::io::copy_folder, "copydirectory", "io::copy_directory");
			gsc::function::add_multiple(static_cast<std::string(*)(const std::string&)>(utils::io::read_file), "readfile", "io::read_file");
		}
	};
}

REGISTER_COMPONENT(io::component)

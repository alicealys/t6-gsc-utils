#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "gsc.hpp"
#include "io.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>

namespace io
{
	namespace
	{
		scripting::script_value http_get(const gsc::function_args& args)
		{
			const auto url = args[0].as<std::string>();
			const scripting::object object{};
			const auto object_id = object.get_entity_id();

			scheduler::once([object_id, url]()
			{
				const auto result = utils::http::get_data(url);
				scheduler::once([object_id, result]()
				{
					const auto value = result.has_value()
						? result.value().substr(0, 0x5000)
						: ""s;
					scripting::notify(object_id, "done", {value});
				});
			}, scheduler::pipeline::async);

			return object;
		}

		scripting::script_value http_post(const gsc::function_args& args)
		{
			const auto url = args[0].as<std::string>();
			const auto data = args[1].as<std::string>();
			const auto headers_array = args[2].as<scripting::array>();

			utils::http::headers headers;
			for (const auto& key : headers_array.get_keys())
			{
				const auto value = headers_array[key];
				if (!key.is<std::string>() || !value.is<std::string>())
				{
					continue;
				}

				headers[key.as<std::string>()] = value.as<std::string>();
			}

			const scripting::object object{};
			const auto object_id = object.get_entity_id();

			scheduler::once([object_id, url, data, headers]()
			{
				const auto result = utils::http::post_data(url, data, headers);
				scheduler::once([object_id, result]()
				{
					const auto value = result.has_value() ? result.value().substr(0, 0x5000) : ""s;
					scripting::notify(object_id, "done", { value });
				});
			}, scheduler::pipeline::async);

			return object;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			const auto path = game::Dvar_FindVar("fs_homepath")->current.string;
			std::filesystem::current_path(path);

			gsc::function::add("fremove", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<const char*>();
				return std::remove(path);
			});

			gsc::function::add("fopen", [](const gsc::function_args& args)
			{
				const auto* path = args[0].as<const char*>();
				const auto* mode = args[1].as<const char*>();

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

			gsc::function::add("fclose", [](const gsc::function_args& args)
			{
				const auto handle = args[0].as_ptr<FILE>();
				return fclose(handle);
			});

			gsc::function::add("fwrite", [](const gsc::function_args& args)
			{
				const auto handle = args[0].as_ptr<FILE>();
				const auto text = args[1].as<const char*>();

				return fprintf(handle, text);
			});

			gsc::function::add("fread", [](const gsc::function_args& args)
			{
				const auto handle = args[0].as_ptr<FILE>();

				fseek(handle, 0, SEEK_END);
				const auto length = ftell(handle);

				fseek(handle, 0, SEEK_SET);
				char* buffer = (char*)calloc(length, sizeof(char));

				fread(buffer, sizeof(char), length, handle);

				const std::string result = buffer;

				free(buffer);

				return result;
			});

			gsc::function::add("fileexists", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::file_exists(path);
			});

			gsc::function::add("writefile", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				const auto data = args[1].as<std::string>();

				auto append = false;
				if (args.size() > 2)
				{
					append = args[2].as<bool>();
				}

				return utils::io::write_file(path, data, append);
			});

			gsc::function::add("readfile", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::read_file(path);
			});

			gsc::function::add("filesize", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::file_size(path);
			});

			gsc::function::add("createdirectory", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::create_directory(path);
			});

			gsc::function::add("deletedirectory", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				auto recursive = false;

				if (args.size() == 2)
				{
					recursive = args[1].as<bool>();
				}

				return utils::io::remove_directory(path, recursive);
			});

			gsc::function::add("directoryexists", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::directory_exists(path);
			});

			gsc::function::add("directoryisempty", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::directory_is_empty(path);
			});

			gsc::function::add("listfiles", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				const auto files = utils::io::list_files(path);

				scripting::array array{};
				for (const auto& file : files)
				{
					array.push(file);
				}

				return array;
			});

			gsc::function::add("copyfolder", [](const gsc::function_args& args)
			{
				const auto source = args[0].as<std::string>();
				const auto target = args[1].as<std::string>();
				utils::io::copy_folder(source, target);

				return scripting::script_value{};
			});

			gsc::function::add("removefile", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::remove_file(path);
			});

			gsc::function::add("hashstring", [](const gsc::function_args& args)
			{
				const auto* str = args[0].as<const char*>();
				return game::BG_StringHashValue(str);
			});

			gsc::function::add("httpget", http_get);
			gsc::function::add("curl", http_get);
			gsc::function::add("httppost", http_post);
		}
	};
}

REGISTER_COMPONENT(io::component)

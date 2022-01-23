#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "io.hpp"
#include "gsc.hpp"
#include "scheduler.hpp"

#include "gsc.hpp"
#include "json.hpp"

#include <utils/hook.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>

namespace io
{
	namespace
	{
		void replace(std::string& str, const std::string& from, const std::string& to) 
		{
			const auto start_pos = str.find(from);

			if (start_pos == std::string::npos)
			{
				return;
			}

			str.replace(start_pos, from.length(), to);
		}

		scripting::script_value http_get(const gsc::function_args& args)
		{
			const auto url = args[0].as<std::string>();
			const auto object = scripting::entity(scripting::make_object());

			scheduler::once([object, url]()
			{
				const auto result = utils::http::get_data(url.data());
				scheduler::once([object, result]()
				{
					const auto value = result.has_value()
						? result.value().substr(0, 0x5000)
						: "";
					scripting::notify(object, "done", {value});
				});
			}, scheduler::pipeline::async);

			return object;
		}
	}

	std::string execute_command(const std::string& cmd)
	{
		const auto handle = _popen(cmd.data(), "r");
		char* buffer = (char*)calloc(256, sizeof(char));
		std::string result;

		if (!handle)
		{
			return "";
		}

		while (!feof(handle))
		{
			if (fgets(buffer, 256, handle))
			{
			result += buffer;
			}
		}

		_pclose(handle);

		return result;
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			const auto path = game::Dvar_FindVar("fs_homepath")->current.string;
			std::filesystem::current_path(path);

			gsc::function::add("va", [](const gsc::function_args& args)
			{
				auto fmt = args[0].as<std::string>();

				for (auto i = 1; i < args.size(); i++)
				{
					const auto arg = args[i].to_string();
					replace(fmt, "%s", arg);
				}

				return fmt;
			});

			gsc::function::add("print", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto args_ = args.get_raw();

				for (const auto arg : args_)
				{
					printf("%s\t", arg.to_string().data());
				}

				printf("\n");

				return {};
			});

			gsc::function::add("jsonprint", [](const gsc::function_args& args) -> scripting::script_value
			{
				std::string buffer;

				for (const auto arg : args.get_raw())
				{
					buffer.append(json::gsc_to_string(arg));
					buffer.append("\t");
				}

				printf("%s\n", buffer.data());
				return {};
			});

			gsc::function::add("fremove", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<const char*>();
				return std::remove(path);
			});

			gsc::function::add("fopen", [](const gsc::function_args& args)
			{
				const auto* path = args[0].as<const char*>();
				const auto* mode = args[1].as<const char*>();

				const auto handle = fopen(path, mode);

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
				const auto str = args[0].as<std::string>();
				return game::BG_StringHashValue(str.data());
			});

			gsc::function::add("httpget", http_get);
			gsc::function::add("curl", http_get);
		}
	};
}

REGISTER_COMPONENT(io::component)

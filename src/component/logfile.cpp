#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/concurrency.hpp>
#include <utils/io.hpp>
#include <utils/flags.hpp>

namespace logfile
{
	namespace
	{
		utils::hook::detour printf_hook;
		std::string filename;
		std::mutex mutex;

		bool is_logging_enabled()
		{
			static std::optional<bool> flag;

			if (!flag.has_value())
			{
				flag.emplace(utils::flags::has_flag("log"));
			}

			return flag.value();
		}

		std::string load_path()
		{
			const auto fs_basegame = game::Dvar_FindVar("fs_homepath");
			if (fs_basegame == nullptr)
			{
				return "";
			}

			return fs_basegame->current.string;
		}

		std::string get_path()
		{
			static const auto path = load_path();
			return path;
		}
	}

	void log_hook(const std::string& buffer)
	{
		std::lock_guard _(mutex);

		if (is_logging_enabled())
		{
			utils::io::write_file(filename, buffer, true);
		}
	}

	class component final : public component_interface
	{
	public:
		void on_after_dvar_init([[maybe_unused]] plugin::plugin* plugin) override
		{
			const auto path = get_path();
			std::filesystem::current_path(path);

			utils::io::create_directory("logs");
			filename = utils::string::va("logs/console-%s.log",
				utils::string::get_timestamp().data());
		}
	};
}

REGISTER_COMPONENT(logfile::component)

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
		std::string filename;
		std::mutex mutex;

		bool is_logging_enabled()
		{
			static const auto is_enabled = utils::flags::has_flag("log");
			return is_enabled;
		}
	}

	void log_hook(const std::string& buffer)
	{
		if (!is_logging_enabled())
		{
			return;
		}

		std::lock_guard _(mutex);
		utils::io::write_file(filename, buffer, true);
	}

	class component final : public component_interface
	{
	public:
		void on_after_dvar_init([[maybe_unused]] plugin::plugin* plugin) override
		{
			utils::io::create_directory("logs");
			filename = utils::string::va("logs/console-%s.log",
				utils::string::get_timestamp().data());
		}
	};
}

REGISTER_COMPONENT(logfile::component)

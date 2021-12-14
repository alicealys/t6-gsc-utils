#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/concurrency.hpp>
#include <utils/io.hpp>

namespace logfile
{
	namespace
	{
		utils::hook::detour printf_hook;
		utils::hook::detour printf_hook2;
		std::string filename;
		std::mutex mutex;

		int printf_stub(const char* fmt, ...)
		{
			std::lock_guard _(mutex);

			static thread_local utils::string::va_provider<8, 256> provider;

			va_list ap;
			va_start(ap, fmt);

			const auto result = provider.get(fmt, ap);

			va_end(ap);

			utils::io::write_file(filename, result, true);

			return printf_hook.invoke<int>(result);
		}

		std::string load_path()
		{
			const auto fs_basegame = game::Dvar_FindVar("fs_homepath");
			return fs_basegame->current.string;
		}

		std::string get_path()
		{
			static const auto path = load_path();
			return path;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			const auto path = get_path();
			std::filesystem::current_path(path);

			utils::io::create_directory("logs");
			filename = utils::string::va("logs/console-%s.log",
				utils::string::get_timestamp().data());

			printf_hook.create(game::plutonium::printf.get(), printf_stub);
			printf_hook2.create(printf, printf_stub);
		}
	};
}

REGISTER_COMPONENT(logfile::component)

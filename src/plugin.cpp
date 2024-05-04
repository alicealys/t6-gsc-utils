#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "component/logfile.hpp"

#include "plugin.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace plugin
{
	namespace
	{
		void printf_stub(const char* fmt, ...)
		{
			char buffer[0x2000] = {};

			va_list ap;
			va_start(ap, fmt);

			vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, ap);

			va_end(ap);

			logfile::log_hook(buffer);

			const auto trimmed = utils::string::trim(buffer);
			get()->get_interface()->logging()->info(trimmed);
		}
	}

	std::uint32_t plugin::plugin_version()
	{
		return 1;
	}

	const char* plugin::plugin_name()
	{
		return "t6-gsc-utils";
	}

	bool  plugin::is_game_supported([[maybe_unused]] plutonium::sdk::game game)
	{
		return game == plutonium::sdk::game::t6;
	}

	void plugin::on_startup(plutonium::sdk::iinterface* interface_ptr, plutonium::sdk::game game)
	{
		this->interface_ = interface_ptr;
		this->game_ = game;
		utils::hook::jump(reinterpret_cast<uintptr_t>(&printf), printf_stub);

		component_loader::on_startup();
		interface_ptr->callbacks()->on_dvar_init(&component_loader::on_dvar_init);
		interface_ptr->callbacks()->on_after_dvar_init(&component_loader::on_after_dvar_init);
	}

	void plugin::on_shutdown()
	{
		component_loader::on_shutdown();
	}

	plutonium::sdk::iinterface* plugin::get_interface()
	{
		return this->interface_;
	}

	plutonium::sdk::game plugin::get_game()
	{
		return this->game_;
	}

	plugin* get()
	{
		static plugin instance;
		return &instance;
	}
}

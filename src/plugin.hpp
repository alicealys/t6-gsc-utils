#pragma once

#include <plutonium_sdk.hpp>

namespace plugin
{
	class plugin : public plutonium::sdk::plugin
	{
	public:
		~plugin() = default;

		std::uint32_t plugin_version() override;
		const char* plugin_name() override;

		bool is_game_supported([[maybe_unused]] plutonium::sdk::game game) override;

		void on_startup(plutonium::sdk::iinterface* interface_ptr, plutonium::sdk::game game) override;
		void on_shutdown() override;

		plutonium::sdk::iinterface* get_interface();
		plutonium::sdk::game get_game();

	private:
		plutonium::sdk::iinterface* interface_{};
		plutonium::sdk::game game_{};

	};

	plugin* get();
}

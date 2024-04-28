#pragma once
#include "component_interface.hpp"

class component_loader final
{
public:
	class premature_shutdown_trigger final : public std::exception
	{
		[[nodiscard]] const char* what() const noexcept override
		{
			return "Premature shutdown requested";
		}
	};

	template <typename T>
	class installer final
	{
		static_assert(std::is_base_of<component_interface, T>::value, "component has invalid base class");

	public:
		installer(const std::string& name)
		{
			register_component(name, std::make_unique<T>());
		}
	};

	template <typename T>
	static T* get()
	{
		for (const auto& component_ : get_components())
		{
			if (typeid(*component_.get()) == typeid(T))
			{
				return reinterpret_cast<T*>(component_.get());
			}
		}

		return nullptr;
	}

	static void register_component(const std::string& name, std::unique_ptr<component_interface>&& component);

	static void on_startup();
	static void on_dvar_init();
	static void on_after_dvar_init();
	static void on_shutdown();
	static void clean();

	static void* load_import(const std::string& library, const std::string& function);

	static void trigger_premature_shutdown();

private:
	static std::vector<std::pair<std::string, std::unique_ptr<component_interface>>>& get_components();
};

#define REGISTER_COMPONENT(name)                    \
namespace                                           \
{                                                   \
	static component_loader::installer<name> __component(#name); \
}

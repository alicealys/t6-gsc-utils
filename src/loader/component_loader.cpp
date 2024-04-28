#include <stdinc.hpp>
#include "component_loader.hpp"

void component_loader::register_component(const std::string& name, std::unique_ptr<component_interface>&& component_)
{
	get_components().push_back(std::make_pair(name, std::move(component_)));
}

#define ON_CALLBACK(__name__) \
	void component_loader::__name__() \
	{ \
		static auto handled = false; \
		if (handled) \
		{ \
			return; \
		} \
		\
		handled = true; \
		\
		for (const auto& component_ : get_components()) \
		{ \
			try \
			{ \
				component_.second->__name__(plugin::get()); \
			} \
			catch (const std::exception& e) \
			{ \
				printf("error executing component \"%s\" callback \"%s\": %s\n", component_.first.data(), #__name__, e.what()); \
			} \
		} \
	} \

ON_CALLBACK(on_startup);
ON_CALLBACK(on_dvar_init);
ON_CALLBACK(on_after_dvar_init);
ON_CALLBACK(on_shutdown);

void component_loader::clean()
{
	auto& components = get_components();
	for (auto i = components.begin(); i != components.end();)
	{
		if (!(*i).second->is_supported())
		{
			(*i).second->on_shutdown(plugin::get());
			i = components.erase(i);
		}
		else
		{
			++i;
		}
	}
}

void component_loader::trigger_premature_shutdown()
{
	throw premature_shutdown_trigger();
}

std::vector<std::pair<std::string, std::unique_ptr<component_interface>>>& component_loader::get_components()
{
	using component_vector = std::vector<std::pair<std::string, std::unique_ptr<component_interface>>>;
	using component_vector_container = std::unique_ptr<component_vector, std::function<void(component_vector*)>>;

	static component_vector_container components(new component_vector, [](component_vector* component_vector)
	{
		on_shutdown();
		delete component_vector;
	});

	return *components;
}

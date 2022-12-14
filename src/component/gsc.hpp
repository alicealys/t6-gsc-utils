#pragma once

#include "game/scripting/array.hpp"
#include "game/scripting/execution.hpp"

namespace gsc
{
	enum classid
	{
		entity,
		hudelem,
		pathnode,
		node,
		count
	};

	void return_value(const scripting::script_value& value);
	std::vector<scripting::script_value> get_arguments();

	class function_args
	{
	public:
		function_args(std::vector<scripting::script_value> = get_arguments());

		unsigned int size() const;
		std::vector<scripting::script_value> get_raw() const;
		scripting::value_wrap get(const int index) const;

		scripting::value_wrap operator[](const int index) const
		{
			return this->get(index);
		}
	private:
		std::vector<scripting::script_value> values_;
	};

	using builtin_function = void(*)();
	using builtin_method = void(*)(game::scr_entref_t);

	using script_function = std::function<scripting::script_value(const function_args&)>;
	using script_method = std::function<scripting::script_value(const scripting::entity&, const function_args&)>;

	std::string find_builtin_name(void* function);
	std::string find_builtin_method_name(void* function);

	namespace function
	{
		void add(const std::string& name, const script_function& function);
	}

	namespace method
	{
		void add(const std::string& name, const script_method& function);
	}

	bool call_method(unsigned int ptr, game::scr_entref_t ent_ref);
	bool call_function(unsigned int ptr);
}
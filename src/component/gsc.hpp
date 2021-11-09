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

	class value_wrap
	{
	public:
		value_wrap(const scripting::script_value& value, int argument_index);

		template <typename T>
		T as() const
		{
			try
			{
				return this->value_.as<T>();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(utils::string::va("parameter %d %s", this->argument_index_, e.what()));
			}
		}

		template <typename T, typename I = int>
		T* as_ptr()
		{
			try
			{
				return this->value_.as_ptr<T>();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(utils::string::va("parameter %d %s", this->argument_index_, e.what()));
			}
		}

		template <typename T>
		T is() const
		{
			return this->value_.is<T>();
		}

		const game::VariableValue& get_raw() const
		{
			return this->value_.get_raw();
		}

		int argument_index_{};
		scripting::script_value value_;
	};

	class function_args
	{
	public:
		function_args(std::vector<scripting::script_value> = get_arguments());

		unsigned int size() const;
		std::vector<scripting::script_value> get_raw() const;
		value_wrap get(const int index) const;

		value_wrap operator[](const int index) const
		{
			return this->get(index);
		}
	private:
		std::vector<scripting::script_value> values_;
	};

	using builtin_function = void(*)();
	using builtin_method = void(*)(game::scr_entref_t);

	using script_function = std::function<scripting::script_value(const function_args&)>;
	using script_method = std::function<scripting::script_value(const game::scr_entref_t, const function_args&)>;

	namespace function
	{
		void add(const std::string& name, const script_function& function);
	}

	namespace method
	{
		void add(const std::string& name, const script_method& function);
	}
}
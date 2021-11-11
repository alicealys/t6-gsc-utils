#include <stdinc.hpp>
#include "function.hpp"
#include "execution.hpp"
#include "../../component/scripting.hpp"

namespace scripting
{
	function::function(const char* pos)
		: pos_(pos)
	{
	}

	script_value function::get_raw() const
	{
		game::VariableValue value;
		value.type = game::SCRIPT_FUNCTION;
		value.u.codePosValue = this->pos_;

		return value;
	}

	const char* function::get_pos() const
	{
		return this->pos_;
	}

	std::string function::get_name() const
	{
		if (scripting::script_function_table_rev.find(this->pos_) != scripting::script_function_table_rev.end())
		{
			const auto func = scripting::script_function_table_rev[this->pos_];
			return utils::string::va("%s::%s", func.first.data(), func.second.data());
		}

		return "unknown function";
	}

	script_value function::call(const entity& self, std::vector<script_value> arguments) const
	{
		return exec_ent_thread(self, this->pos_, arguments);
	}
}

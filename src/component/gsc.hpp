#pragma once

namespace gsc
{
	using script_function = void(*)();
	using script_method = void(*)(game::scr_entref_t);

	namespace function
	{
		void add(const char* name, int min_args, int max_args, script_function);
		game::BuiltinFunctionDef* find(const std::string& name);
	}

	namespace method
	{
		void add(const char* name, int min_args, int max_args, script_method);
		game::BuiltinMethodDef* find(const std::string& name);
	}
}
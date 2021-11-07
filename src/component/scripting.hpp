#pragma once

namespace scripting
{
	extern std::unordered_map<int, std::unordered_map<std::string, int>> fields_table;

	extern std::unordered_map<std::string, game::BuiltinMethodDef> method_map;
	extern std::unordered_map<std::string, game::BuiltinFunctionDef> function_map;

	using script_function = void(*)(game::scr_entref_t);

	script_function find_function(const std::string& name);
}
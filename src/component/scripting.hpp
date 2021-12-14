#pragma once

namespace scripting
{
	extern std::unordered_map<int, std::unordered_map<std::string, int>> fields_table;

	extern std::unordered_map<std::string, std::unordered_map<std::string, const char*>> script_function_table;
	extern std::unordered_map<std::string, std::vector<std::pair<std::string, const char*>>> script_function_table_sort;
	extern std::unordered_map<const char*, std::pair<std::string, std::string>> script_function_table_rev;

	extern std::unordered_map<std::string, game::BuiltinMethodDef> method_map;
	extern std::unordered_map<std::string, game::BuiltinFunctionDef> function_map;

	using script_function = void(*)(game::scr_entref_t);

	script_function find_function_ptr(const std::string& name);
	std::string find_function(const char* pos);
	const char* find_function_start(const char* pos);
	std::optional<std::pair<std::string, std::string>> find_function_pair(const char* pos);

	void on_shutdown(const std::function<void()>& callback);
}
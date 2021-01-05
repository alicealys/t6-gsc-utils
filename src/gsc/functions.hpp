#pragma once

namespace function
{
	void add(const char* name, int min_args, int max_args, void(*f)());
	game::BuiltinFunctionDef* find(const std::string& name);
}
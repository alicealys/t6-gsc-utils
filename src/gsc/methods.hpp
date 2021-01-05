#pragma once

namespace method
{
	void add(const char* name, int min_args, int max_args, void(*f)(game::scr_entref_t));
	game::BuiltinMethodDef* find(const std::string& name);
}
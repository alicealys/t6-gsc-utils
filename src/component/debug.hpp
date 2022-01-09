#pragma once

namespace debug
{
	std::string get_call_stack(bool print_local_vars = false);
	std::string get_child_var_allocations(int limit);
}
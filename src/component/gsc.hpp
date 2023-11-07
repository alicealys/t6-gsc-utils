#pragma once

#include "game/scripting/array.hpp"
#include "game/scripting/execution.hpp"

#if _HAS_CXX20
namespace gsc
{
	using function_t = std::function<scripting::script_value(const scripting::function_arguments& args)>;
	using script_function = void(*)(game::scr_entref_t);

	extern std::unordered_map<std::string, function_t> functions;
	extern std::unordered_map<std::string, function_t> methods;

	extern std::unordered_map<std::string, void*> function_wraps;
	extern std::unordered_map<std::string, void*> method_wraps;

	// auto = []{} forces template reevaluation

	template <class... Args, std::size_t... I, auto = []{}>
	auto wrap_function(const std::function<void(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const scripting::function_arguments& args)
		{
			f(args[I]...);
			return scripting::script_value{};
		};
	}

	template <class... Args, std::size_t... I, auto = []{}>
	auto wrap_function(const std::function<scripting::script_value(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const scripting::function_arguments& args)
		{
			return f(args[I]...);
		};
	}

	template <typename R, class... Args, std::size_t... I, auto = []{}>
	auto wrap_function(const std::function<R(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const scripting::function_arguments& args)
		{
			return scripting::script_value{f(args[I]...)};
		};
	}

	template <typename R, class... Args, auto = []{}>
	auto wrap_function(const std::function<R(Args...)>& f)
	{
		return wrap_function(f, std::index_sequence_for<Args...>{});
	}

	template <class F, auto = []{}>
	auto wrap_function(F f)
	{
		return wrap_function(std::function(f));
	}

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

	using builtin_function = void(*)();
	using builtin_method = void(*)(game::scr_entref_t);

	std::string find_builtin_name(void* function);
	std::string find_builtin_method_name(void* function);

	bool execute_hook(const void* ptr);

	void call_function(const function_t& function, const std::string& name);
	void call_method(const function_t& method, const std::string& name, const game::scr_entref_t entref);

	namespace function
	{
		template <typename F, auto = []{}>
		void add_internal(const std::string& name, F function)
		{
			static auto name_ = name;
			static auto done = false;
			if (done)
			{
				printf("already added %s (%s)\n", name.data(), name_.data());
			}
			done = true;

			static const auto lower = utils::string::to_lower(name);
			static const auto [iterator, was_inserted] = functions.insert(std::make_pair(lower, function));
			static const auto& function_ptr = iterator->second;

			function_wraps[lower] = []()
			{
				call_function(function_ptr, lower);
			};
		}

		template <typename F, auto = []{}>
		void add(const std::string& name, F f)
		{
			add_internal(name, wrap_function(f));
		}

		template <typename ...Args, typename F, auto = []{}>
		void add_multiple(F f, Args&& ...names)
		{
			(add(names, f), ...);
		}
	}

	namespace method
	{
		template <typename F, auto = []{}>
		void add_internal(const std::string& name, F function)
		{
			static auto name_ = name;
			static auto done = false;
			if (done)
			{
				printf("already added %s (%s)\n", name.data(), name_.data());
			}
			done = true;

			static const auto lower = utils::string::to_lower(name);
			static const auto [iterator, was_inserted] = functions.insert(std::make_pair(lower, function));
			static const auto& function_ptr = iterator->second;

			method_wraps[lower] = [](game::scr_entref_t entref)
			{
				call_method(function_ptr, lower, entref);
			};
		}

		template <typename F, auto = []{}>
		void add(const std::string& name, F f)
		{
			add_internal(name, wrap_function(f));
		}

		template <typename ...Args, typename F, auto = []{}>
		void add_multiple(F f, Args&& ...names)
		{
			(add(names, f), ...);
		}
	}
}
#else
#error pre c++20 is not supported
#endif

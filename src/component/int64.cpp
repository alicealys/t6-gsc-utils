#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

#include <utils/io.hpp>
#include <utils/hook.hpp>

#define INT64_OPERATION(expr) \
	[](const std::int64_t a, [[maybe_unused]] const std::int64_t b)  \
	{ \
		return expr; \
	} \

namespace int64
{
	namespace
	{
		std::int64_t get_int64_arg(const scripting::variadic_args& args, const size_t index, bool optional)
		{
			if (optional && index >= static_cast<int>(args.size()))
			{
				return 0;
			}

			if (args[index].is<int>())
			{
				return static_cast<std::int64_t>(args[index].as<int>());
			}

			if (args[index].is<const char*>())
			{
				return std::strtoll(args[index].as<const char*>(), nullptr, 0);
			}

			throw std::runtime_error(std::format("parameter {} does not have type 'string' or 'int'", index));
		}

		std::unordered_map<std::string, std::function<std::int64_t(std::int64_t, std::int64_t)>> operations =
		{
			{"+",  INT64_OPERATION(a + b)},
			{"-",  INT64_OPERATION(a - b)},
			{"*",  INT64_OPERATION(a * b)},
			{"/",  INT64_OPERATION(a / b)},
			{"&",  INT64_OPERATION(a & b)},
			{"^",  INT64_OPERATION(a ^ b)},
			{"|",  INT64_OPERATION(a | b)},
			{"~",  INT64_OPERATION(~a)},
			{"%",  INT64_OPERATION(a % b)},
			{">>", INT64_OPERATION(a >> b)},
			{"<<", INT64_OPERATION(a << b)},
			{"++", INT64_OPERATION(a + 1)},
			{"--", INT64_OPERATION(a - 1)},
		};

		std::unordered_map<std::string, std::function<bool(std::int64_t, std::int64_t)>> comparisons =
		{
			{">",  INT64_OPERATION(a > b)},
			{">=", INT64_OPERATION(a >= b)},
			{"==", INT64_OPERATION(a == b)},
			{"!=", INT64_OPERATION(a != b)},
			{"<=", INT64_OPERATION(a <= b)},
			{"<",  INT64_OPERATION(a < b)},
		};
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			gsc::function::add_multiple([](const scripting::variadic_args& args)
			{
				auto value = get_int64_arg(args, 0, false);
				return value <= std::numeric_limits<std::int32_t>::max() && value >= std::numeric_limits<std::int32_t>::min();
			}, "int64_is_int", "int64::is_int");

			gsc::function::add_multiple([](const scripting::variadic_args& args)
			{
				return static_cast<std::int32_t>(get_int64_arg(args, 0, false));
			}, "int64_to_int", "int64::to_int");

			gsc::function::add_multiple([](const scripting::variadic_args& args) -> scripting::script_value
			{
				auto a = get_int64_arg(args, 0, false);
				const auto op = args[1].as<std::string>();
				auto b = get_int64_arg(args, 2, true);

				if (const auto iter = operations.find(op); iter != operations.end())
				{
					return std::to_string(iter->second(a, b));
				}

				if (const auto iter = comparisons.find(op); iter != comparisons.end())
				{
					return iter->second(a, b);
				}

				throw std::runtime_error("invalid int64 operation");
			}, "int64_op", "int64::op");

			gsc::function::add_multiple([](const scripting::variadic_args& args)
			{
				const auto a = get_int64_arg(args, 0, false);
				const auto b = get_int64_arg(args, 1, false);
				return std::to_string(std::min(a, b));
			}, "int64_min", "int64::min");

			gsc::function::add_multiple([](const scripting::variadic_args& args)
			{
				const auto a = get_int64_arg(args, 0, false);
				const auto b = get_int64_arg(args, 1, false);
				return std::to_string(std::max(a, b));
			}, "int64_max", "int64::max");
		}
	};
}

REGISTER_COMPONENT(int64::component)

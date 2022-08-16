#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

#include <utils/io.hpp>
#include <utils/hook.hpp>

#define INT64_OPERATION(expr) [](const int64_t a, [[maybe_unused]] const int64_t b) { return expr; }

namespace int64
{
	namespace
	{
		int64_t get_int64_arg(const gsc::function_args& args, int index, bool optional)
		{
			if (optional && index >= static_cast<int>(args.size()))
			{
				return 0;
			}

			if (args[index].is<int>())
			{
				return args[index].as<int>();
			}

			if (args[index].is<const char*>())
			{
				return std::strtoll(args[index].as<const char*>(), nullptr, 0);
			}

			throw std::runtime_error(utils::string::va("parameter %d does not have type 'string' or 'int'", index));
		}

		std::unordered_map<std::string, std::function<int64_t(int64_t, int64_t)>> operations =
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

		std::unordered_map<std::string, std::function<bool(int64_t, int64_t)>> comparisons =
		{
			{">",  INT64_OPERATION(a > b)},
			{">=", INT64_OPERATION(a >= b)},
			{"==", INT64_OPERATION(a == b)},
			{"<=", INT64_OPERATION(a <= b)},
			{"<",  INT64_OPERATION(a < b)},
		};
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			gsc::function::add("int64_is_int", [](const gsc::function_args& args)
			{
				auto value = get_int64_arg(args, 0, false);
				return value <= std::numeric_limits<int32_t>::max() && value >= std::numeric_limits<int32_t>::min();
			});

			gsc::function::add("int64_to_int", [](const gsc::function_args& args)
			{
				return static_cast<int32_t>(get_int64_arg(args, 0, false));
			});

			gsc::function::add("int64_op", [](const gsc::function_args& args) -> scripting::script_value
			{
				auto a = get_int64_arg(args, 0, false);
				const auto op = args[1].as<std::string>();
				auto b = get_int64_arg(args, 2, true);

				if (operations.find(op) != operations.end())
				{
					return utils::string::va("%lld", operations[op](a, b));
				}

				if (comparisons.find(op) != comparisons.end())
				{
					return comparisons[op](a, b);
				}

				throw std::runtime_error("Invalid int64 operation");
			});
		}
	};
}

REGISTER_COMPONENT(int64::component)

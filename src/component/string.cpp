#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

namespace string
{
	namespace
	{
		void replace(std::string& str, const std::string& from, const std::string& to)
		{
			const auto start_pos = str.find(from);

			if (start_pos == std::string::npos)
			{
				return;
			}

			str.replace(start_pos, from.length(), to);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			gsc::function::add("va", [](const gsc::function_args& args)
			{
				auto fmt = args[0].as<std::string>();

				for (auto i = 1u; i < args.size(); i++)
				{
					const auto arg = args[i].to_string();
					replace(fmt, "%s", arg);
				}

				return fmt;
			});

			gsc::function::add("regex_replace", [](const gsc::function_args& args) -> scripting::script_value
			{
				auto str = args[0].as<std::string>();
				std::regex expr{args[1].as<std::string>()};
				auto with = args[2].as<std::string>();

				return std::regex_replace(str, expr, with);
			});

			gsc::function::add("regex_match", [](const gsc::function_args& args) -> scripting::script_value
			{
				auto str = args[0].as<std::string>();
				std::regex expr{args[1].as<std::string>()};

				scripting::array array_match{};
				std::smatch match{};

				if (std::regex_match(str, match, expr))
				{
					for (const auto& s : match)
					{
						array_match.push((s.str()));
					}
				}

				return array_match;
			});

			gsc::function::add("regex_search", [](const gsc::function_args& args) -> scripting::script_value
			{
				auto str = args[0].as<std::string>();
				std::regex expr{args[1].as<std::string>()};

				return std::regex_search(str, expr);
			});
		}
	};
}

REGISTER_COMPONENT(string::component)

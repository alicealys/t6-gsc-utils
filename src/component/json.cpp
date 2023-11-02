#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/array.hpp"

#include "gsc.hpp"
#include "json.hpp"
#include "scripting.hpp"

#include <utils/io.hpp>

namespace json
{
	namespace
	{
		std::unordered_set<unsigned int> dumped_objects;

		nlohmann::json gsc_to_json(const scripting::script_value& value, bool print_id);

		nlohmann::json array_to_json(const scripting::array& array, bool print_id)
		{
			nlohmann::json obj;

			auto string_indexed = -1;
			const auto keys = array.get_keys();
			for (auto i = 0u; i < keys.size(); i++)
			{
				const auto is_int = keys[i].is<int>();
				const auto is_string = keys[i].is<std::string>();

				if (string_indexed == -1)
				{
					string_indexed = is_string;
				}

				if (!string_indexed && is_int)
				{
					const auto index = keys[i].as<int>();
					obj[index] = gsc_to_json(array[index], print_id);
				}
				else if (string_indexed && is_string)
				{
					const auto key = keys[i].as<std::string>();
					obj.emplace(key, gsc_to_json(array[key], print_id));
				}
			}

			return obj;
		}

		nlohmann::json object_to_json(const scripting::object& object, bool print_id)
		{
			const auto id = object.get_entity_id();
			if (dumped_objects.find(id) != dumped_objects.end())
			{
				return utils::string::va("[struct reference %i]", id);
			}

			dumped_objects.insert(id);
			nlohmann::json obj;

			if (print_id)
			{
				obj["__id"] = id;
			}

			const auto keys = object.get_keys();
			for (const auto& key : keys)
			{
				if (key == "__id")
				{
					continue;
				}

				obj.emplace(key, gsc_to_json(object[key], print_id));
			}

			return obj;
		}

		nlohmann::json vector_to_array(const float* value)
		{
			nlohmann::json obj;
			obj.push_back(value[0]);
			obj.push_back(value[1]);
			obj.push_back(value[2]);

			return obj;
		}

		nlohmann::json gsc_to_json(const scripting::script_value& value, bool print_id)
		{
			const auto variable = value.get_raw();

			if (value.is<int>())
			{
				return value.as<int>();
			}

			if (value.is<float>())
			{
				return value.as<float>();
			}

			if (value.is<std::string>())
			{
				return value.as<std::string>();
			}

			if (value.is<scripting::vector>())
			{
				return vector_to_array(variable.u.vectorValue);
			}

			if (value.is<scripting::object>())
			{
				return object_to_json(variable.u.uintValue, print_id);
			}

			if (value.is<scripting::array>())
			{
				return array_to_json(variable.u.uintValue, print_id);
			}

			if (value.is<scripting::entity>())
			{
				return object_to_json(variable.u.uintValue, print_id);
			}

			if (value.is<scripting::function>())
			{
				const auto function = value.as<scripting::function>();
				return utils::string::va("[[ %s ]]", function.get_name().data());
			}

			if (variable.type == game::SCRIPT_NONE)
			{
				return {};
			}

			if (variable.type == game::SCRIPT_CODEPOS)
			{
				const auto function = scripting::find_function(variable.u.codePosValue);
				return utils::string::va("[codepos: %s]", function.data());
			}

			if (variable.type == game::SCRIPT_END)
			{
				const auto function = scripting::find_function(variable.u.codePosValue);
				return utils::string::va("[precodepos: %s]", function.data());
			}

			return utils::string::va("[%s]", value.type_name().data());
		}

		scripting::script_value json_to_gsc(nlohmann::json obj)
		{
			const auto type = obj.type();

			switch (type)
			{
			case (nlohmann::detail::value_t::number_integer):
			case (nlohmann::detail::value_t::number_unsigned):
				return obj.get<int>();
			case (nlohmann::detail::value_t::number_float):
				return obj.get<float>();
			case (nlohmann::detail::value_t::string):
				return obj.get<std::string>();
			case (nlohmann::detail::value_t::array):
			{
				scripting::array array;

				for (const auto& [key, value] : obj.items())
				{
					array.push(json_to_gsc(value));
				}

				return array.get_raw();
			}
			case (nlohmann::detail::value_t::object):
			{
				scripting::array array;

				for (const auto& [key, value] : obj.items())
				{
					array[key] = json_to_gsc(value);
				}

				return array.get_raw();
			}
			}

			return {};
		}
	}

	std::string gsc_to_string(const scripting::script_value& value)
	{
		dumped_objects = {};
		return gsc_to_json(value, false).dump();
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			gsc::function::add("createmap", [](const gsc::function_args& args)
			{
				scripting::array array;

				for (auto i = 0u; i < args.size(); i += 2)
				{
					if (i >= args.size() - 1)
					{
						continue;
					}

					const auto key = args[i].as<std::string>();
					array[key] = args[i + 1];
				}

				return array;
			});

			gsc::function::add("jsonparse", [](const gsc::function_args& args)
			{
				const auto json = args[0].as<std::string>();
				const auto obj = nlohmann::json::parse(json);
				return json_to_gsc(obj);
			});

			gsc::function::add("jsonserialize", [](const gsc::function_args& args)
			{
				const auto value = args[0];
				auto indent = -1;
				auto print_id = false;

				if (args.size() > 1)
				{
					indent = args[1].as<int>();
				}

				if (args.size() > 2)
				{
					print_id = args[2].as<bool>();
				}

				dumped_objects = {};
				return gsc_to_json(value, print_id).dump(indent).substr(0, 0x5000);
			});

			gsc::function::add("jsondump", [](const gsc::function_args& args)
			{
				auto file = args[0].as<std::string>();
				const auto value = args[1];
				auto indent = -1;
				auto print_id = false;

				if (!file.ends_with(".json"))
				{
					file.append(".json");
				}

				if (args.size() > 2)
				{
					indent = args[2].as<int>();
				}

				if (args.size() > 3)
				{
					print_id = args[3].as<bool>();
				}

				dumped_objects = {};
				return utils::io::write_file(file, gsc_to_json(value, print_id).dump(indent));
			});

			gsc::function::add("jsonprint", [](const gsc::function_args& args) -> scripting::script_value
			{
				std::string buffer;

				for (const auto& arg : args.get_raw())
				{
					buffer.append(gsc_to_string(arg));
					buffer.append("\t");
				}

				printf("%s\n", buffer.data());
				return {};
			});
		}
	};
}

REGISTER_COMPONENT(json::component)
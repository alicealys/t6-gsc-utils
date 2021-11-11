#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/array.hpp"

#include "gsc.hpp"
#include "json.hpp"
#include "scripting.hpp"

#include <json.hpp>
#include <utils/io.hpp>

namespace json
{
	namespace
	{
		nlohmann::json gsc_to_json(scripting::script_value value);

		nlohmann::json array_to_json(const scripting::array& array)
		{
			nlohmann::json obj;

			auto string_indexed = -1;
			const auto keys = array.get_keys();
			for (auto i = 0; i < keys.size(); i++)
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
					obj[index] = gsc_to_json(array[index]);
				}
				else if (string_indexed && is_string)
				{
					const auto key = keys[i].as<std::string>();
					obj.emplace(key, gsc_to_json(array[key]));
				}
			}

			return obj;
		}

		std::unordered_set<unsigned int> dumped_objects;
		nlohmann::json object_to_json(const scripting::object& object)
		{
			const auto id = object.get_entity_id();
			if (dumped_objects.find(id) != dumped_objects.end())
			{
				return "[circular reference]";
			}

			dumped_objects.insert(id);
			nlohmann::json obj;

			const auto keys = object.get_keys();
			for (const auto& key : keys)
			{
				obj.emplace(key, gsc_to_json(object[key]));
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

		nlohmann::json gsc_to_json(scripting::script_value value)
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
				return object_to_json(variable.u.uintValue);
			}

			if (value.is<scripting::array>())
			{
				return array_to_json(variable.u.uintValue);
			}

			if (value.is<scripting::entity>())
			{
				return object_to_json(variable.u.uintValue);
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

			return "[unknown type]";
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
		return gsc_to_json(value).dump().substr(0, 0x5000);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			gsc::function::add("createmap", [](const gsc::function_args& args)
			{
				scripting::array array;

				for (auto i = 0; i < args.size(); i += 2)
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

				if (args.size() > 1)
				{
					indent = args[1].as<int>();
				}

				dumped_objects = {};
				gsc_to_json(value).dump(indent).substr(0, 0x5000);
				return 0;
			});

			gsc::function::add("jsondump", [](const gsc::function_args& args)
			{
				const auto file = args[0].as<std::string>();
				const auto value = args[1];
				auto indent = -1;

				if (args.size() > 2)
				{
					indent = args[2].as<int>();
				}

				dumped_objects = {};
				return utils::io::write_file(file, gsc_to_json(value).dump(indent));
			});
		}
	};
}

REGISTER_COMPONENT(json::component)
#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"

#include <utils/http.hpp>
#include <curl/curl.h>

namespace http
{
	namespace
	{
		std::unordered_map<uint64_t, bool> active_requests{};
		uint64_t request_id{};

		scripting::script_value http_post(const std::string& url, const std::string& data, const scripting::array& headers_array)
		{
			utils::http::headers headers;
			for (const auto& [key, value] : headers_array)
			{
				if (!key.is<std::string>() || !value.is<std::string>())
				{
					continue;
				}

				const auto key_str = key.as<std::string>();
				headers[key_str] = value.as<std::string>();
			}

			const scripting::object object{};
			const auto object_id = object.get_entity_id();

			scheduler::once([object_id, url, data, headers]()
			{
				const auto result = utils::http::post_data(url, data, headers);
				scheduler::once([object_id, result]()
				{
					const auto value = result.has_value() ? result.value().substr(0, 0x5000) : ""s;
					scripting::notify(object_id, "done", {value});
				});
			}, scheduler::pipeline::async);

			return object;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scripting::on_shutdown([]()
			{
				active_requests.clear();
			});

			gsc::function::add_multiple([](const std::string& url)
			{
				const auto id = request_id++;
				active_requests[id] = true;

				const auto object = scripting::object{};
				const auto object_id = object.get_entity_id();

				scheduler::once([id, object_id, url]()
				{
					const auto data = utils::http::get_data_result(url);
					scheduler::once([id, object_id, data]()
					{
						if (active_requests.find(id) == active_requests.end())
						{
							return;
						}

						if (!data.has_value())
						{
							scripting::notify(object_id, "done", {{}, false, "Unknown error"});
							return;
						}

						const auto& result = data.value();
						const auto error = curl_easy_strerror(result.code);

						if (result.code != CURLE_OK)
						{
							scripting::notify(object_id, "done", {{}, false, error});
							return;
						}

						if (result.buffer.size() >= 0x5000)
						{
							printf("^3WARNING: http result size bigger than 20480 bytes (%i), truncating!", static_cast<int>(result.buffer.size()));
						}

						scripting::notify(object_id, "done", {result.buffer.substr(0, 0x5000), true});
					}, scheduler::pipeline::server);
				}, scheduler::pipeline::async);

				return object;
			}, "http::get", "httpget", "curl");

			gsc::function::add("http::request", [](const std::string& url, const scripting::variadic_args& va)
			{
				const auto id = request_id++;
				active_requests[id] = true;

				const auto object = scripting::object{};
				const auto object_id = object.get_entity_id();

				std::string fields_string{};
				std::unordered_map<std::string, std::string> headers_map{};

				if (va.size() > 0)
				{
					const auto options = va[0].as<scripting::array>();

					const auto fields = options["parameters"];
					const auto body = options["body"];
					const auto headers = options["headers"];

					if (fields.is<scripting::array>())
					{
						const auto fields_arr = fields.as<scripting::array>();

						for (const auto& [key, value] : fields_arr)
						{
							if (!key.is<std::string>())
							{
								continue;
							}

							const auto key_str = key.as<std::string>();
							const auto value_str = value.to_string();
							fields_string += std::format("{}={}", key_str, value_str);
						}

					}

					if (body.is<std::string>())
					{
						fields_string = body.as<std::string>();
					}

					if (headers.is<scripting::array>())
					{
						const auto headers_arr = headers.as<scripting::array>();

						for (const auto& [key, value] : headers_arr)
						{
							if (!key.is<std::string>())
							{
								continue;
							}

							const auto key_str = key.as<std::string>();
							const auto value_str = value.to_string();

							headers_map[key_str] = value_str;
						}
					}
				}

				scheduler::once([id, object_id, url, fields_string, headers_map]()
				{
					const auto data = utils::http::get_data_result(url, fields_string, headers_map);
					scheduler::once([data, object_id, id]
					{
						if (active_requests.find(id) == active_requests.end())
						{
							return;
						}

						if (!data.has_value())
						{
							scripting::notify(object_id, "done", {{}, false, "Unknown error"});
							return;
						}

						const auto& result = data.value();
						const auto error = curl_easy_strerror(result.code);

						if (result.code != CURLE_OK)
						{
							scripting::notify(object_id, "done", {{}, false, error});
							return;
						}

						if (result.buffer.size() >= 0x5000)
						{
							printf("^3WARNING: http result size bigger than 20480 bytes (%i), truncating!", static_cast<int>(result.buffer.size()));
						}

						scripting::notify(object_id, "done", {result.buffer.substr(0, 0x5000), true});
					}, scheduler::pipeline::server);
				}, scheduler::pipeline::async);

				return object;
			});

			gsc::function::add("httppost", http_post);
		}
	};
}

REGISTER_COMPONENT(http::component)

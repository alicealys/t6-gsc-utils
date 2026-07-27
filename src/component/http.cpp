#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"

#include <utils/http.hpp>
#include <utils/concurrency.hpp>
#include <curl/curl.h>

namespace http
{
	namespace
	{
		constexpr const auto max_result_size = 0x5000u;

		struct http_request_params_t
		{
			std::string method;
			std::string url;
			std::string fields;
			utils::http::headers headers;
		};

		struct http_request_t
		{
			http_request_params_t params;
			scripting::object handle;
			std::optional<utils::http::result> result;
			std::atomic_bool completed;
		};

		std::vector<std::shared_ptr<http_request_t>> requests;

		void notify_request_result(std::shared_ptr<http_request_t>& request)
		{
			const auto handle_id = request->handle.get_entity_id();

			if (!request->result.has_value())
			{
				scripting::notify(handle_id, "done", {{}, false, "unknown error"});
				return;
			}

			auto& result = request->result.value();
			const auto error = curl_easy_strerror(result.code);

			if (result.code != CURLE_OK)
			{
				scripting::notify(handle_id, "done", {{}, false, error});
				return;
			}

			if (result.buffer.size() >= max_result_size)
			{
				printf("^3WARNING: http result size bigger than %i bytes (%i), truncating!", max_result_size,
					static_cast<int>(result.buffer.size()));
				result.buffer.resize(max_result_size);
			}

			scripting::notify(handle_id, "done", {result.buffer, true});
		}

		void check_requests()
		{
			for (auto i = requests.begin(); i != requests.end(); )
			{
				auto& request = *i;
				if (!request->completed)
				{
					++i;
					continue;
				}
				else
				{
					notify_request_result(request);
					i = requests.erase(i);
				}
			}
		}

		scripting::object create_request(const http_request_params_t& params)
		{
			const auto request = std::make_shared<http_request_t>();
			requests.emplace_back(request);

			request->params = params;
			scheduler::thread_pool.push([request]
			{
				request->result = utils::http::get_data(
					request->params.url,
					request->params.fields,
					request->params.headers,
					request->params.method);
				request->completed = true;
			});

			return request->handle;
		}

		template <typename T>
		void push_request(T&& r)
		{
			const auto request = std::make_shared<http_request_t>(std::forward<T>(r));
			requests.emplace_back(request);
		}

		void wait_and_clear_requests()
		{
			for (auto& task : requests)
			{
				while (!task->completed)
				{
					std::this_thread::sleep_for(10ms);
				}
			}

			requests.clear();
		}

		void parse_request_options(http_request_params_t& params, const scripting::array& options)
		{
			const auto fields = options["parameters"];
			const auto body = options["body"];
			const auto headers = options["headers"];
			const auto method = options["method"];

			if (method.is<std::string>())
			{
				params.method = method.as<std::string>();
			}

			if (fields.is<scripting::array>())
			{
				const auto fields_ = fields.as<scripting::array>();
				const auto keys = fields_.get_keys();

				for (const auto& key : keys)
				{
					if (!key.is<std::string>())
					{
						continue;
					}

					const auto key_ = key.as<std::string>();
					const auto value = fields_[key].to_string();
					params.fields += key_ + "=" + value + "&";
				}

			}
			else if (body.is<std::string>())
			{
				params.fields = body.as<std::string>();
			}

			if (headers.is<scripting::array>())
			{
				const auto headers_arr = headers.as<scripting::array>();
				const auto keys = headers_arr.get_keys();

				for (const auto& key : keys)
				{
					if (!key.is<std::string>())
					{
						continue;
					}

					const auto key_str = key.as<std::string>();
					const auto value = headers_arr[key].to_string();

					params.headers[key_str] = value;
				}
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void on_shutdown([[maybe_unused]] plugin::plugin* plugin) override
		{
			scripting::on_shutdown(wait_and_clear_requests);
		}

		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			scheduler::loop(check_requests, scheduler::server);
			scripting::on_shutdown(wait_and_clear_requests);

			gsc::function::add_multiple([](const std::string& url)
			{
				http_request_params_t params{};
				params.url = url;
				return create_request(params);
			}, "http::get", "httpget", "curl");

			gsc::function::add("http::request", [](const std::string& url, const scripting::variadic_args& va)
			{
				http_request_params_t params{};
				params.url = url;

				if (va.size() > 0)
				{
					const auto options = va[0].as<scripting::array>();
					parse_request_options(params, options);
				}

				return create_request(params);
			});

			gsc::function::add_multiple([](const std::string& url, const scripting::variadic_args& va)
			{
				http_request_params_t params{};
				params.url = url;
				params.method = "POST";

				if (va.size() > 0)
				{
					const auto options = va[0].as<scripting::array>();
					parse_request_options(params, options);
				}

				return create_request(params);
			}, "httppost", "http::post");
		}
	};
}

REGISTER_COMPONENT(http::component)

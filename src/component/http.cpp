#include "stdafx.hpp"

namespace http
{
	namespace
	{
		struct request
		{
			std::string url;
			std::function<void(const std::string&)> callback;
		};

		std::mutex mutex;
		std::queue<request> request_queue;

		void run_request(request r)
		{
			std::string command("curl -s " + r.url);
			const auto result = io::execute_command(command.data());

			r.callback(result);
		}

		void start_thread()
		{
			std::thread ([]()
			{
				while (true)
				{
					std::this_thread::sleep_for(50ms);

					std::queue<request> request_queue_copy;

					{
						std::lock_guard<std::mutex> _(mutex);
						request_queue_copy = std::move(request_queue);
						request_queue = {};
					}

					while (!request_queue_copy.empty())
					{
						run_request(request_queue_copy.front());
						request_queue_copy.pop();
					}
				}
			}).detach();
		}
	}

	void add_request(const std::string& url, std::function<void(const std::string&)> callback)
	{
		std::lock_guard<std::mutex> _(mutex);

		request r;
		r.url = url;
		r.callback = callback;

		request_queue.push(r);
	}

	void init()
	{
		start_thread();

		function::add("curl", 1, 1, []()
		{
			const auto url = game::get<std::string>(0);

			const auto id = game::AllocObject(game::SCRIPTINSTANCE_SERVER);
			game::Scr_AddObject(game::SCRIPTINSTANCE_SERVER, id);
			game::RemoveRefToObject(game::SCRIPTINSTANCE_SERVER, id);

			http::add_request(url, [id](const std::string& result)
			{
				const auto name = game::SL_GetString("done", 0);

				scheduler::once([result, name, id]()
				{
					game::add(result.data());
					game::Scr_NotifyId(game::SCRIPTINSTANCE_SERVER, 0, id, name, 1);
				});
			});
		});
	}
}
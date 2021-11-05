#pragma once

namespace scheduler
{
	enum pipeline
	{
		server,
		async,
		count,
	};

	static const bool cond_continue = false;
	static const bool cond_end = true;

	void schedule(const std::function<bool()>& callback, pipeline type = pipeline::server,
		std::chrono::milliseconds delay = 0ms);
	void loop(const std::function<void()>& callback, pipeline type = pipeline::server,
		std::chrono::milliseconds delay = 0ms);
	void once(const std::function<void()>& callback, pipeline type = pipeline::server,
		std::chrono::milliseconds delay = 0ms);
}

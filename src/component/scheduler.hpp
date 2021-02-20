#pragma once

namespace scheduler
{
	void schedule(const std::function<bool()>& callback, const std::chrono::milliseconds delay = 0ms);

	void loop(const std::function<void()>& callback, const std::chrono::milliseconds delay = 0ms);
	void once(const std::function<void()>& callback, const std::chrono::milliseconds delay = 0ms);

	void init();
}

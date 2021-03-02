#pragma once

namespace scheduler
{
	void once(const std::function<void()>& callback);

	void init();
}

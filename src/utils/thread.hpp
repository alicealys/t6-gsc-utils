#pragma once
#include <thread>

namespace utils::thread
{
	template <typename ...Args>
	std::thread create_named_thread(const std::string& name, Args&&... args)
	{
		auto t = std::thread(std::forward<Args>(args)...);
		set_name(t, name);
		return t;
	}

	std::vector<DWORD> get_thread_ids();
	void for_each_thread(const std::function<void(HANDLE)>& callback);

	void suspend_other_threads();
	void resume_other_threads();
}

#include "stdafx.hpp"

namespace scheduler
{
	namespace
	{
		int glass_update;
		std::mutex mutex;

		struct task_t
		{
			int index;
			std::function<bool()> callback;
			std::chrono::milliseconds interval{};
			std::chrono::high_resolution_clock::time_point last_call{};
		};

		std::vector<task_t> tasks;

		void execute()
		{
			for (auto &task : tasks)
			{
				const auto now = std::chrono::high_resolution_clock::now();
				const auto diff = now - task.last_call;

				if (diff < task.interval)
				{
					continue;
				}

				task.last_call = now;

				const auto res = task.callback();

				if (res)
				{
					tasks.erase(tasks.begin() + task.index);
				}
			}
		}

		__declspec(naked) void server_frame()
		{
			__asm
			{
				call execute;
				push glass_update;
				retn;
			}
		}
	}

	void schedule(const std::function<bool()>& callback, const std::chrono::milliseconds delay)
	{
		std::lock_guard _(mutex);

		task_t task;
		task.index = tasks.size();
		task.callback = callback;
		task.interval = delay;
		task.last_call = std::chrono::high_resolution_clock::now();

		tasks.push_back(task);
	}

	void loop(const std::function<void()>& callback, const std::chrono::milliseconds delay)
	{
		schedule([callback]()
		{
			callback();
			return false;
		}, delay);
	}

	void once(const std::function<void()>& callback, const std::chrono::milliseconds delay)
	{
		schedule([callback]()
		{
			callback();
			return true;
		}, delay);
	}

	void init()
	{
		glass_update = SELECT(0x49E910, 0x5001A0);
		utils::hook::jump(SELECT(0x4A59F7, 0x6AA2F7), server_frame);
	}
}
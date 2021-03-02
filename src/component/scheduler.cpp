#include "stdafx.hpp"

namespace scheduler
{
	namespace
	{
		int glass_update;
		std::mutex mutex;

		std::queue<std::function<void()>> tasks;

		void execute()
		{
			while (!tasks.empty())
			{
				const auto task = tasks.front();

				const auto now = std::chrono::high_resolution_clock::now();

				task();
				tasks.pop();
			}
		}

		__declspec(naked) void server_frame()
		{
			__asm
			{
				pushad
				call execute
				popad

				push glass_update
				retn
			}
		}
	}

	void once(const std::function<void()>& callback)
	{
		tasks.push(callback);
	}

	void init()
	{
		glass_update = SELECT(0x49E910, 0x5001A0);
		utils::hook::jump(SELECT(0x4A59F7, 0x6AA2F7), server_frame);
	}
}
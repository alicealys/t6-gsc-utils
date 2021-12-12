#include <stdinc.hpp>
#include "thread.hpp"
#include "string.hpp"

#include <TlHelp32.h>

#include <gsl/gsl>

namespace utils::thread
{
	std::vector<DWORD> get_thread_ids()
	{
		auto* const h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
		if (h == INVALID_HANDLE_VALUE)
		{
			return {};
		}

		const auto _ = gsl::finally([h]()
		{
			CloseHandle(h);
		});

		THREADENTRY32 entry{};
		entry.dwSize = sizeof(entry);
		if (!Thread32First(h, &entry))
		{
			return {};
		}

		std::vector<DWORD> ids{};

		do
		{
			const auto check_size = entry.dwSize < FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID)
				+ sizeof(entry.th32OwnerProcessID);
			entry.dwSize = sizeof(entry);

			if (check_size && entry.th32OwnerProcessID == GetCurrentProcessId())
			{
				ids.emplace_back(entry.th32ThreadID);
			}
		}
		while (Thread32Next(h, &entry));

		return ids;
	}

	void for_each_thread(const std::function<void(HANDLE)>& callback)
	{
		const auto ids = get_thread_ids();

		for (const auto& id : ids)
		{
			auto* const thread = OpenThread(THREAD_ALL_ACCESS, FALSE, id);
			if (thread != nullptr)
			{
				const auto _ = gsl::finally([thread]()
				{
					CloseHandle(thread);
				});

				callback(thread);
			}
		}
	}

	void suspend_other_threads()
	{
		for_each_thread([](const HANDLE thread)
		{
			if (GetThreadId(thread) != GetCurrentThreadId())
			{
				SuspendThread(thread);
			}
		});
	}

	void resume_other_threads()
	{
		for_each_thread([](const HANDLE thread)
		{
			if (GetThreadId(thread) != GetCurrentThreadId())
			{
				ResumeThread(thread);
			}
		});
	}
}

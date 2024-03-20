#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace utils::io
{
	class file_watcher
	{
	public:
		file_watcher(const std::string& path);

		file_watcher(file_watcher&&) noexcept = delete;
		file_watcher& operator=(file_watcher&&) noexcept = delete;

		file_watcher(const file_watcher&) = delete;
		file_watcher& operator=(const file_watcher&) = delete;

		~file_watcher();

		bool init();
		bool run(int wait_time, std::string* data, bool force_check);
		bool run(const std::chrono::high_resolution_clock::duration& wait_time, 
			std::string* data, bool force_check = false);
		bool run_blocking(std::string* data, bool force_check = false);
		void stop();

	private:
		std::string path_{};
		std::string filename_{};

		std::ifstream stream_{};
		size_t current_pos_{};

#ifdef WIN32
		HANDLE handle_{};
		HANDLE event_handle_{};
		HANDLE stop_handle_{};
#endif

		bool stop_{};

	};
}

#include <stdinc.hpp>
#include <fstream>

#include "string.hpp"
#include "file_watcher.hpp"

namespace utils::io
{
	file_watcher::file_watcher(const std::string& path)
	{
		this->path_ = path;
		const auto pos = this->path_.find_last_of("/\\");
		if (pos != std::string::npos)
		{
			this->filename_ = path.substr(pos + 1);
		}
		else
		{
			this->filename_ = path;
		}
	}

	file_watcher::~file_watcher()
	{
#ifdef WIN32
		const auto close = [](HANDLE handle)
		{
			if (handle != NULL && handle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(handle);
			}
		};
		
		close(this->handle_);
		close(this->event_handle_);
		close(this->stop_handle_);
#endif
	}

	bool file_watcher::init()
	{
#ifdef WIN32
		std::string folder{};
		const auto pos = this->path_.find_last_of("/\\");
		if (pos != std::string::npos)
		{
			folder = this->path_.substr(0, pos);
		}

		this->handle_ = CreateFile(
			folder.data(), 
			FILE_LIST_DIRECTORY,
			FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
			NULL, 
			OPEN_EXISTING, 
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL
		);
		this->event_handle_ = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->stop_handle_ = CreateEvent(NULL, TRUE, FALSE, NULL);

		const auto is_valid = [](HANDLE handle)
		{
			return handle != NULL && handle != INVALID_HANDLE_VALUE;
		};

		if (!is_valid(this->event_handle_) ||
			!is_valid(this->handle_ ) ||
			!is_valid(this->stop_handle_))
		{
			return false;
		}

		this->stream_ = std::ifstream{this->path_, std::ios::binary};
		this->stream_.seekg(0, std::ios::end);
		this->current_pos_ = static_cast<size_t>(this->stream_.tellg());

		return true;
#endif
	}

	bool file_watcher::run(int wait_time, std::string* data, bool force_check)
	{
#ifdef WIN32
		this->stop_ = false;

		char buffer[0x1000] = {0};
		OVERLAPPED overlapped = {};
		overlapped.hEvent = this->event_handle_;

		ReadDirectoryChangesW(this->handle_, buffer, sizeof(buffer),
			FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &overlapped, NULL);

		HANDLE handles[2] =
		{
			this->event_handle_,
			this->stop_handle_
		};

		WaitForMultipleObjects(2, handles, FALSE, wait_time);
		if (this->stop_)
		{
			return false;
		}

		const auto info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
		const auto filename = utils::string::convert(info->FileName);

		if (!force_check && (filename != this->filename_ || (info->Action & FILE_GENERIC_WRITE) == 0))
		{
			return false;
		}

		this->stream_.seekg(0, std::ios::end);
		const auto new_size = static_cast<size_t>(this->stream_.tellg());

		if (new_size < this->current_pos_)
		{
			this->current_pos_ = new_size;
			return false;
		}

		const auto bytes_written = new_size - this->current_pos_;
		if (bytes_written == 0)
		{
			return false;
		}

		this->stream_.seekg(-1 * static_cast<int64_t>(bytes_written), std::ios::cur);
		this->current_pos_ += bytes_written;
		
		data->resize(bytes_written);
		this->stream_.read(const_cast<char*>(data->data()), bytes_written);
		return true;
#endif
	}

	bool file_watcher::run_blocking(std::string* data, bool force_check)
	{
#ifdef WIN32
		return this->run(INFINITE, data, force_check);
#endif
	}

	bool file_watcher::run(const std::chrono::high_resolution_clock::duration& wait_time, std::string* data, bool force_check)
	{
#ifdef WIN32
		const auto msec = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(wait_time).count());
		return this->run(msec, data, force_check);
#endif
	}

	void file_watcher::stop()
	{
		this->stop_ = true;
#ifdef WIN32
		SetEvent(this->stop_handle_);
#endif
	}
}

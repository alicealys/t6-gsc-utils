#include <stdinc.hpp>
#include "http.hpp"
#include <gsl/gsl>

#pragma comment(lib, "ws2_32.lib")

namespace utils::http
{
	namespace
	{
		struct progress_helper
		{
			const std::function<void(size_t)>* callback{};
			std::exception_ptr exception{};
		};

		int progress_callback(void* clientp, const curl_off_t /*dltotal*/, const curl_off_t dlnow, const curl_off_t /*ultotal*/, const curl_off_t /*ulnow*/)
		{
			auto* helper = static_cast<progress_helper*>(clientp);

			try
			{
				if (*helper->callback)
				{
					(*helper->callback)(static_cast<size_t>(dlnow));
				}
			}
			catch (...)
			{
				helper->exception = std::current_exception();
				return -1;
			}

			return 0;
		}

		size_t write_callback(void* contents, const size_t size, const size_t nmemb, void* userp)
		{
			auto* buffer = static_cast<std::string*>(userp);

			const auto total_size = size * nmemb;
			buffer->append(static_cast<char*>(contents), total_size);
			return total_size;
		}
	}

	std::optional<std::string> get_data(const std::string& url, const headers& headers, const std::function<void(size_t)>& callback)
	{
		curl_slist* header_list = nullptr;
		auto* curl = curl_easy_init();
		if (!curl)
		{
			return {};
		}

		auto _ = gsl::finally([&]()
		{
			curl_slist_free_all(header_list);
			curl_easy_cleanup(curl);
		});

		for (const auto& header : headers)
		{
			auto data = header.first + ": " + header.second;
			header_list = curl_slist_append(header_list, data.data());
		}

		std::string buffer{};
		progress_helper helper{};
		helper.callback = &callback;

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.data());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &helper);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

		if (curl_easy_perform(curl) == CURLE_OK)
		{
			return { std::move(buffer) };
		}

		if (helper.exception)
		{
			std::rethrow_exception(helper.exception);
		}

		return {};
	}

	std::optional<std::string> post_data(const std::string& url, const std::string& data, const headers& headers, const std::function<void(size_t)>& callback)
	{
		curl_slist* header_list = nullptr;

		auto* curl = curl_easy_init();
		if (!curl) return {};

		auto _ = gsl::finally([&]()
		{
			curl_slist_free_all(header_list);
			curl_easy_cleanup(curl);
		});

		for (const auto& header : headers)
		{
			const auto header_string = header.first + ": " + header.second;
			header_list = curl_slist_append(header_list, header_string.data());
		}

		std::string buffer{};
		progress_helper helper{};
		helper.callback = &callback;

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.data());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.data());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &helper);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

		if (curl_easy_perform(curl) == CURLE_OK)
		{
			return { std::move(buffer) };
		}

		if (helper.exception)
		{
			std::rethrow_exception(helper.exception);
		}

		return {};
	}

	std::future<std::optional<std::string>> get_data_async(const std::string& url, const headers& headers)
	{
		return std::async(std::launch::async, [url, headers]()
		{
			return get_data(url, headers);
		});
	}

	std::future<std::optional<std::string>> post_data_async(const std::string& url, const std::string& data, const headers& headers)
	{
		return std::async(std::launch::async, [url, data, headers]()
		{
			return post_data(url, data, headers);
		});
	}

	std::optional<result> get_data_result(const std::string& url, const std::string& fields,
		const headers& headers, const std::function<void(size_t)>& callback)
	{
		curl_slist* header_list = nullptr;
		auto* curl = curl_easy_init();
		if (!curl)
		{
			return {};
		}

		auto _ = gsl::finally([&]()
		{
			curl_slist_free_all(header_list);
			curl_easy_cleanup(curl);
		});

		for (const auto& header : headers)
		{
			auto data = header.first + ": " + header.second;
			header_list = curl_slist_append(header_list, data.data());
		}

		std::string buffer{};
		progress_helper helper{};
		helper.callback = &callback;

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.data());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &helper);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

		if (!fields.empty())
		{
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields.data());
		}

		const auto code = curl_easy_perform(curl);

		if (code == CURLE_OK)
		{
			result result;
			result.code = code;
			result.buffer = std::move(buffer);

			return result;
		}
		else
		{
			result result;
			result.code = code;

			return result;
		}
	}
}
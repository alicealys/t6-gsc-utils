#include <stdinc.hpp>
#include "http.hpp"

#include <gsl/gsl>

#pragma comment(lib, "ws2_32.lib")

namespace utils::http
{
	namespace
	{
		size_t write_callback(void* contents, const size_t size, const size_t nmemb, void* userp)
		{
			const auto buffer = static_cast<std::string*>(userp);
			const auto total_size = size * nmemb;
			buffer->append(static_cast<char*>(contents), total_size);
			return total_size;
		}
	}

	std::optional<result> get_data(const std::string& url, const std::string& fields,
		const headers& headers, const std::string& method)
	{
		curl_slist* header_list = nullptr;
		const auto curl = curl_easy_init();
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

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.data());

		if (!fields.empty())
		{
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields.data());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, fields.size());
		}

		if (!method.empty())
		{
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.data());
		}

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

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

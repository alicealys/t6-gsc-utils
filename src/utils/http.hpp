#pragma once

#include <string>
#include <optional>

#include <curl/curl.h>

namespace utils::http
{
	struct result
	{
		CURLcode code;
		std::string buffer;
	};

	using headers = std::unordered_map<std::string, std::string>;

	std::optional<result> get_data(const std::string& url, const std::string& fields = {},
		const headers& headers = {}, const std::string& method = {});
}

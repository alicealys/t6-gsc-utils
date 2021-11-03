#include "stdafx.hpp"

namespace http
{
	void add_request(const std::string& url, std::function<void(const std::string&)> callback);
}
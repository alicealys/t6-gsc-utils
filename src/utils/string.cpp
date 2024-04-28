#include <stdinc.hpp>
#include "string.hpp"
#include <sstream>
#include <cstdarg>
#include <algorithm>

namespace utils::string
{
	const char* va(const char* fmt, ...)
	{
		static thread_local va_provider<8, 256> provider;

		va_list ap;
		va_start(ap, fmt);

		const char* result = provider.get(fmt, ap);

		va_end(ap);
		return result;
	}

	std::vector<std::string> split(const std::string& s, const char delim)
	{
		std::stringstream ss(s);
		std::string item;
		std::vector<std::string> elems;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item); // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
		}

		return elems;
	}

	std::string to_lower(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](const unsigned char input)
		{
				return static_cast<char>(tolower(input));
		});

		return text;
	}

	std::string to_upper(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](const unsigned char input)
		{
				return static_cast<char>(toupper(input));
		});

		return text;
	}

	bool starts_with(const std::string& text, const std::string& substring)
	{
		return text.find(substring) == 0;
	}

	bool ends_with(const std::string& text, const std::string& substring)
	{
		if (substring.size() > text.size()) return false;
		return std::equal(substring.rbegin(), substring.rend(), text.rbegin());
	}

	bool is_numeric(const std::string& text)
	{
		return !text.empty() && std::find_if(text.begin(),
			text.end(),
			[](unsigned char input)
		{
			return !std::isdigit(input);
		}) == text.end();
	}

	std::string dump_hex(const std::string& data, const std::string& separator)
	{
		std::string result;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			if (i > 0)
			{
				result.append(separator);
			}

			result.append(va("%02X", data[i] & 0xFF));
		}

		return result;
	}

	std::string bin_to_hex(const std::string& data)
	{
		return dump_hex(data, "");
	}

	std::string hex_to_bin(const std::string& data)
	{
		std::string result;

		for (auto i = 0u; i < data.size(); i += 2)
		{
			const auto byte = data.substr(i, 2);
			const auto value = static_cast<char>(std::strtol(byte.data(), nullptr, 0x10));
			result += value;
		}

		return result;
	}

	std::string convert(const std::wstring& wstr)
	{
		std::string result;
		result.reserve(wstr.size());

		for (const auto& chr : wstr)
		{
			result.push_back(static_cast<char>(chr));
		}

		return result;
	}

	std::wstring convert(const std::string& str)
	{
		std::wstring result;
		result.reserve(str.size());

		for (const auto& chr : str)
		{
			result.push_back(static_cast<wchar_t>(chr));
		}

		return result;
	}

	std::string replace(std::string str, const std::string& from, const std::string& to)
	{
		if (from.empty())
		{
			return str;
		}

		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}

		return str;
	}

	std::string get_timestamp()
	{
		tm ltime{};
		char timestamp[MAX_PATH] = { 0 };
		const auto time = _time64(nullptr);

		_localtime64_s(&ltime, &time);
		strftime(timestamp, sizeof(timestamp) - 1, "%Y-%m-%d-%H-%M-%S", &ltime);

		return timestamp;
	}

	std::string trim(const std::string& str, const std::string& whitespace)
	{
		const auto first = str.find_first_not_of(whitespace);
		if (first == std::string::npos)
		{
			return {};
		}

		const auto last = str.find_last_not_of(whitespace);
		const auto range = last - first + 1;

		return str.substr(first, range);
	}
}
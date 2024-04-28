#include <stdinc.hpp>

#include "game/game.hpp"
#include "signatures.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <Psapi.h>

namespace signatures
{
	size_t load_image_size()
	{
		MODULEINFO info{};
		GetModuleInformation(GetCurrentProcess(),
			GetModuleHandle("plutonium-bootstrapper-win32.exe"), &info, sizeof(MODULEINFO));
		return info.SizeOfImage;
	}

	size_t get_image_size()
	{
		static const auto image_size = load_image_size();
		return image_size;
	}

	size_t find_string_ptr(const std::string& string)
	{
		const char* string_ptr = nullptr;
		std::string mask(string.size(), 'x');

		constexpr auto payload_size = 0x12000000;
		const auto base = reinterpret_cast<size_t>(GetModuleHandle("plutonium-bootstrapper-win32.exe")) + payload_size;

		utils::hook::signature signature(base, get_image_size() - base);

		signature.add({
			string,
			mask,
			[&](char* address)
			{
				string_ptr = address;
			}
		});

		signature.process();
		return reinterpret_cast<size_t>(string_ptr);
	}

	size_t find_string_ref(const std::string& string)
	{
		char bytes[4] = {0};
		const auto string_ptr = find_string_ptr(string);
		memcpy(bytes, &string_ptr, sizeof(bytes));
		return find_string_ptr({bytes, 4});
	}

	bool process()
	{
		return true;
	}
}

#include <stdafx.hpp>

namespace utils::hook
{
	void get(uint64_t address, LPVOID buffer, size_t size)
	{
		DWORD oldProtect = 0;

		auto* place = reinterpret_cast<void*>(address);
		VirtualProtect(place, size, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(buffer, place, size);
		VirtualProtect(place, size, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), place, size);
	}

	void set(std::uintptr_t address, std::vector<std::uint8_t>&& bytes)
	{
		DWORD oldProtect = 0;

		auto* place = reinterpret_cast<void*>(address);
		VirtualProtect(place, bytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(place, bytes.data(), bytes.size());
		VirtualProtect(place, bytes.size(), oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), place, bytes.size());
	}

	void set(std::uintptr_t address, void* buffer, size_t size)
	{
		DWORD oldProtect = 0;

		auto* place = reinterpret_cast<void*>(address);
		VirtualProtect(place, size, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(place, buffer, size);
		VirtualProtect(place, size, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), place, size);
	}

	void nop(std::uintptr_t address, size_t size)
	{
		std::uint8_t* bytes = new std::uint8_t[size];
		memset(bytes, 0x90, size);

		set(address, bytes, size);

		delete[] bytes;
	}

	void call(std::uintptr_t address, void* destination)
	{
		if (!address) return;

		std::uint8_t* bytes = new std::uint8_t[5];
		*bytes = 0xE8;
		*reinterpret_cast<std::uint32_t*>(bytes + 1) = CalculateRelativeJMPAddress(address, destination);

		set(address, bytes, 5);

		delete[] bytes;
	}

	void jump(std::uintptr_t address, void* destination)
	{
		if (!address) return;

		std::uint8_t* bytes = new std::uint8_t[5];
		*bytes = 0xE9;
		*reinterpret_cast<std::uint32_t*>(bytes + 1) = CalculateRelativeJMPAddress(address, destination);

		set(address, bytes, 5);

		delete[] bytes;
	}
}
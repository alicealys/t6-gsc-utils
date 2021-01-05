#pragma once

#define CalculateRelativeJMPAddress(X, Y) (((std::uintptr_t)Y - (std::uintptr_t)X) - 5)

namespace utils::hook
{
	void get(uint64_t address, LPVOID buffer, size_t size);
	void set(std::uintptr_t address, std::vector<std::uint8_t>&& bytes);
	void set(std::uintptr_t address, void* buffer, size_t size);

	void nop(std::uintptr_t address, size_t size);
	void call(std::uintptr_t address, void* destination);
	void jump(std::uintptr_t address, void* destination);

	template<typename T> void set(std::uintptr_t address, T data)
	{
		set(address, &data, sizeof(T));
	}
}
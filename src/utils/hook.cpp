#include <stdinc.hpp>
#include "hook.hpp"
#include "string.hpp"
// iw6x-client

namespace utils::hook
{
	void signature::process()
	{
		if (this->signatures_.empty()) return;

		const auto start = static_cast<char*>(this->start_);

		const unsigned int sig_count = this->signatures_.size();
		const auto containers = this->signatures_.data();

		for (size_t i = 0; i < this->length_; ++i)
		{
			const auto address = start + i;

			for (unsigned int k = 0; k < sig_count; ++k)
			{
				const auto container = &containers[k];

				unsigned int j;
				for (j = 0; j < static_cast<unsigned int>(container->mask.size()); ++j)
				{
					if (container->mask[j] != '?' && container->signature[j] != address[j])
					{
						break;
					}
				}

				if (j == container->mask.size())
				{
					container->callback(address);
				}
			}
		}
	}

	void signature::add(const container& container)
	{
		signatures_.push_back(container);
	}

	namespace
	{
		[[maybe_unused]] class _
		{
		public:
			_()
			{
				if (MH_Initialize() != MH_OK)
				{
					throw std::runtime_error("Failed to initialize MinHook");
				}
			}

			~_()
			{
				MH_Uninitialize();
			}
		} __;
	}

	asmjit::Error assembler::call(void* target)
	{
		return Assembler::call(size_t(target));
	}

	asmjit::Error assembler::jmp(void* target)
	{
		return Assembler::jmp(size_t(target));
	}

	detour::detour(const size_t place, void* target) : detour(reinterpret_cast<void*>(place), target)
	{
	}

	detour::detour(void* place, void* target)
	{
		this->create(place, target);
	}

	detour::~detour()
	{
		this->clear();
	}

	void detour::enable() const
	{
		MH_EnableHook(this->place_);
	}

	void detour::disable() const
	{
		MH_DisableHook(this->place_);
	}

	void detour::create(void* place, void* target)
	{
		this->clear();
		this->place_ = place;

		if (MH_CreateHook(this->place_, target, &this->original_) != MH_OK)
		{
			throw std::runtime_error(string::va("Unable to create hook at location: %p", this->place_));
		}

		this->enable();
	}

	void detour::create(const size_t place, void* target)
	{
		this->create(reinterpret_cast<void*>(place), target);
	}

	void detour::clear()
	{
		if (this->place_)
		{
			MH_RemoveHook(this->place_);
		}

		this->place_ = nullptr;
		this->original_ = nullptr;
	}

	void* detour::get_original() const
	{
		return this->original_;
	}

	void nop(void* place, const size_t length)
	{
		DWORD old_protect{};
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memset(place, 0x90, length);

		VirtualProtect(place, length, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, length);
	}

	void nop(const size_t place, const size_t length)
	{
		nop(reinterpret_cast<void*>(place), length);
	}

	void copy(void* place, const void* data, const size_t length)
	{
		DWORD old_protect{};
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memmove(place, data, length);

		VirtualProtect(place, length, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, length);
	}

	void copy(const size_t place, const void* data, const size_t length)
	{
		copy(reinterpret_cast<void*>(place), data, length);
	}

	bool is_relatively_far(const void* pointer, const void* data, int offset)
	{
		const int64_t diff = size_t(data) - (size_t(pointer) + offset);
		const auto small_diff = int32_t(diff);
		return diff != int64_t(small_diff);
	}

	void call(void* pointer, void* data)
	{
		if (is_relatively_far(pointer, data))
		{
			throw std::runtime_error("Too far away to create 32bit relative branch");
		}

		auto* patch_pointer = PBYTE(pointer);
		set<uint8_t>(patch_pointer, 0xE8);
		set<int32_t>(patch_pointer + 1, int32_t(size_t(data) - (size_t(pointer) + 5)));
	}

	void call(const size_t pointer, void* data)
	{
		return call(reinterpret_cast<void*>(pointer), data);
	}

	void call(const size_t pointer, const size_t data)
	{
		return call(pointer, reinterpret_cast<void*>(data));
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

	void jump(std::uintptr_t address, void* destination)
	{
		if (!address) return;

		std::uint8_t* bytes = new std::uint8_t[5];
		*bytes = 0xE9;
		*reinterpret_cast<std::uint32_t*>(bytes + 1) = CalculateRelativeJMPAddress(address, destination);

		set(address, bytes, 5);

		delete[] bytes;
	}

	void* assemble(const std::function<void(assembler&)>& asm_function)
	{
		static asmjit::JitRuntime runtime;

		asmjit::CodeHolder code;
		code.init(runtime.environment());

		assembler a(&code);

		asm_function(a);

		void* result = nullptr;
		runtime.add(&result, &code);

		return result;
	}
}
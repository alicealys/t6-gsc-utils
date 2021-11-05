#pragma once

namespace command
{
	class params
	{
	public:
		int size();
		const char* get(int index);
		std::string join(int index);

		const char* operator[](const int index)
		{
			return this->get(index);
		}
	};

	void add_raw(const char* name, void (*callback)());
	void add(const char* name, std::function<void(params&)> callback);
}
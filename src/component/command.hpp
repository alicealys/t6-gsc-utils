#pragma once

namespace command
{
	class params
	{
	public:
		params();

		int size() const;
		const char* get(int index) const;
		std::string join(int index) const;

		const char* operator[](const int index) const
		{
			return this->get(index);
		}

	private:
		int nesting_;
	};

	void add_raw(const char* name, void (*callback)());
	void add(const char* name, std::function<void(params&)> callback);

	void add_script_command(const std::string& name, const std::function<void(const params&)>& callback);
	void clear_script_commands();

	void execute(std::string command, const bool sync = false);
}
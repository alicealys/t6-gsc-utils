#pragma once

namespace command
{
	class params
	{
	public:
		params();

		[[nodiscard]] int size() const;
		[[nodiscard]] const char* get(int index) const;
		[[nodiscard]] std::string join(int index) const;

		[[nodiscard]] const char* operator[](const int index) const
		{
			return this->get(index);
		}

	private:
		int nesting_;
	};

	class params_sv
	{
	public:
		params_sv();

		[[nodiscard]] int size() const;
		[[nodiscard]] const char* get(int index) const;
		[[nodiscard]] std::string join(int index) const;
		[[nodiscard]] std::vector<std::string> get_all() const;

		[[nodiscard]] const char* operator[](const int index) const
		{
			return this->get(index);
		}

	private:
		int nesting_;
	};

	void add_raw(const char* name, void (*callback)());
	void add(const char* name, const std::function<void(params&)>& callback);

	void add_sv(const std::string& name, const std::function<void(int, const params_sv&)>& callback);

	void add_script_command(const std::string& name, const std::function<void(const params&)>& callback);
	void clear_script_commands();

	void execute(std::string command, bool sync = false);
}
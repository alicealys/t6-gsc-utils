#pragma once
#include "entity.hpp"
#include "script_value.hpp"

namespace scripting
{
	class thread
	{
	public:
		thread(unsigned int);

		script_value get_raw() const;

		unsigned int get_entity_id() const;
		unsigned int get_type() const;

		unsigned int get_self() const;
		unsigned int get_wait_time() const;
		unsigned int get_notify_name_id() const;
		std::string get_notify_name() const;

		const char* get_pos() const;
		const char* get_start_pos() const;

		void kill() const;

	private:
		unsigned int id_{};
		unsigned int type_{};

	};
}

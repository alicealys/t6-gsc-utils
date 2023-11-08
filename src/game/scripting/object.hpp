#pragma once
#include "script_value.hpp"

namespace scripting
{
	class object_value : public script_value
	{
	public:
		object_value(const std::string& key, unsigned int, unsigned int);
		void operator=(const script_value&);

		template <typename T>
		T as() const
		{
			try
			{
				return script_value::as<T>();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(std::format("object field '{}' {}", this->key_, e.what()));
			}
		}

	private:
		std::string key_;
		unsigned int id_;
		unsigned int parent_id_;

	};

	class object final
	{
	public:
		object();
		object(const unsigned int);

		object(std::unordered_map<std::string, script_value>);

		object(const object& other);
		object(object&& other) noexcept;

		~object();

		object& operator=(const object& other);
		object& operator=(object&& other) noexcept;

		std::vector<std::string> get_keys() const;
		unsigned int size() const;
		void erase(const std::string&) const;

		script_value get(const std::string&) const;
		void set(const std::string&, const script_value&) const;

		unsigned int get_entity_id() const;
		unsigned int get_value_id(const std::string&) const;

		entity get_raw() const;

		object_value operator[](const std::string& key) const
		{
			return {key, this->id_, this->get_value_id(key)};
		}

	private:
		void add() const;
		void release() const;

		unsigned int id_{};
	};
}

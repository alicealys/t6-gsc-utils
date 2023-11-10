#pragma once

#include "script_value.hpp"
#include "container_iterator.hpp"

namespace scripting
{
	class object;

	class object_value : public script_value
	{
	public:
		object_value(const object* object, const std::string& key);
		void operator=(const script_value& value);

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
		const object* object_;
		std::string key_;

	};

	template <typename IteratorType>
	class object_iterator_base : public IteratorType
	{
	public:
		object_iterator_base(const object* container)
			: IteratorType(container)
		{
		}

		object_iterator_base(const object* container, const std::vector<std::string>& keys, const std::int64_t key_index)
			: IteratorType(container, keys, key_index)
		{
		}
	};

	using object_iterator = object_iterator_base<container_iterator<object, std::string, object_value>>;

	class object final
	{
	public:
		object();
		object(const std::uint32_t id);

		object(const object& other);
		object(object&& other) noexcept;

		~object();

		object& operator=(const object& other);
		object& operator=(object&& other) noexcept;

		void iterate_keys(const std::function<bool(const std::string& key)>& callback) const;
		std::vector<std::string> get_keys() const;
		std::optional<std::string> get_next_key(const std::string& current) const;

		std::uint32_t size() const;
		void erase(const std::string& key) const;
		void erase(const object_iterator& iter) const;

		script_value get(const std::string&) const;
		void set(const std::string&, const script_value&) const;

		std::uint32_t get_entity_id() const;
		std::uint32_t get_value_id(const std::string&) const;

		entity get_raw() const;

		object_value operator[](const std::string& key) const;

		object_iterator begin() const;
		object_iterator end() const;
		object_iterator find(const std::string& key) const;

	private:
		void add() const;
		void release() const;

		std::uint32_t id_{};

	};
}

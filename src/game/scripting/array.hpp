#pragma once

#include "script_value.hpp"
#include "container_iterator.hpp"

namespace scripting
{
	class array;

	class array_value : public script_value
	{
	public:
		array_value(const array* array, const std::uint32_t id);
		void operator=(const script_value& value);

	private:
		std::uint32_t id_;
		const array* array_;

	};

	template <typename IteratorType>
	class array_iterator_base : public IteratorType
	{
	public:
		array_iterator_base(const array* container)
			: IteratorType(container)
		{
		}

		array_iterator_base(const array* container, const std::vector<script_value>& keys, const std::int64_t key_index)
			: IteratorType(container, keys, key_index)
		{
		}
	};

	using array_iterator = array_iterator_base<container_iterator<array, script_value, array_value>>;

	class array final
	{
	public:
		array();
		array(const std::uint32_t id);

		array(const array& other);
		array(array&& other) noexcept;

		~array();

		array& operator=(const array& other);
		array& operator=(array&& other) noexcept;

		std::vector<script_value> get_keys() const;
		std::uint32_t size() const;

		std::uint32_t push_back(const script_value& value) const;

		template <typename ...Args>
		void emplace_back(Args&&... args)
		{
			this->push_back(std::forward<Args>(args)...);
		}

		void erase(const std::uint32_t index) const;
		void erase(const std::string& key) const;
		void erase(const script_value& key) const;
		void erase(const array_iterator& iter) const;

		script_value get(const script_value&) const;
		script_value get(const std::string&) const;
		script_value get(const std::uint32_t) const;

		void set(const script_value&, const script_value&) const;
		void set(const std::string&, const script_value&) const;
		void set(const std::uint32_t, const script_value&) const;

		std::uint32_t get_entity_id() const;

		std::uint32_t get_value_id(const std::string&) const;
		std::uint32_t get_value_id(std::uint32_t) const;

		entity get_raw() const;

		array_value operator[](const script_value& key) const;

		array_iterator begin() const;
		array_iterator end() const;
		array_iterator find(const script_value& key) const;

	private:
		void add() const;
		void release() const;

		std::uint32_t id_{};

	};
}

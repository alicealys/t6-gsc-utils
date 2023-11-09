#pragma once

#include "script_value.hpp"

namespace scripting
{
	template <typename ContainerType, typename KeyType, typename ValueType>
	class container_iterator
	{
	public:
		static constexpr std::int64_t container_iterator_end_key = -1;

		container_iterator(const ContainerType* container)
			: container_(container)
			  , key_index_(container_iterator_end_key)
		{
		}

		container_iterator(const ContainerType* container, const std::vector<KeyType>& keys, const std::int64_t key_index)
			: container_(container)
			  , keys_(keys)
			  , key_index_(key_index)
		{
			this->update_pair();
		}

		std::pair<KeyType, ValueType>& operator*()
		{
			return this->pair_.value();
		}

		std::pair<KeyType, ValueType>* operator->()
		{
			return &this->pair_.value();
		}

		const std::pair<KeyType, ValueType>& operator*() const
		{
			return this->pair_.value();
		}

		const std::pair<KeyType, ValueType>* operator->() const
		{
			return &this->pair_.value();
		}

		container_iterator& operator++()
		{
			if (this->key_index_ == container_iterator_end_key)
			{
				return *this;
			}

			const auto size = this->keys_.size();
			if (this->key_index_ + 1 >= size)
			{
				this->key_index_ = container_iterator_end_key;
				return *this;
			}

			this->key_index_++;
			this->update_pair();

			return *this;
		}

		container_iterator operator++(int)
		{
			const auto pre = *this;
			this->operator++();
			return pre;
		}

		friend bool operator==(const container_iterator& a, const container_iterator& b)
		{
			return a.container_ == b.container_ && a.key_index_ == b.key_index_;
		};

		friend bool operator!=(const container_iterator& a, const container_iterator& b)
		{
			return a.container_ != b.container_ || a.key_index_ != b.key_index_;
		};

	private:
		void update_pair()
		{
			const auto index = static_cast<size_t>(this->key_index_);
			const auto& key = this->keys_[index];
			const auto value = this->container_->operator[](key);
			this->pair_.emplace(std::make_pair(key, value));
		}

		const ContainerType* container_;
		std::int64_t key_index_;
		std::vector<KeyType> keys_;
		std::optional<std::pair<KeyType, ValueType>> pair_;

	};
}

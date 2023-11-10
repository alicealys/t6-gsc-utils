#include <stdinc.hpp>

#include "array.hpp"
#include "execution.hpp"

namespace scripting
{
	array_value::array_value(const array* array, const script_value& key)
		: array_(array)
		  , key_(key)
	{
		const auto value = this->array_->get(key);
		this->script_value::operator=(value);
	}

	void array_value::operator=(const script_value& value)
	{
		this->array_->set(this->key_, value);
		this->script_value::operator=(value);
	}

	array::array(const std::uint32_t id)
		: id_(id)
	{
		this->add();
	}

	array::array(const array& other)
	{
		this->operator=(other);
	}

	array::array(array&& other) noexcept
	{
		this->id_ = other.id_;
		other.id_ = 0;
	}

	array::array()
	{
		this->id_ = make_array();
	}

	array::~array()
	{
		this->release();
	}

	array& array::operator=(const array& other)
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			this->add();
		}

		return *this;
	}

	array& array::operator=(array&& other) noexcept
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			other.id_ = 0;
		}

		return *this;
	}

	void array::add() const
	{
		if (this->id_)
		{
			game::VariableValue value{};
			value.u.uintValue = this->id_;
			value.type = game::SCRIPT_OBJECT;

			game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value);
		}
	}

	void array::release() const
	{
		if (this->id_)
		{
			game::VariableValue value{};
			value.u.uintValue = this->id_;
			value.type = game::SCRIPT_OBJECT;

			game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, value.type, value.u);
		}
	}

	std::vector<script_value> array::get_keys() const
	{
		std::vector<script_value> result;

		auto current = game::scr_VarGlob->objectVariableChildren[this->id_].firstChild;

		while (current)
		{
			const auto var = game::scr_VarGlob->childVariableValue[current];

			if (var.type == game::SCRIPT_NONE)
			{
				current = var.nextSibling;
				continue;
			}

			const auto string_value = static_cast<std::uint8_t>(var.name_lo) + (var.k.keys.name_hi << 8);
			const auto* str = game::SL_ConvertToString(string_value);

			script_value key;
			if (string_value < 0x40000 && str)
			{
				key = str;
			}
			else
			{
				key = (string_value - 0x800000) & 0xFFFFFF;
			}

			result.push_back(key);

			current = var.nextSibling;
		}

		return result;
	}

	std::uint32_t array::size() const
	{
		return game::Scr_GetSelf(game::SCRIPTINSTANCE_SERVER, this->id_);
	}

	std::uint32_t array::push_back(const script_value& value) const
	{
		this->set(this->size(), value);
		return this->size();
	}

	void array::erase(const std::uint32_t index) const
	{
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, (index - 0x800000) & 0xFFFFFF);
		if (variable_id)
		{
			game::RemoveVariableValue(game::SCRIPTINSTANCE_SERVER, this->id_, variable_id);
		}
	}

	void array::erase(const std::string& key) const
	{
		const auto string_value = game::SL_GetString(key.data(), 0);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);
		if (variable_id)
		{
			game::RemoveVariableValue(game::SCRIPTINSTANCE_SERVER, this->id_, variable_id);
		}
	}

	void array::erase(const script_value& key) const
	{
		if (key.is<int>())
		{
			return this->erase(key.as<int>());
		}

		if (key.is<std::string>())
		{
			return this->erase(key.as<std::string>());
		}
	}

	void array::erase(const array_iterator& iter) const
	{
		this->erase(iter->first);
	}

	script_value array::get(const std::string& key) const
	{
		const auto string_value = game::SL_GetString(key.data(), 0);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);

		if (!variable_id)
		{
			return {};
		}

		const auto value = game::scr_VarGlob->childVariableValue[variable_id];
		game::VariableValue variable{};
		variable.u = value.u.u;
		variable.type = static_cast<game::scriptType_e>(value.type);

		return variable;
	}

	script_value array::get(const std::uint32_t index) const
	{
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, (index - 0x800000) & 0xFFFFFF);

		if (!variable_id)
		{
			return {};
		}

		const auto value = game::scr_VarGlob->childVariableValue[variable_id];
		game::VariableValue variable{};
		variable.u = value.u.u;
		variable.type = static_cast<game::scriptType_e>(value.type);

		return variable;
	}

	script_value array::get(const script_value& key) const
	{
		if (key.is<int>())
		{
			return this->get(key.as<int>());
		}

		if (key.is<std::string>())
		{
			return this->get(key.as<std::string>());
		}

		throw std::runtime_error(std::format("invalid key type '{}'", key.type_name()));
	}

	void array::set(const std::string& key, const script_value& value) const
	{
		const auto variable_id = this->get_value_id(key);

		if (!variable_id)
		{
			return;
		}

		const auto& value_raw = value.get_raw();
		const auto variable = &game::scr_VarGlob->childVariableValue[variable_id];
		game::VariableValue variable_value{};
		variable_value.type = variable->type;
		variable_value.u = variable->u.u;

		game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value_raw);
		game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, variable_value.type, variable_value.u);

		variable->type = gsl::narrow_cast<char>(value_raw.type);
		variable->u.u = value_raw.u;
	}

	void array::set(const std::uint32_t index, const script_value& value) const
	{
		const auto variable_id = this->get_value_id(index);

		if (!variable_id)
		{
			return;
		}

		const auto& value_raw = value.get_raw();
		const auto variable = &game::scr_VarGlob->childVariableValue[variable_id];
		game::VariableValue variable_value{};
		variable_value.type = variable->type;
		variable_value.u = variable->u.u;

		game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value_raw);
		game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, variable_value.type, variable_value.u);

		variable->type = gsl::narrow_cast<char>(value_raw.type);
		variable->u.u = value_raw.u;
	}

	void array::set(const script_value& key, const script_value& value) const
	{
		if (key.is<int>())
		{
			return this->set(key.as<int>(), value);
		}

		if (key.is<std::string>())
		{
			return this->set(key.as<std::string>(), value);
		}

		throw std::runtime_error(std::format("invalid key type '{}'", key.type_name()));
	}

	std::uint32_t array::get_entity_id() const
	{
		return this->id_;
	}

	std::uint32_t array::get_value_id(const std::string& key) const
	{
		const auto string_value = game::SL_GetString(key.data(), 0);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);

		if (!variable_id)
		{
			return game::GetNewVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);
		}

		return variable_id;
	}

	std::uint32_t array::get_value_id(const std::uint32_t index) const
	{
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, (index - 0x800000) & 0xFFFFFF);
		if (!variable_id)
		{
			return game::GetNewArrayVariable(game::SCRIPTINSTANCE_SERVER, this->id_, index);
		}

		return variable_id;
	}

	entity array::get_raw() const
	{
		return entity(this->id_);
	}

	array_value array::operator[](const script_value& key) const
	{
		return array_value(this, key);
	}

	array_iterator array::begin() const
	{
		const auto keys = this->get_keys();
		return array_iterator(this, keys, 0);
	}

	array_iterator array::end() const
	{
		return array_iterator(this);
	}

	array_iterator array::find(const script_value& key) const
	{
		const auto keys = this->get_keys();
		for (auto i = 0u; i < keys.size(); i++)
		{
			if (keys[i] == key)
			{
				return array_iterator(this, keys, i);
			}
		}

		return array_iterator(this);
	}
}

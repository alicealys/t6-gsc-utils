#include <stdinc.hpp>

#include "object.hpp"
#include "execution.hpp"

namespace scripting
{
	object_value::object_value(const object* object, const std::string& key)
		: object_(object)
		  , key_(key)
	{
		const auto value = this->object_->get(key);
		this->script_value::operator=(value);
	}

	void object_value::operator=(const script_value& value)
	{
		this->object_->set(this->key_, value);
		this->script_value::operator=(value);
	}

	object::object(const std::uint32_t id)
		: id_(id)
	{
		this->add();
	}

	object::object(const object& other)
	{
		this->operator=(other);
	}

	object::object(object&& other) noexcept
	{
		this->id_ = other.id_;
		other.id_ = 0;
	}

	object::object()
	{
		this->id_ = make_object();
	}

	object::~object()
	{
		this->release();
	}

	object& object::operator=(const object& other)
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			this->add();
		}

		return *this;
	}

	object& object::operator=(object&& other) noexcept
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			other.id_ = 0;
		}

		return *this;
	}

	void object::add() const
	{
		if (this->id_)
		{
			game::VariableValue value{};
			value.u.uintValue = this->id_;
			value.type = game::SCRIPT_OBJECT;

			game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value);
		}
	}

	void object::release() const
	{
		if (this->id_)
		{
			game::VariableValue value{};
			value.u.uintValue = this->id_;
			value.type = game::SCRIPT_OBJECT;

			game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, value.type, value.u);
		}
	}

	void object::iterate_keys(const std::function<bool(const std::string& key)>& callback) const
	{
		auto current = game::scr_VarGlob->objectVariableChildren[this->id_].firstChild;

		while (current)
		{
			const auto var = game::scr_VarGlob->childVariableValue[current];
			const auto string_id = static_cast<std::uint8_t>(var.name_lo) + (var.k.keys.name_hi << 8);

			if (string_id < 0x34BC)
			{
				static const auto address = SELECT(0x2DACC28, 0x2D7CF28);
				const auto string = reinterpret_cast<const char**>(address)[string_id];
				if (string != nullptr && callback(string))
				{
					return;
				}
			}

			current = var.nextSibling;
		}
	}

	std::vector<std::string> object::get_keys() const
	{
		std::vector<std::string> result;
		iterate_keys([&](const std::string& key)
		{
			result.emplace_back(key);
			return false;
		});

		return result;
	}

	std::optional<std::string> object::get_next_key(const std::string& current) const
	{
		auto is_next = false;
		std::optional<std::string> next;

		iterate_keys([&](const std::string& key)
		{
			if (is_next)
			{
				next.emplace(key);
				return true;
			}

			if (key == current)
			{
				is_next = true;
			}

			return false;
		});

		return next;
	}

	std::uint32_t object::size() const
	{
		return game::Scr_GetSelf(game::SCRIPTINSTANCE_SERVER, this->id_);
	}

	void object::erase(const std::string& key) const
	{
		const auto string_value = game::SL_GetCanonicalString(key.data(), 0);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);
		if (variable_id)
		{
			game::RemoveVariableValue(game::SCRIPTINSTANCE_SERVER, this->id_, variable_id);
		}
	}

	void object::erase(const object_iterator& iter) const
	{
		this->erase(iter->first);
	}

	script_value object::get(const std::string& key) const
	{
		const auto string_value = game::SL_GetCanonicalString(key.data(), 0);
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

	void object::set(const std::string& key, const script_value& value) const
	{
		const auto& value_raw = value.get_raw();
		const auto variable_id = this->get_value_id(key);

		if (!variable_id)
		{
			return;
		}

		const auto variable = &game::scr_VarGlob->childVariableValue[variable_id];
		game::VariableValue variable_{};
		variable_.type = variable->type;
		variable_.u = variable->u.u;

		game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value_raw);
		game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, variable_.type, variable_.u);

		variable->type = gsl::narrow_cast<char>(value_raw.type);
		variable->u.u = value_raw.u;
	}

	std::uint32_t object::get_entity_id() const
	{
		return this->id_;
	}

	std::uint32_t object::get_value_id(const std::string& key) const
	{
		const auto string_value = game::SL_GetCanonicalString(key.data(), 0);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);

		if (!variable_id)
		{
			return game::GetNewVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);
		}

		return variable_id;
	}

	entity object::get_raw() const
	{
		return entity(this->id_);
	}

	object_value object::operator[](const std::string& key) const
	{
		return object_value(this, key);
	}

	object_iterator object::begin() const
	{
		const auto keys = this->get_keys();
		return object_iterator(this, keys, 0);
	}

	object_iterator object::end() const
	{
		return object_iterator(this);
	}

	object_iterator object::find(const std::string& key) const
	{
		const auto keys = this->get_keys();
		for (auto i = 0u; i < keys.size(); i++)
		{
			if (keys[i] == key)
			{
				return object_iterator(this, keys, i);
			}
		}

		return object_iterator(this);
	}
}

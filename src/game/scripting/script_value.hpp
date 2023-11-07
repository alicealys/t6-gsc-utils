#pragma once
#include "game/game.hpp"
#include "variable_value.hpp"
#include "vector.hpp"

#include <utils/string.hpp>

namespace scripting
{
	class entity;
	class array;
	class object;
	class function;
	class script_value;

	namespace
	{
		std::unordered_map<int, std::string> typenames = 
		{
			{0, "undefined"},
			{1, "object"},
			{2, "string"},
			{3, "localized string"},
			{4, "vector"},
			{5, "hash"},
			{6, "float"},
			{7, "int"},
			{8, "codepos"},
			{9, "precodepos"},
			{10, "function"},
			{11, "stack"},
			{12, "animation"},
			{13, "developer codepos"},
			{14, "thread"},
			{15, "thread"},
			{16, "thread"},
			{17, "thread"},
			{18, "struct"},
			{19, "removed entity"},
			{20, "entity"},
			{21, "array"},
			{22, "removed thread"},
			{23, "<free>"},
			{24, "thread list"},
			{25, "ent list"},
		};

		std::string get_typename(const game::VariableValue& value)
		{
			if (value.type == game::SCRIPT_OBJECT)
			{
				const auto type = game::scr_VarGlob->objectVariableValue[value.u.uintValue].w.type & 0x7F;
				return typenames[type];
			}
			else
			{
				return typenames[value.type];
			}
		}

		template <typename T>
		std::string get_c_typename()
		{
			auto& info = typeid(T);

			if (info == typeid(std::string))
			{
				return "string";
			}

			if (info == typeid(const char*))
			{
				return "string";
			}

			if (info == typeid(entity))
			{
				return "entity";
			}

			if (info == typeid(array))
			{
				return "array";
			}

			if (info == typeid(object))
			{
				return "struct";
			}

			if (info == typeid(function))
			{
				return "function";
			}

			if (info == typeid(vector))
			{
				return "vector";
			}

			return info.name();
		}
	}

	using arguments = std::vector<script_value>;

	class script_value
	{
	public:
		script_value() = default;
		script_value(const game::VariableValue& value);

		script_value(void* value);

		script_value(int value);
		script_value(unsigned int value);
		script_value(bool value);

		script_value(float value);
		script_value(double value);

		script_value(const char* value);
		script_value(const std::string& value);

		script_value(const entity& value);
		script_value(const array& value);
		script_value(const object& value);

		script_value(const function& value);

		script_value(const vector& value);

	private:
		template <typename T>
		T get() const
		{
			if constexpr (std::is_pointer<T>::value)
			{
				if (this->is<unsigned int>())
				{
					return reinterpret_cast<T>(this->as<unsigned int>());
				}
			}

			if constexpr (std::is_constructible<T, std::string>::value)
			{
				if (this->is<std::string>())
				{
					return T(this->as<std::string>());
				}
			}

			throw std::runtime_error("Invalid type");
		}
	public:

#define ADD_TYPE(type) \
		template <> \
		bool is<type>() const; \
private: \
		template <> \
		type get<>() const; \
public: \

		template <typename T>
		bool is() const
		{
			if (std::is_pointer<T>::value && this->is<unsigned int>())
			{
				return true;
			}

			if (std::is_constructible<T, std::string>::value && this->is<std::string>())
			{
				return true;
			}

			return false;
		}

		ADD_TYPE(bool)
		ADD_TYPE(int)
		ADD_TYPE(unsigned int)
		ADD_TYPE(unsigned short)
		ADD_TYPE(float)
		ADD_TYPE(float*)
		ADD_TYPE(double)
		ADD_TYPE(const char*)
		ADD_TYPE(std::string)

		ADD_TYPE(vector)
		ADD_TYPE(array)
		ADD_TYPE(object)
		ADD_TYPE(function)
		ADD_TYPE(entity)
		ADD_TYPE(script_value)

		template <typename T>
		T as() const
		{
			if (!this->is<T>())
			{
				const auto type = get_typename(this->get_raw());
				const auto c_type = get_c_typename<T>();
				throw std::runtime_error(utils::string::va("has type '%s' but should be '%s'", type.data(), c_type.data()));
			}

			return get<T>();
		}

		std::string type_name() const
		{
			return get_typename(this->get_raw());
		}

		template <template<class, class> class C, class T, typename ArrayType = array>
		script_value(const C<T, std::allocator<T>>& container)
		{
			ArrayType array_{};

			for (const auto& value : container)
			{
				array_.push(value);
			}

			game::VariableValue value{};
			value.type = game::SCRIPT_OBJECT;
			value.u.pointerValue = array_.get_entity_id();

			this->value_ = value;
		}

		template<class ...T>
		arguments operator()(T... arguments) const
		{
			return this->as<function>().call({arguments...});
		}

		std::string to_string() const;

		const game::VariableValue& get_raw() const;

		variable_value value_{};

	};

	class function_argument;

	class variadic_args : public std::vector<function_argument>
	{
	public:
		variadic_args(const size_t begin);

		function_argument operator[](size_t index) const;
	private:
		size_t begin_;
	};

	class function_argument : public script_value
	{
	public:
		function_argument(const arguments& args, const script_value& value, const size_t index, const bool exists);

		template <typename T>
		T as() const
		{
			if (!this->exists_)
			{
				throw std::runtime_error(utils::string::va("parameter %d does not exist", this->index_));
			}

			try
			{
				return script_value::as<T>();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(utils::string::va("parameter %d %s",
					this->index_, e.what()));
			}
		}

		template <typename T, typename I = int>
		T* as_ptr() const
		{
			const auto value = script_value::as<I>();

			if (!value)
			{
				throw std::runtime_error("is null");
			}

			return reinterpret_cast<T*>(value);
		}

		template <>
		variadic_args as() const
		{
			variadic_args args{this->index_};
			for (auto i = this->index_; i < this->values_.size(); i++)
			{
				args.push_back({this->values_, this->values_[i], i, true});
			}
			return args;
		}

		operator variadic_args() const
		{
			return this->as<variadic_args>();
		}

		template <template<class, class> class C, class T, class ArrayType = array>
		operator C<T, std::allocator<T>>() const
		{
			const auto container_type = get_c_typename<C<T, std::allocator<T>>>();
			if (!script_value::as<ArrayType>())
			{
				const auto type = get_typename(this->get_raw());

				throw std::runtime_error(utils::string::va("has type '%s' but should be '%s'",
					type.data(),
					container_type.data()
				));
			}

			C<T, std::allocator<T>> container{};
			const auto array = script_value::as<ArrayType>();
			for (auto i = 0; i < array.size(); i++)
			{
				try
				{
					container.push_back(array.get(i).as<T>());
				}
				catch (const std::exception& e)
				{
					throw std::runtime_error(utils::string::va("element %d of parameter %d of type '%s' %s",
						i, this->index_, container_type.data(), e.what()));
				}
			}

			return container;
		}

		template <typename T>
		operator T() const
		{
			return this->as<T>();
		}

	private:
		arguments values_{};
		size_t index_{};
		bool exists_{};
	};

	class function_arguments
	{
	public:
		function_arguments(const arguments& values);

		function_argument operator[](const size_t index) const;

		arguments get_raw() const;

		size_t size() const;

	private:
		arguments values_{};
	};
}

#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "mysql.hpp"
#include "component/gsc.hpp"
#include "component/scheduler.hpp"
#include "component/scripting.hpp"

#include <utils/string.hpp>

namespace mysql
{
	namespace
	{
		struct task_t
		{
			std::thread thread;
			bool done;
			bool canceled;
			unsigned int handle_id;
			MYSQL_RES* result;
		};

		uint64_t task_index{};
		std::unordered_map<uint64_t, task_t> tasks;

		scripting::script_value field_to_value(const MYSQL_FIELD* field, const std::string& row)
		{
			switch (field->type)
			{
			case enum_field_types::MYSQL_TYPE_INT24:
			case enum_field_types::MYSQL_TYPE_LONG:
			case enum_field_types::MYSQL_TYPE_LONGLONG:
			case enum_field_types::MYSQL_TYPE_SHORT:
				return std::atoi(row.data());
			case enum_field_types::MYSQL_TYPE_FLOAT:
			case enum_field_types::MYSQL_TYPE_DOUBLE:
				return std::atof(row.data());
			}

			return row;
		}

		scripting::array generate_result(MYSQL_RES* result)
		{
			scripting::array result_arr;

			if (result == nullptr)
			{
				return {};
			}

			const auto num_rows = mysql_num_rows(result);
			const auto num_fields = mysql_num_fields(result);
			const auto fields = mysql_fetch_fields(result);

			for (auto i = 0u; i < num_rows; i++)
			{
				scripting::array row_arr;

				const auto row = mysql_fetch_row(result);
				const auto lengths = mysql_fetch_lengths(result);

				for (auto f = 0u; f < num_fields; f++)
				{
					const auto field = &fields[f];

					const std::string field_str = {field->name, field->name_length};
					const std::string row_str = {row[f], lengths[f]};
					const auto value = field_to_value(field, row_str);

					row_arr[field_str] = value;
				}

				result_arr.push(row_arr);
			}

			return result_arr;
		}

		template <typename F, typename ...Args>
		scripting::object create_mysql_query(F&& cb)
		{
			auto task = &tasks[task_index++];

			task->done = false;
			task->canceled = false;

			task->thread = std::thread([=]()
			{
				try
				{
					mysql::access([&](database_t& db)
					{
						task->result = cb(db);
						task->done = true;
					});
				}
				catch (const std::exception& e)
				{
					printf("%s\n", e.what());
					task->done = true;
				}
			});

			scripting::object handle;
			const auto handle_id = handle.get_entity_id();
			task->handle_id = handle_id;

			game::VariableValue value{};
			value.type = game::SCRIPT_OBJECT;
			value.u.uintValue = handle_id;
			game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value);

			return handle;
		}
	}

	std::array<connection_t, max_connections> connection_pool;

	utils::concurrency::container<sql::connection_config>& get_config()
	{
		static utils::concurrency::container<sql::connection_config> config;
		static auto initialized = false;

		if (!initialized)
		{
			config.access([&](sql::connection_config& cfg)
			{
				cfg.user = "root";
				cfg.password = "root";
				cfg.host = "localhost";
				cfg.port = 3306;
				cfg.database = "default";
			});

			initialized = true;
		}
		return config;
	}

	void cleanup_connections()
	{
		for (auto& connection : connection_pool)
		{
			std::unique_lock<database_mutex_t> lock(connection.mutex, std::try_to_lock);
			if (!lock.owns_lock())
			{
				continue;
			}

			const auto now = std::chrono::high_resolution_clock::now();
			const auto diff = now - connection.last_access;
			if (diff >= connection_timeout)
			{
				connection.db.reset();
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scripting::on_shutdown([]()
			{
				for (auto i = tasks.begin(); i != tasks.end(); ++i)
				{
					i->second.canceled = true;
				}
			});

			scheduler::loop([]
			{
				cleanup_connections();
			}, scheduler::async, 1s);

			scheduler::loop([]
			{
				for (auto i = tasks.begin(); i != tasks.end(); )
				{
					if (!i->second.done)
					{
						++i;
						continue;
					}

					if (i->second.thread.joinable())
					{
						i->second.thread.join();
					}

					if (!i->second.canceled)
					{
						const auto result = generate_result(i->second.result);
						scripting::notify(i->second.handle_id, "done", {result});

						game::RemoveRefToObject(game::SCRIPTINSTANCE_SERVER, i->second.handle_id);

						mysql_free_result(i->second.result);
						i->second.result = nullptr;
					}

					i = tasks.erase(i);
				}
			}, scheduler::server);

			gsc::function::add("mysql::set_config", [](const scripting::object& config)
			{
				get_config().access([&](sql::connection_config& cfg)
				{
					cfg.host = config["host"].as<std::string>();
					cfg.user = config["user"].as<std::string>();
					cfg.password = config["password"].as<std::string>();
					cfg.port = config["port"].as<unsigned short>();
					cfg.database = config["database"].as<std::string>();
				});
			});

			gsc::function::add("mysql::query", [](const std::string& query)
			{
				return create_mysql_query([=](database_t& db)
				{
					const auto handle = db->get_handle();
					mysql_query(handle, query.data());
					return mysql_store_result(handle);
				});
			});

			gsc::function::add("mysql::statement", [](const std::string& query, const scripting::variadic_args& values)
			{
				MYSQL_BIND* binds = nullptr;
				size_t bind_count = 0;

				const auto free_binds = [=]
				{
					if (binds == nullptr)
					{
						return;
					}

					for (auto i = 0u; i < bind_count; i++)
					{
						utils::memory::free(binds[i].buffer);
					}

					utils::memory::free(binds);
				};

				try
				{
					const auto bind_args = [&]<typename T>(const T& args)
					{
						bind_count = args.size();
						binds = utils::memory::allocate_array<MYSQL_BIND>(bind_count);

						for (auto i = 0u; i < args.size(); i++)
						{
							const auto& arg = args[i];
							const auto& raw_value = arg.get_raw();

							switch (raw_value.type)
							{
							case game::SCRIPT_FLOAT:
							{
								binds[i].buffer = utils::memory::allocate<float>();
								binds[i].buffer_type = MYSQL_TYPE_FLOAT;
								*reinterpret_cast<float*>(binds[i].buffer) = raw_value.u.floatValue;
								break;
							}
							case game::SCRIPT_INTEGER:
							{
								binds[i].buffer = utils::memory::allocate<int>();
								binds[i].buffer_type = MYSQL_TYPE_LONG;
								*reinterpret_cast<int*>(binds[i].buffer) = raw_value.u.intValue;
								break;
							}
							case game::SCRIPT_STRING:
							{
								const auto str = arg.as<std::string>();
								const auto str_copy = utils::memory::duplicate_string(str);
								binds[i].buffer = str_copy;
								binds[i].buffer_length = str.size();
								binds[i].buffer_type = MYSQL_TYPE_STRING;
								break;
							}
							default:
							{
								binds[i].buffer_type = MYSQL_TYPE_NULL;
								break;
							}
							}
						}
					};

					if (values.size() > 0 && values[0].is<scripting::array>())
					{
						bind_args(values[0].as<scripting::array>());
					}
					else
					{
						bind_args(values);
					}
				}
				catch (const std::exception& e)
				{
					free_binds();
					throw e;
				}

				return create_mysql_query([=](database_t& db)
					-> MYSQL_RES*
				{
					const auto _0 = gsl::finally([&]
					{
						free_binds();
					});

					const auto handle = db->get_handle();
					const auto stmt = mysql_stmt_init(handle);
					mysql_stmt_prepare(stmt, query.data(), query.size());
					mysql_stmt_bind_param(stmt, binds);
					mysql_stmt_execute(stmt);
					return mysql_store_result(handle);
				});
			});
		}
	};
}

REGISTER_COMPONENT(mysql::component)

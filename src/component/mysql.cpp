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
		struct mysql_result_t
		{
			MYSQL_RES* result;
			MYSQL_STMT* stmt;
			uint64_t affected_rows;
			std::string error;
		};

		struct task_t
		{
			scripting::object handle;
			mysql_result_t result{};
			std::atomic_bool completed;
		};

		std::vector<std::shared_ptr<task_t>> tasks;
		std::array<connection, max_connections> connection_pool;

		scripting::script_value field_to_value(const MYSQL_FIELD* field, const std::string& row)
		{
			switch (field->type)
			{
			case enum_field_types::MYSQL_TYPE_INT24:
			case enum_field_types::MYSQL_TYPE_LONG:
			case enum_field_types::MYSQL_TYPE_SHORT:
				return std::atoi(row.data());
			case enum_field_types::MYSQL_TYPE_LONGLONG:
				return row;
			case enum_field_types::MYSQL_TYPE_FLOAT:
			case enum_field_types::MYSQL_TYPE_DOUBLE:
				return static_cast<float>(std::atof(row.data()));
			}

			return row;
		}

		scripting::script_value bind_to_value(const MYSQL_BIND* bind)
		{
			switch (bind->buffer_type)
			{
			case enum_field_types::MYSQL_TYPE_INT24:
			case enum_field_types::MYSQL_TYPE_LONG:
			case enum_field_types::MYSQL_TYPE_SHORT:
				return *reinterpret_cast<int*>(bind->buffer);
			case enum_field_types::MYSQL_TYPE_LONGLONG:
				return std::to_string(*reinterpret_cast<std::int64_t*>(bind->buffer));
			case enum_field_types::MYSQL_TYPE_FLOAT:
				return *reinterpret_cast<float*>(bind->buffer);
			case enum_field_types::MYSQL_TYPE_DOUBLE:
				return static_cast<float>(*reinterpret_cast<double*>(bind->buffer));
			case enum_field_types::MYSQL_TYPE_STRING:
				return std::string{reinterpret_cast<char*>(bind->buffer), bind->buffer_length};
			}

			return {};
		}

		scripting::array generate_result(MYSQL_STMT* stmt)
		{
			if (stmt == nullptr)
			{
				return {};
			}

			utils::memory::allocator allocator;

			scripting::array result_arr;

			const auto meta = mysql_stmt_result_metadata(stmt);
			if (meta == nullptr)
			{
				return {};
			}

			const auto column_count = mysql_num_fields(meta);
			const auto fields = mysql_fetch_fields(meta);

			const auto is_null = allocator.allocate_array<my_bool>(column_count);
			const auto errors = allocator.allocate_array<my_bool>(column_count);
			const auto real_lengths = allocator.allocate_array<unsigned long>(column_count);
			const auto binds = allocator.allocate_array<MYSQL_BIND>(column_count);

			for (auto i = 0u; i < column_count; i++)
			{
				binds[i].length = &real_lengths[i];
				binds[i].is_null = &is_null[i];
				binds[i].error = &errors[i];
			}

			if (mysql_stmt_bind_result(stmt, binds) != 0 ||
				mysql_stmt_store_result(stmt) != 0)
			{
				return {};
			}

			while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA)
			{
				for (auto i = 0u; i < column_count; i++)
				{
					switch (fields[i].type)
					{
					case enum_field_types::MYSQL_TYPE_INT24:
					case enum_field_types::MYSQL_TYPE_LONG:
					case enum_field_types::MYSQL_TYPE_SHORT:
					{
						binds[i].buffer_type = MYSQL_TYPE_LONG;
						binds[i].buffer = allocator.allocate<int>();
						binds[i].buffer_length = sizeof(int);
						break;
					};
					case enum_field_types::MYSQL_TYPE_LONGLONG:
					{
						binds[i].buffer_type = MYSQL_TYPE_LONGLONG;
						binds[i].buffer = allocator.allocate<std::int64_t>();
						binds[i].buffer_length = sizeof(std::int64_t);
						break;
					};
					case enum_field_types::MYSQL_TYPE_FLOAT:
						binds[i].buffer_type = MYSQL_TYPE_FLOAT;
						binds[i].buffer = allocator.allocate<float>();
						binds[i].buffer_length = sizeof(float);
						break;
					case enum_field_types::MYSQL_TYPE_DOUBLE:
					{
						binds[i].buffer_type = MYSQL_TYPE_DOUBLE;
						binds[i].buffer = allocator.allocate<double>();
						binds[i].buffer_length = sizeof(double);
						break;
					}
					default:
						binds[i].buffer_type = MYSQL_TYPE_STRING;
						binds[i].buffer = allocator.allocate_array<char>(real_lengths[i]);
						binds[i].buffer_length = real_lengths[i];
						break;
					}

					mysql_stmt_fetch_column(stmt, &binds[i], i, 0);
				}

				scripting::array row_arr;

				for (auto i = 0u; i < column_count; i++)
				{
					const auto field = &fields[i];
					const std::string field_name = {field->name, field->name_length};
					row_arr[field_name] = bind_to_value(&binds[i]);
				}

				result_arr.emplace_back(row_arr);
			}

			mysql_free_result(meta);

			return result_arr;
		}

		scripting::array generate_result(MYSQL_RES* result)
		{
			if (result == nullptr)
			{
				return {};
			}

			scripting::array result_arr;

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

				result_arr.emplace_back(row_arr);
			}

			return result_arr;
		}

		template <typename F>
		scripting::object create_mysql_query(F&& cb)
		{
			auto task = std::make_shared<task_t>();
			tasks.emplace_back(task);

			scheduler::thread_pool.push([=]
			{
				try
				{
					mysql::access([&](database_t& db)
					{
						task->result = cb(db);
						task->completed = true;
					});
				}
				catch (const std::exception& e)
				{
					printf("^1MySQL ERROR: %s\n", e.what());
					task->completed = true;
				}
			});

			return task->handle;
		}

		void wait_and_clear_tasks()
		{
			for (auto& task : tasks)
			{
				while (!task->completed)
				{
					std::this_thread::sleep_for(10ms);
				}
			}

			tasks.clear();
		}

		void notify_task_result(std::shared_ptr<task_t>& task)
		{
			const auto result = task->result.result
				? generate_result(task->result.result)
				: generate_result(task->result.stmt);

			const auto rows = static_cast<std::size_t>(task->result.affected_rows);
			scripting::notify(task->handle.get_entity_id(), "done", {result, rows, task->result.error});

			if (task->result.result != nullptr)
			{
				mysql_free_result(task->result.result);
				task->result.result = nullptr;
			}

			if (task->result.stmt != nullptr)
			{
				mysql_stmt_close(task->result.stmt);
				task->result.stmt = nullptr;
			}
		}

		void check_tasks()
		{
			for (auto i = tasks.begin(); i != tasks.end(); )
			{
				auto& task = *i;
				if (!task->completed)
				{
					++i;
					continue;
				}
				else
				{
					notify_task_result(task);
					i = tasks.erase(i);
				}
			}
		}

		template <typename T>
		MYSQL_BIND* bind_statement_args(const T& args, std::size_t& bind_count)
		{
			bind_count = args.size();
			const auto binds = utils::memory::allocate_array<MYSQL_BIND>(bind_count);

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

			return binds;
		}

		void cleanup_connections()
		{
			for (auto& connection : connection_pool)
			{
				connection.cleanup();
			}
		}
	}

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

	void connection::check()
	{
		const auto now = std::chrono::high_resolution_clock::now();
		const auto diff = now - this->start_;

		if (this->db.get() == nullptr || !this->db->ping_server() || diff >= connection_timeout)
		{
			get_config().access([&](sql::connection_config& cfg)
			{
				this->db = std::make_unique<sql::connection>(cfg);
				this->start_ = now;
			});
		}

		this->last_access_ = now;
	}

	void connection::cleanup()
	{
		std::unique_lock<database_mutex_t> lock(this->mutex, std::try_to_lock);
		if (!lock.owns_lock())
		{
			return;
		}

		const auto now = std::chrono::high_resolution_clock::now();
		const auto diff = now - this->last_access_;
		if (diff >= connection_timeout)
		{
			this->db.reset();
		}
	}

	connection* get_connection(std::unique_lock<database_mutex_t>& lock)
	{
		static thread_local connection* last_connection{};
		if (last_connection != nullptr)
		{
			lock = std::unique_lock(last_connection->mutex, std::try_to_lock);
			if (lock.owns_lock())
			{
				return last_connection;
			}
		}

		for (auto i = 0u; i < connection_pool.size(); i++)
		{
			auto connection = &connection_pool[i];
			if (connection == last_connection)
			{
				continue;
			}

			lock = std::unique_lock(connection->mutex, std::try_to_lock);
			if (!lock.owns_lock())
			{
				continue;
			}

			last_connection = connection;
			return connection;
		}

		return nullptr;
	}

	class component final : public component_interface
	{
	public:
		void on_shutdown([[maybe_unused]] plugin::plugin* plugin) override
		{
			wait_and_clear_tasks();
		}

		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			scripting::on_shutdown(wait_and_clear_tasks);
			scheduler::loop(cleanup_connections, scheduler::async, 60s);
			scheduler::loop(check_tasks, scheduler::server);

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

			gsc::function::add_multiple([](const std::string& query)
			{
				return create_mysql_query([=](database_t& db)
				{
					const auto handle = db->get_handle();

					mysql_result_t result{};
					if (mysql_query(handle, query.data()) != 0)
					{
						result.error = mysql_error(handle);
						return result;
					}

					result.result = mysql_store_result(handle);
					result.affected_rows = mysql_affected_rows(handle);

					return result;
				});
			}, "mysql::query", "mysql::execute");

			gsc::function::add("mysql::prepared_statement", [](const std::string& query, const scripting::variadic_args& values)
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
					if (values.size() > 0 && values[0].is<scripting::array>())
					{
						binds = bind_statement_args(values[0].as<scripting::array>(), bind_count);
					}
					else
					{
						binds = bind_statement_args(values, bind_count);
					}
				}
				catch (const std::exception& e)
				{
					free_binds();
					throw e;
				}

				return create_mysql_query([=](database_t& db)
				{
					const auto _0 = gsl::finally(free_binds);

					mysql_result_t result{};

					const auto handle = db->get_handle();
					const auto stmt = mysql_stmt_init(handle);

					if (mysql_stmt_prepare(stmt, query.data(), query.size()) != 0 ||
						(binds != nullptr && mysql_stmt_bind_param(stmt, binds) != 0) ||
						mysql_stmt_execute(stmt) != 0)
					{
						result.error = mysql_stmt_error(stmt);
						return result;
					}

					result.stmt = stmt;
					result.affected_rows = mysql_stmt_affected_rows(stmt);

					return result;
				});
			});
		}
	};
}

REGISTER_COMPONENT(mysql::component)

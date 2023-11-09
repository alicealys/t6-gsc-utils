#pragma once

#include <utils/concurrency.hpp>

#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4267)
#pragma warning(disable: 4018)
#pragma warning(disable: 4996)
#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/mysql/mysql.h>
#pragma warning(pop)

namespace sql = sqlpp::mysql;

namespace mysql
{
	constexpr auto max_connections = 256;
	constexpr auto connection_timeout = 200s;

	using database_mutex_t = std::recursive_mutex;
	using database_t = std::unique_ptr<sql::connection>;

	struct connection_t
	{
		database_t db;
		database_mutex_t mutex;
		std::chrono::high_resolution_clock::time_point start;
		std::chrono::high_resolution_clock::time_point last_access;
	};

	extern std::array<connection_t, max_connections> connection_pool;

	utils::concurrency::container<sql::connection_config>& get_config();

	template <typename T = void, typename F>
	T access(F&& accessor)
	{
		for (auto& connection : connection_pool)
		{
			std::unique_lock<database_mutex_t> lock(connection.mutex, std::try_to_lock);
			if (!lock.owns_lock())
			{
				continue;
			}

			const auto now = std::chrono::high_resolution_clock::now();
			const auto diff = now - connection.start;

			if (!connection.db.get() || !connection.db->ping_server() || diff >= 1h)
			{
				get_config().access([&](sql::connection_config& cfg)
				{
					connection.db = std::make_unique<sql::connection>(cfg);
					connection.start = now;
				});
			}

			connection.last_access = now;
			return accessor(connection.db);
		}

		throw std::runtime_error("out of connections");
	}

	void cleanup_connections();

	void run_tasks();
}

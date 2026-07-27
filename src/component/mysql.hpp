#pragma once

#include <utils/concurrency.hpp>

#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4267)
#pragma warning(disable: 4018)
#pragma warning(disable: 4996)
#pragma warning(disable: 4244)
#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/mysql/mysql.h>
#pragma warning(pop)

namespace sql = sqlpp::mysql;

namespace mysql
{
	constexpr auto max_connections = 100;
	constexpr auto connection_timeout = 200s;

	using database_mutex_t = std::recursive_mutex;
	using database_t = std::unique_ptr<sql::connection>;

	utils::concurrency::container<sql::connection_config>& get_config();

	class connection
	{
	public:
		connection() = default;
		void check();
		void cleanup();

		database_t db;
		database_mutex_t mutex;

	private:
		std::chrono::high_resolution_clock::time_point start_;
		std::chrono::high_resolution_clock::time_point last_access_;

	};

	connection* get_connection(std::unique_lock<database_mutex_t>& lock);

	template <typename T = void, typename F>
	T access(F&& accessor)
	{
		std::unique_lock<database_mutex_t> lock;
		auto conn = get_connection(lock);
		if (conn == nullptr)
		{
			throw std::runtime_error("out of connections");
		}

		conn->check();
		return accessor(conn->db);
	}
}

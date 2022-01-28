#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "gsc.hpp"

#include <utils/hook.hpp>

typedef std::pair<std::string, std::string> bot_entry;

namespace bots
{
	namespace
	{
		std::vector<bot_entry> bot_names{};
		std::mutex bot_names_mutex;
		utils::hook::detour sv_bot_name_random_hook;

		void add_bot_data(const std::string name, const std::string clantag)
		{
			std::unique_lock<std::mutex> _(bot_names_mutex);

			if (!bot_names.empty())
			{
				for (const auto& entry : bot_names)
				{
					if (entry.first == name)
					{
						// No duplicates
						return;
					}
				}
			}

			bot_names.emplace_back(std::make_pair(name, clantag));
		}

		void clear_bot_data()
		{
			std::unique_lock<std::mutex> _(bot_names_mutex);
			bot_names.clear();
		}

		// If the list contains at least 18 names there should not be any collisions
		const char* sv_bot_name_random_stub()
		{
			std::unique_lock<std::mutex> _(bot_names_mutex);

			static int botId = 0;
			if (!bot_names.empty())
			{
				botId %= bot_names.size();
				const auto& entry = bot_names.at(botId++);
				return entry.first.data();
			}

			return sv_bot_name_random_hook.invoke<const char*>();
		}

		int build_connect_string(char* buf, const char* connectString, const char* name, const char* xuid,
			const char* xnaddr, int protocol, int netfield, int sessionMode, int port)
		{
			std::unique_lock<std::mutex> _(bot_names_mutex);

			auto clantag = "3arch"s; // Default
			if (!bot_names.empty())
			{
				for (const auto& entry : bot_names)
				{
					if (entry.first == name)
					{
						clantag = entry.second;
						break;
					}
				}
			}

			return _snprintf_s(buf, 0x400, _TRUNCATE, connectString, name,
				clantag.data(), xuid, xnaddr, protocol, netfield, sessionMode, port);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// Add custom clantag
			utils::hook::set<const char*>(SELECT(0x5515FC, 0x428BDB), "connect \"\\invited\\1\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\"
				" model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\clanAbbrev\\%s\\xuid\\%s\\xnaddr\\%s\\"
				" natType\\2\\protocol\\%d\\netfieldchk\\%d\\sessionmode\\%d\\qport\\%d\"");

			sv_bot_name_random_hook.create(SELECT(0x6A2C70, 0x62D750), &sv_bot_name_random_stub);
			utils::hook::call(SELECT(0x551601, 0x428BE0), build_connect_string);

			command::add("addbotdata", [](command::params& params)
			{
				if (params.size() < 3)
				{
					printf("Usage: %s <name> <clantag>\n", params.get(0));
					return;
				}

				add_bot_data(params.get(1), params.get(2));
			});

			command::add("clearbotdata", [](command::params&)
			{
				clear_bot_data();
			});

			gsc::function::add("addbotdata", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto name = args[0].as<std::string>();
				const auto clantag = args[1].as<std::string>();
				add_bot_data(name, clantag);

				return {};
			});

			gsc::function::add("clearbotdata", [](const gsc::function_args& args) -> scripting::script_value
			{
				clear_bot_data();

				return {};
			});
		}
	};
}

REGISTER_COMPONENT(bots::component)

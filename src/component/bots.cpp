#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

#include <json.hpp>
#include <utils/io.hpp>
#include <utils/hook.hpp>

namespace bots
{
	namespace
	{
		typedef std::pair<std::string, std::string> bot_entry;

		std::vector<bot_entry> bot_names;
		utils::hook::detour sv_bot_name_random_hook;

		// Json file is expected to contain one "names" object and that should contain a string (key)
		// for the bot's name and one string (value) for the clantag
		void load_bot_data()
		{
			if (!utils::io::file_exists("bots/bots.json"))
			{
				printf("bots.json was not found\n");
				return;
			}

			nlohmann::json obj;
			try
			{
				obj = nlohmann::json::parse(utils::io::read_file("bots/bots.json").data());
			}
			catch (nlohmann::json::parse_error& ex)
			{
				printf("json parse error\n");
				return;
			}

			for (const auto& [key, val] : obj["names"].items())
			{
				bot_names.emplace_back(std::make_pair(key, val.get<std::string>()));
			}
		}

		// If the list contains at least 18 names there should not be any collisions
		const char* sv_bot_name_random_stub()
		{
			if (bot_names.empty())
			{
				load_bot_data();
			}

			static int bot_id = 0;
			if (!bot_names.empty())
			{
				bot_id %= bot_names.size();
				const auto& entry = bot_names.at(bot_id++);
				return entry.first.data();
			}

			return sv_bot_name_random_hook.invoke<const char*>();
		}

		int build_connect_string(char* buf, const char* connectString, const char* name, const char* xuid,
			const char* xnaddr, int protocol, int netfield, int sessionMode, int port)
		{
			// Default
			auto clantag = "3arc"s;
			for (const auto& entry : bot_names)
			{
				if (entry.first == name)
				{
					clantag = entry.second;
					break;
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
		}
	};
}

REGISTER_COMPONENT(bots::component)

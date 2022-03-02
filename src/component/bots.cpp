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

		//std::vector<bot_entry> bot_names;

		std::vector<std::string> names;
		std::vector<bot_entry> clantags;


		utils::hook::detour sv_bot_name_random_hook;

		// Json file is expected to contain one "names" object and that should contain a string (key)
		// for the bot's name and one string (value) for the clantag
		// exemple: 
		//{ 
		//		"names": ["name1", "name2", "name3"],
		//		"clantags" : ["c1", "c2", "c3"]
		//}

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
			catch (nlohmann::json::parse_error&)
			{
				printf("json parse error\n");
				return;
			}

			std::vector<std::string> ns = obj["names"];

			for (int i = 0; i < ns.size(); i++)
			{
				names.push_back(ns.at(i).c_str());
			}

			std::vector<std::string> cs = obj["clantags"];
			for (int i = 0; i < cs.size(); i++)
			{
				clantags.emplace_back(ns.at(i).c_str(), cs.at(i).c_str());
			}
		}

		// If the list contains at least 18 names there should not be any collisions
		const char* sv_bot_name_random_stub()
		{
			if (names.empty())
			{
				load_bot_data();
			}
			static auto bot_id = 0;
			if (!names.empty())
			{
				bot_id %= names.size();
				//const auto& entry = names.at(bot_id++).c_str();
				return names.at(bot_id++).c_str();
			}


			return sv_bot_name_random_hook.invoke<const char*>();
		}

		int build_connect_string(char* buf, const char* connect_string, const char* name, const char* xuid,
			const char* xnaddr, int protocol, int netfield, int session_mode, int port)
		{
			// Default
			auto clantag = "3arc"s;
			for (const auto& entry : clantags)
			{
				if (entry.first == name)
				{
					clantag = entry.second;
					break;
				}
			}

			return _snprintf_s(buf, 0x400, _TRUNCATE, connect_string, name,
				clantag.data(), xuid, xnaddr, protocol, netfield, session_mode, port);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			add_gsc_functions();

			// Add custom clantag
			utils::hook::set<const char*>(SELECT(0x5515FC, 0x428BDB), "connect \"\\invited\\1\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\"
				" model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\clanAbbrev\\%s\\xuid\\%s\\xnaddr\\%s\\"
				" natType\\2\\protocol\\%d\\netfieldchk\\%d\\sessionmode\\%d\\qport\\%d\"");

			sv_bot_name_random_hook.create(SELECT(0x6A2C70, 0x62D750), &sv_bot_name_random_stub);
			utils::hook::call(SELECT(0x551601, 0x428BE0), build_connect_string);
		}

	private:
		static void add_gsc_functions()
		{
			gsc::function::add("dropallbots", [](const gsc::function_args&) -> scripting::script_value
			{
				const auto* dvar = game::Dvar_FindVar("com_maxclients");
				const auto client_count = game::Dvar_GetInt(dvar);

				for (auto i = 0; i < client_count; i++)
				{
					if (game::SV_IsTestClient(i))
					{
						game::SV_GameDropClient(i, "GAME_DROPPEDFORINACTIVITY");
					}
				}

				return {};
			});
		}
	};
}

REGISTER_COMPONENT(bots::component)

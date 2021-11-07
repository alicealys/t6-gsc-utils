#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "gsc.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace chat
{
	namespace
	{
		int post_get_name;
		int post_get_clantag;

		int client_index;

		std::string names[18];
		std::string clantags[18];

		const char* get_clantag(int clientNum, const char* s, const char* key)
		{
			if (clantags[clientNum].empty())
			{
				return reinterpret_cast<const char* (*)(const char*, 
					const char*)>(SELECT(0x68B420, 0x46AD00))(s, key);
			}

			return clantags[clientNum].data();
		}

		const char* get_name(int clientNum, const char* s, const char* key)
		{
			if (names[clientNum].empty())
			{
				return reinterpret_cast<const char* (*)(const char*, 
					const char*)>(SELECT(0x68B420, 0x46AD00))(s, key);
			}

			return names[clientNum].data();
		}

		void sv_get_user_info_stub(int index, char* buffer, int bufferSize)
		{
			client_index = index;

			reinterpret_cast<void (*)(int, char*, 
				int)>(SELECT(0x68BB90, 0x4C10F0))(index, buffer, bufferSize);
		}

		const char* info_value_for_name(const char* s, const char* key)
		{
			const auto name = get_name(client_index, s, key);
			
			return name;
		}

		const char* info_value_for_clantag(const char* s, const char* key)
		{
			const auto name = get_clantag(client_index, s, key);

			return name;
		}

		void client_disconnect_stub(int clientNum)
		{
			clantags[clientNum].clear();
			names[clientNum].clear();

			reinterpret_cast<void (*)(int)>(SELECT(0x42FB00, 0x64ADE0))(clientNum);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			post_get_name = SELECT(0x4ED799, 0x427EB9);
			post_get_clantag = SELECT(0x4ED7C2, 0x427EE2);

			utils::hook::call(SELECT(0x4ED794, 0x427EB4), info_value_for_name);
			utils::hook::call(SELECT(0x4ED7BD, 0x427EDD), info_value_for_clantag);

			utils::hook::call(SELECT(0x4ED6D0, 0x427DF0), sv_get_user_info_stub);

			utils::hook::call(SELECT(0x4D8888, 0x5304C8), client_disconnect_stub);

			gsc::method::add("resetname", [](game::scr_entref_t ent, const gsc::function_args&) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					return {};
				}

				names[ent.entnum].clear();
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("resetclantag", [](game::scr_entref_t ent, const gsc::function_args&) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					return {};
				}

				clantags[ent.entnum].clear();
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("rename", [](game::scr_entref_t ent, const gsc::function_args&) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					return {};
				}

				const auto name = game::get<std::string>(0);
				names[ent.entnum] = name;
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("setclantag", [](game::scr_entref_t ent, const gsc::function_args& args) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					return {};
				}

				const auto clantag = args[0].as<std::string>();
				clantags[ent.entnum] = clantag;
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("tell", [](game::scr_entref_t ent, const gsc::function_args& args) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					return {};
				}

				const auto client = ent.entnum;
				const auto msg = args[0].as<std::string>();
				game::SV_GameSendServerCommand(client, 0, utils::string::va("j \"%s\"", msg.data()));

				return {};
			});

			gsc::function::add("say", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto msg = args[0].as<std::string>();
				game::SV_GameSendServerCommand(-1, 0, utils::string::va("j \"%s\"", msg.data()));
				return {};
			});

			gsc::function::add("cmdexecute", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto cmd = args[0].as<std::string>();
				game::Cbuf_InsertText(0, cmd.data());
				return {};
			});

			gsc::function::add("sendservercommand", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto client = args[0].as<int>();
				const auto cmd = args[1].as<std::string>();
				game::SV_GameSendServerCommand(client, 0, cmd.data());
				return {};
			});
		}
	};
}

REGISTER_COMPONENT(chat::component)

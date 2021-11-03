#include <stdafx.hpp>

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

	void init()
	{
		post_get_name = SELECT(0x4ED799, 0x427EB9);
		post_get_clantag = SELECT(0x4ED7C2, 0x427EE2);

		utils::hook::call(SELECT(0x4ED794, 0x427EB4), info_value_for_name);
		utils::hook::call(SELECT(0x4ED7BD, 0x427EDD), info_value_for_clantag);

		utils::hook::call(SELECT(0x4ED6D0, 0x427DF0), sv_get_user_info_stub);

		utils::hook::call(SELECT(0x4D8888, 0x5304C8), client_disconnect_stub);

		method::add("resetname", 0, 0, [](game::scr_entref_t ent)
		{
			if (ent.classnum != 0)
			{
				return;
			}

			names[ent.entnum].clear();
			game::ClientUserInfoChanged(ent.entnum);
		});

		method::add("resetclantag", 0, 0, [](game::scr_entref_t ent)
		{
			if (ent.classnum != 0)
			{
				return;
			}

			clantags[ent.entnum].clear();
			game::ClientUserInfoChanged(ent.entnum);
		});

		method::add("rename", 1, 1, [](game::scr_entref_t ent)
		{
			if (ent.classnum != 0)
			{
				return;
			}

			const auto name = game::get<std::string>(0);
			names[ent.entnum] = name;

			game::ClientUserInfoChanged(ent.entnum);
		});

		method::add("setclantag", 1, 1, [](game::scr_entref_t ent)
		{
			if (ent.classnum != 0)
			{
				return;
			}

			const auto clantag = game::get<std::string>(0);
			clantags[ent.entnum] = clantag;

			game::ClientUserInfoChanged(ent.entnum);
		});
		
		method::add("tell", 1, 1, [](game::scr_entref_t ent)
		{
			if (ent.classnum != 0)
			{
				return;
			}

			const auto clientNum = ent.entnum;
			const auto msg = game::get<const char*>(0);

			game::SV_GameSendServerCommand(clientNum, 0, utils::string::va("j \"%s\"", msg));
		});

		function::add("say", 1, 1, []()
		{
			const auto msg = game::get<const char*>(0);

			game::SV_GameSendServerCommand(-1, 0, utils::string::va("j \"%s\"", msg));
		});

		function::add("cmdexecute", 1, 1, []()
		{
			const auto cmd = game::get<const char*>(0);

			game::Cbuf_InsertText(0, cmd);
		});

		function::add("sendservercommand", 2, 2, []()
		{
			const auto clientNum = game::get<int>(0);
			const auto cmd = game::get<const char*>(1);

			game::SV_GameSendServerCommand(clientNum, 0, cmd);
		});
	}
}

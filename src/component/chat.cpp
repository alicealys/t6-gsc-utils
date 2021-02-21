#include <stdafx.hpp>

namespace chat
{
	namespace
	{
		auto hidden = true;

		int g_say_to;
		int pre_say;

		int post_get_name;
		int post_get_clantag;

		std::string names[18];
		std::string clantags[18];

		char* evaluate_say(char* text, game::gentity_s* ent)
		{
			hidden = false;

			const auto name = game::SL_GetString("say", 0);

			game::add(text);
			game::add(ent);

			game::Scr_NotifyId(game::SCRIPTINSTANCE_SERVER, 0, *game::levelEntityId, name, 2);

			if (text[0] == '/')
			{
				hidden = true;
				++text;
			}

			return text;
		}

		char* clean_str(const char* str)
		{
			return game::I_CleanStr(str);
		}

		__declspec(naked) void pre_say_stub()
		{
			__asm
			{
				mov eax, [esp + 0xE4 + 0x10]

				push eax
				pushad

				push[esp + 0xE4 + 0x28]
				push eax
				call evaluate_say
				add esp, 0x8

				mov[esp + 0x20], eax
				popad
				pop eax

				mov[esp + 0xE4 + 0x10], eax

				call clean_str

				push pre_say
				retn
			}
		}

		__declspec(naked) void post_say_stub()
		{
			__asm
			{
				push eax

				xor eax, eax

				mov al, hidden

				cmp al, 0
				jne hide

				pop eax

				push g_say_to
				retn
			hide:
				pop eax

				retn
			}
		}

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

		__declspec(naked) void info_value_for_name()
		{
			__asm
			{
				push ebx;
				call get_name;
				pop ebx;

				push post_get_name;
				retn;
			}
		}

		__declspec(naked) void info_value_for_clantag()
		{
			__asm
			{
				push ebx;
				call get_clantag;
				pop ebx;

				push post_get_clantag;
				retn;
			}
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
		g_say_to = SELECT(0x82BB50, 0x82A3D0);
		pre_say = SELECT(0x6A7AB3, 0x493E63);

		post_get_name = SELECT(0x4ED799, 0x427EB9);
		post_get_clantag = SELECT(0x4ED7C2, 0x427EE2);

		utils::hook::jump(SELECT(0x6A7AAE, 0x493E5E), pre_say_stub);
		utils::hook::call(SELECT(0x6A7B5F, 0x493F0F), post_say_stub);
		utils::hook::call(SELECT(0x6A7B9B, 0x493F4B), post_say_stub);

		utils::hook::jump(SELECT(0x4ED794, 0x427EB4), info_value_for_name);
		utils::hook::jump(SELECT(0x4ED7BD, 0x427EDD), info_value_for_clantag);

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

			game::Cbuf_AddText(0, cmd);
		});

		function::add("sendservercommand", 2, 2, []()
		{
			const auto clientNum = game::get<int>(0);
			const auto cmd = game::get<const char*>(1);

			game::SV_GameSendServerCommand(clientNum, 0, cmd);
		});
	}
}

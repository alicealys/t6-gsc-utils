#include <stdafx.hpp>

namespace chat
{
	namespace
	{
		auto hidden = true;

		int g_say_to;
		int pre_say;

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
	}

	void init()
	{
		g_say_to = SELECT(0x82BB50, 0x82A3D0);
		pre_say = SELECT(0x6A7AB3, 0x493E63);

		utils::hook::jump(SELECT(0x6A7AAE, 0x493E5E), pre_say_stub);
		utils::hook::call(SELECT(0x6A7B5F, 0x493F0F), post_say_stub);
		utils::hook::call(SELECT(0x6A7B9B, 0x493F4B), post_say_stub);

		method::add("tell", 1, 1, [](game::scr_entref_t ent)
		{
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

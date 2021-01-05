#include <stdafx.hpp>

namespace chat
{
	namespace
	{
		auto hidden = true;

		int g_say_to;
		int pre_say;

		const char* evaluate_say(const char* text, game::gentity_s* ent)
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

		void pre_say_stub()
		{
			__asm
			{
				mov eax, edi
				push eax

				mov ecx, esi
				push ecx

				call evaluate_say

				mov edx, eax

				pop ecx
				pop eax

				add esp, 0x10
				mov eax, ebx
				dec eax
				mov eax, [edi]

				push edx

				push pre_say
				retn
			}
		}

		void post_say_stub()
		{
			__asm
			{
				mov al, hidden

				cmp al, 0
				jne hide

				push g_say_to
				retn
			hide:
				retn
			}
		}
	}

	void init()
	{
		g_say_to = SELECT(0x82BB50, 0x82A3D0);
		pre_say = SELECT(0x6A7AC3, 0x493E73);

		utils::hook::jump(SELECT(0x6A7ABA, 0x493E6A), pre_say_stub);
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

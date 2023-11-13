#include <stdinc.hpp>
#include "game.hpp"

#include <utils/hook.hpp>

namespace game
{
	gamemode current = reinterpret_cast<const char*>(0xC2F028) == "multiplayer"s
		? gamemode::multiplayer
		: gamemode::zombies;

	namespace environment
	{
		bool t6mp()
		{
			return current == gamemode::multiplayer;
		}

		bool t6zm()
		{
			return current == gamemode::zombies;
		}
	}

	gentity_s* GetEntity(scr_entref_t entref)
	{
		if (entref.classnum != 0)
		{
			game::Scr_ObjectError(SCRIPTINSTANCE_SERVER, "Not an entity");
			return nullptr;
		}

		assert(entref.entnum < (1 << 10));

		return &game::g_entities[entref.entnum];
	}

	scr_entref_t Scr_GetEntityIdRef(unsigned int ent_id)
	{
		scr_entref_t entref{};

		const auto v2 = &game::scr_VarGlob->objectVariableValue[ent_id];

		entref.entnum = v2->u.f.next;
		entref.classnum = gsl::narrow_cast<unsigned short>(v2->w.classnum >> 8);

		return entref;
	}

	void Scr_TerminateWaitThread(scriptInstance_t inst, unsigned int local_id, unsigned int start_local_id)
	{
		static const auto func = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.pushad();
			a.mov(esi, dword_ptr(esp, 0x24));
			a.push(dword_ptr(esp, 0x28));
			a.push(dword_ptr(esp, 0x2C));
			a.call(SELECT(0x8F4620, 0x8F3380));
			a.add(esp, 0x8);
			a.popad();

			a.ret();
		});

		utils::hook::invoke<void>(func, inst, local_id, start_local_id);
	}

	void Scr_TerminateWaittillThread(scriptInstance_t inst, unsigned int local_id, unsigned int start_local_id)
	{
		static const auto func = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.pushad();
			a.mov(esi, dword_ptr(esp, 0x24));
			a.mov(edi, dword_ptr(esp, 0x28));
			a.push(dword_ptr(esp, 0x2C));
			a.call(SELECT(0x8F4750, 0x8F34B0));
			a.add(esp, 0x4);
			a.popad();

			a.ret();
		});

		utils::hook::invoke<void>(func, inst, local_id, start_local_id);
	}
}

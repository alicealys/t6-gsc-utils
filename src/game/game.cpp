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

	void add(int value)
	{
		game::Scr_AddInt(SCRIPTINSTANCE_SERVER, value);
	}

	void add(float value)
	{
		game::Scr_AddFloat(SCRIPTINSTANCE_SERVER, value);
	}

	void add(float* value)
	{
		game::Scr_AddVector(SCRIPTINSTANCE_SERVER, value);
	}

	void add(const char* value)
	{
		game::Scr_AddString(SCRIPTINSTANCE_SERVER, value);
	}

	void add(gentity_s* value)
	{
		game::Scr_AddEntity(SCRIPTINSTANCE_SERVER, value);
	}

	void add(void* value)
	{
		game::Scr_AddInt(SCRIPTINSTANCE_SERVER, reinterpret_cast<uintptr_t>(value));
	}

	template <>
	int get(int index)
	{
		return game::Scr_GetInt(SCRIPTINSTANCE_SERVER, index);
	}

	template <>
	float get(int index)
	{
		return game::Scr_GetFloat(SCRIPTINSTANCE_SERVER, index);
	}

	template <>
	const char* get(int index)
	{
		return game::Scr_GetString(SCRIPTINSTANCE_SERVER, index);
	}

	template <>
	std::string get(int index)
	{
		return game::Scr_GetString(SCRIPTINSTANCE_SERVER, index);
	}

	/*gentity_s* GetEntity(scr_entref_t entref)
	{
		if (entref.classnum != 0)
		{
			game::Scr_ObjectError("Not an entity");
			return nullptr;
		}

		assert(entref.entnum < (1 << 10));

		return &game::g_entities[entref.entnum];
	}*/

	scr_entref_t Scr_GetEntityIdRef(unsigned int entId)
	{
		scr_entref_t entref;

		const auto v2 = &game::scr_VarGlob->objectVariableValue[entId];

		entref.entnum = v2->u.f.next;
		entref.classnum = v2->w.classnum >> 8;

		return entref;
	}

	void Scr_TerminateWaitThread(scriptInstance_t inst, unsigned int localId, unsigned int startLocalId)
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

		utils::hook::invoke<void>(func, inst, localId, startLocalId);
	}

	void Scr_TerminateWaittillThread(scriptInstance_t inst, unsigned int localId, unsigned int startLocalId)
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

		utils::hook::invoke<void>(func, inst, localId, startLocalId);
	}

	namespace plutonium
	{
		bool is_up_to_date()
		{
			const auto value = *reinterpret_cast<DWORD*>(0x21600000);
			return value == 0x9730166E;
		}
	}
}

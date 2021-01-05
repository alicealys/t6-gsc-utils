#include <stdafx.hpp>

namespace game
{
	gamemode current = gamemode::none;

	gentity_s* g_entities;

	unsigned int* levelEntityId;

	void* (__cdecl* Cbuf_AddText)(int, const char*);

	void* (__cdecl* Player_GetMethod)(const char**, int*, int*);

	void(__cdecl* Scr_AddEntity)(scriptInstance_t, gentity_s*);
	void(__cdecl* Scr_AddFloat)(scriptInstance_t, float);
	void(__cdecl* Scr_AddInt)(scriptInstance_t, int);
	void(__cdecl* Scr_AddString)(scriptInstance_t, const char*);
	void(__cdecl* Scr_AddVector)(scriptInstance_t, float*);
	void(__cdecl* Scr_AddObject)(scriptInstance_t, unsigned int);

	unsigned int(__cdecl* AllocObject)(scriptInstance_t);
	void(__cdecl* RemoveRefToObject)(scriptInstance_t, unsigned int);

	unsigned int(__cdecl* Scr_NotifyId)(scriptInstance_t, int, unsigned int, unsigned int, unsigned int);

	unsigned int(__cdecl* SL_GetString)(const char*, unsigned int);

	void* (__cdecl* Scr_GetCommonFunction)(const char**, int*, int*, int*);
	int(__cdecl* Scr_GetNumParam)(scriptInstance_t);
	gentity_s* (__cdecl* Scr_GetEntity)(scriptInstance_t, int);
	float(__cdecl* Scr_GetFloat)(scriptInstance_t, int);
	int(__cdecl* Scr_GetInt)(scriptInstance_t, int);
	const char* (__cdecl* Scr_GetString)(scriptInstance_t, int);
	void(__cdecl* Scr_GetVector)(scriptInstance_t, int, float*);

	void* (__cdecl* SV_GameSendServerCommand)(int clientNum, int type, const char* cmd);

	bool is_mp()
	{
		return current == gamemode::multiplayer;
	}

	bool is_zm()
	{
		return current == gamemode::zombies;
	}

	void init()
	{
		current = (strcmp(reinterpret_cast<const char*>(0xC2F028), "multiplayer") == 0) ? gamemode::multiplayer : gamemode::zombies;

		g_entities = reinterpret_cast<gentity_s*>(SELECT(0x21EF7C0, 0x21C13C0));

		levelEntityId = reinterpret_cast<unsigned int*>(SELECT(0x2E1A51C, 0x2DEA81C));

		Cbuf_AddText = (decltype(Cbuf_AddText))SELECT(0x5C6F10, 0x6B9D20);

		Player_GetMethod = (decltype(Player_GetMethod))SELECT(0x432480, 0x6F2DB0);

		Scr_AddEntity = (decltype(Scr_AddEntity))SELECT(0x4C20F0, 0x5D8F80);
		Scr_AddFloat = (decltype(Scr_AddFloat))SELECT(0x579130, 0x503480);
		Scr_AddInt = (decltype(Scr_AddInt))SELECT(0x57AFF0, 0x643A40);
		Scr_AddString = (decltype(Scr_AddString))SELECT(0x4F1650, 0x6A7A70);
		Scr_AddVector = (decltype(Scr_AddVector))SELECT(0x4C1A40, 0x4FAB00);
		Scr_AddObject = (decltype(Scr_AddObject))SELECT(0x539B50, 0x584DF0);

		AllocObject = (decltype(AllocObject))SELECT(0x6FB1B0, 0x6FE9D0);
		RemoveRefToObject = (decltype(RemoveRefToObject))SELECT(0x6FB1B0, 0x550DC0);

		Scr_NotifyId = (decltype(Scr_NotifyId))SELECT(0x4FDC10, 0x4BC0D0);

		SL_GetString = (decltype(SL_GetString))SELECT(0x602C40, 0x4601E0);

		Scr_GetCommonFunction = (decltype(Scr_GetCommonFunction))SELECT(0x691110, 0x4EB070);
		Scr_GetNumParam = (decltype(Scr_GetNumParam))SELECT(0x42E990, 0x680EA0);
		Scr_GetEntity = (decltype(Scr_GetEntity))SELECT(0x48F250, 0x489100);
		Scr_GetFloat = (decltype(Scr_GetFloat))SELECT(0x633400, 0x625EE0);
		Scr_GetInt = (decltype(Scr_GetInt))SELECT(0x45D840, 0x49A060);
		Scr_GetString = (decltype(Scr_GetString))SELECT(0x67C6A0, 0x488110);
		Scr_GetVector = (decltype(Scr_GetVector))SELECT(0x4CBCC0, 0x4BB100);

		SV_GameSendServerCommand = (decltype(SV_GameSendServerCommand))SELECT(0x45D7D0, 0x40D450);
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

	void add(std::string& value)
	{
		game::Scr_AddString(SCRIPTINSTANCE_SERVER, value.data());
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
}

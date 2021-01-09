#pragma once

#define SELECT(mp, zm) (game::is_mp() ? mp : zm)

namespace game
{
	enum gamemode
	{
		none,
		multiplayer,
		zombies
	};

	extern gamemode current;

	extern gentity_s* g_entities;

	extern unsigned int* levelEntityId;

	extern void* (__cdecl* Cbuf_AddText)(int, const char*);

	extern char* (__cdecl* I_CleanStr)(char*);

	extern void* (__cdecl* Player_GetMethod)(const char**, int*, int*);

	extern void(__cdecl* Scr_AddEntity)(scriptInstance_t inst, gentity_s* entity);
	extern void(__cdecl* Scr_AddFloat)(scriptInstance_t inst, float value);
	extern void(__cdecl* Scr_AddInt)(scriptInstance_t inst, int value);
	extern void(__cdecl* Scr_AddString)(scriptInstance_t inst, const char* value);
	extern void(__cdecl* Scr_AddVector)(scriptInstance_t inst, float* value);
	extern void(__cdecl* Scr_AddObject)(scriptInstance_t inst, unsigned int id);

	extern unsigned int(__cdecl* AllocObject)(scriptInstance_t inst);
	extern void(__cdecl* RemoveRefToObject)(scriptInstance_t inst, unsigned int id);

	extern unsigned int(__cdecl* Scr_NotifyId)(scriptInstance_t inst, int localClientNum, unsigned int id, unsigned int stringValue, unsigned int paramcount);

	extern unsigned int(__cdecl* SL_GetString)(const char* str, unsigned int user);

	extern int(__cdecl* Scr_GetNumParam)(scriptInstance_t inst);
	extern void* (__cdecl* Scr_GetCommonFunction)(const char**, int*, int*, int*);
	extern gentity_s* (__cdecl* Scr_GetEntity)(scriptInstance_t inst, int index);
	extern float(__cdecl* Scr_GetFloat)(scriptInstance_t inst, int index);
	extern int(__cdecl* Scr_GetInt)(scriptInstance_t inst, int index);
	extern const char* (__cdecl* Scr_GetString)(scriptInstance_t inst, int index);
	extern void(__cdecl* Scr_GetVector)(scriptInstance_t inst, int index, float* out);

	extern void* (__cdecl* SV_GameSendServerCommand)(int, int, const char*);

	void add(int);
	void add(float);
	void add(const char*);
	void add(gentity_s*);
	void add(void*);

	template <typename T>
	T get(int index);

	template <typename T>
	T get_ptr(int index)
	{
		return reinterpret_cast<T>(game::Scr_GetInt(SCRIPTINSTANCE_SERVER, index));
	}

	bool is_mp();
	bool is_zm();
	void init();
}
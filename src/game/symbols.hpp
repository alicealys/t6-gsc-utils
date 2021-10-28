#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	// Functions

	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{0x5C6F10, 0x6B9D20};
	WEAK symbol<void(const char* cmdName, void(), cmd_function_t* allocedCmd)> Cmd_AddCommandInternal{0x5B3070, 0x4DC2A0};
	WEAK symbol<const char*(int index)> Cmd_Argv{0x5608F0, 0x6B3D40};

	WEAK symbol<void(int clientNum)> ClientUserInfoChanged{0x4ED6A0, 0x427DC0};

	WEAK symbol<const dvar_t*(const char*)> Dvar_FindVar{0x563A70, 0x673C80};

	WEAK symbol<char*(const char*)> I_CleanStr{0x44F2B0, 0x483F40};

	WEAK symbol<void*(const char** pName, int* min_args, int* max_args)> Player_GetMethod{0x432480, 0x6F2DB0};
	WEAK symbol<void*(const char** pName, int* type, int* min_args, int* max_args)> Scr_GetCommonFunction{0x691110, 0x4EB070};

	WEAK symbol<void(scriptInstance_t inst, gentity_s* ent)> Scr_AddEntity{0x4C20F0, 0x5D8F80};
	WEAK symbol<void(scriptInstance_t inst, float value)> Scr_AddFloat{0x579130, 0x503480};
	WEAK symbol<void(scriptInstance_t inst, int value)> Scr_AddInt{0x57AFF0, 0x643A40};
	WEAK symbol<void(scriptInstance_t inst, const char* value)> Scr_AddString{0x4F1650, 0x6A7A70};
	WEAK symbol<void(scriptInstance_t inst, float* value)> Scr_AddVector{0x4C1A40, 0x4FAB00};
	WEAK symbol<void(scriptInstance_t inst, unsigned int id)> Scr_AddObject{0x539B50, 0x584DF0};

	WEAK symbol<unsigned int(scriptInstance_t inst)> AllocObject{0x6FB1B0, 0x6FE9D0};
	WEAK symbol<void(scriptInstance_t inst, unsigned int id)> RemoveRefToObject{0x6FB1B0, 0x550DC0};

	WEAK symbol<void(gentity_s* ent, unsigned int stringValue, unsigned int paramcount)> Scr_Notify{0x644AB0, 0x495420};
	WEAK symbol<void(scriptInstance_t inst, int clientNum, unsigned int id, unsigned int stringValue, unsigned int paramcount)> Scr_NotifyId{0x4FDC10, 0x4BC0D0};
	WEAK symbol<void(int entnum, unsigned int classnum, unsigned int stringValue, unsigned int paramcount)> Scr_NotifyNum{0x578690, 0x664770};

	WEAK symbol<unsigned int(const char* str, unsigned int user)> SL_GetString{0x602C40, 0x4601E0};

	WEAK symbol<int(scriptInstance_t inst)> Scr_GetNumParam{0x42E990, 0x680EA0};
	WEAK symbol<gentity_s*(scriptInstance_t inst, int index)> Scr_GetEntity{0x48F250, 0x489100};
	WEAK symbol<float(scriptInstance_t inst, int index)> Scr_GetFloat{0x633400, 0x625EE0};
	WEAK symbol<int(scriptInstance_t inst, int index)> Scr_GetInt{0x45D840, 0x49A060};
	WEAK symbol<const char*(scriptInstance_t inst, int index)> Scr_GetString{0x67C6A0, 0x488110};
	WEAK symbol<void(scriptInstance_t inst, int index, float* out)> Scr_GetVector{0x4CBCC0, 0x4BB100};
	WEAK symbol<int(scriptInstance_t inst, const char* filename, 
		const char* name, unsigned int* checksum, bool errorIfMissing)> Scr_GetFunctionHandle{0x416D30, 0x53E5A0};

	WEAK symbol<void(int clientNum, int type, const char* command)> SV_GameSendServerCommand{0x45D7D0, 0x40D450};

	// Variables

	WEAK symbol<gentity_s*> g_entities{0x21EF7C0, 0x21C13C0};
	WEAK symbol<unsigned int> levelEntityId{0x2E1A51C, 0x2DEA81C};
}

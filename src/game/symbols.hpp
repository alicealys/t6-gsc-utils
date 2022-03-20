#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	// Functions

	WEAK symbol<int(const char* str)> BG_StringHashValue{0x6BB660, 0x612780};

	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_InsertText{0x4A5FF0, 0x702E40};
	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{0x5C6F10, 0x6B9D20};
	WEAK symbol<void(int localClientNum, int controllerIndex, const char* text)> Cmd_ExecuteSingleCommand{0x6B9C10, 0x43B8D0};
	WEAK symbol<void(const char* cmdName, void(), cmd_function_t* allocedCmd)> Cmd_AddCommandInternal{0x5B3070, 0x4DC2A0};
	WEAK symbol<const char*(int index)> Cmd_Argv{0x5608F0, 0x6B3D40};
	WEAK symbol<void(const char* cmdName)> Cmd_RemoveCommand{0x6033D0, 0x65EB30};

	WEAK symbol<void(int clientNum)> ClientUserInfoChanged{0x4ED6A0, 0x427DC0};

	WEAK symbol<const dvar_t*(const char*)> Dvar_FindVar{0x563A70, 0x673C80};
	WEAK symbol<int(const dvar_t*)> Dvar_GetInt{0x44DFC0, 0x6909F0};
	WEAK symbol<dvar_t*(const char* dvarName, int value, int min, int max, 
		unsigned int flags, const char* description)> Dvar_RegisterInt{0x5F24E0, 0x5A3D70};

	WEAK symbol<XAssetHeader(XAssetType type, const char* name, bool errorIfMissing, int waitTime)> DB_FindXAssetHeader{0x6F9030, 0x43F460};

	WEAK symbol<char*(const char*)> I_CleanStr{0x44F2B0, 0x483F40};

	WEAK symbol<void*(const char** pName, int* min_args, int* max_args)> Player_GetMethod{0x432480, 0x6F2DB0};
	WEAK symbol<void*(const char** pName, int* type, int* min_args, int* max_args)> Scr_GetCommonFunction{0x691110, 0x4EB070};
	WEAK symbol<void*(const char** pName, int* type, int* min_args, int* max_args)> Scr_GetMethod{0x59D090, 0x486120};

	WEAK symbol<void(scriptInstance_t inst, gentity_s* ent)> Scr_AddEntity{0x4C20F0, 0x5D8F80};
	WEAK symbol<void(scriptInstance_t inst, float value)> Scr_AddFloat{0x579130, 0x503480};
	WEAK symbol<void(scriptInstance_t inst, int value)> Scr_AddInt{0x57AFF0, 0x643A40};
	WEAK symbol<void(scriptInstance_t inst, const char* value)> Scr_AddString{0x4F1650, 0x6A7A70};
	WEAK symbol<void(scriptInstance_t inst, float* value)> Scr_AddVector{0x4C1A40, 0x4FAB00};
	WEAK symbol<void(scriptInstance_t inst, unsigned int id)> Scr_AddObject{0x539B50, 0x584DF0};

	WEAK symbol<void(scriptInstance_t inst)> Scr_ClearOutParams{0x591410, 0x627540};

	WEAK symbol<unsigned int(scriptInstance_t inst)> AllocObject{0x6FB1B0, 0x6FE9D0};
	WEAK symbol<ObjectVariableValue*(scriptInstance_t inst, unsigned int* index)> AllocVariable{0x8F3150, 0x8F1EB0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int id)> AllocThread{0x49FDF0, 0x482C40};
	WEAK symbol<void(scriptInstance_t inst, unsigned int id)> RemoveRefToObject{0x6FB1B0, 0x550DC0};
	WEAK symbol<void(scriptInstance_t inst, const float* vectorValue)> RemoveRefToVector{0x50C580, 0x432600};
	WEAK symbol<void(scriptInstance_t inst, const VariableValue* value)> AddRefToValue{0x488FA0, 0x4E5060};
	WEAK symbol<void(scriptInstance_t inst, const int type, VariableUnion value)> RemoveRefToValue{0x55E820, 0x42E7D0};
	WEAK symbol<void(scriptInstance_t inst, unsigned int parentId, unsigned int index)> RemoveVariableValue{0x56A820, 0x6CC320};

	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int id)> FindVariable{0x6EB1B0, 0x5FE180};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int id)> GetVariable{0, 0x5CD170};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int name)> GetNewVariable{0x5AD920, 0x5C18A0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)> GetNewArrayVariable{0x5D80C0, 0x42C260};

	WEAK symbol<void(unsigned int classnum, int entnum, int offset)> Scr_SetObjectField{0x5B9820, 0x43F2A0};
	WEAK symbol<VariableValue(scriptInstance_t inst, unsigned int classnum, int entnum, int clientNum, int offset)> GetEntityFieldValue{0x693130, 0x6DD0E0};

	WEAK symbol<void(gentity_s* ent, unsigned int stringValue, unsigned int paramcount)> Scr_Notify{0x644AB0, 0x495420};
	WEAK symbol<void(scriptInstance_t inst, int clientNum, unsigned int id, unsigned int stringValue, unsigned int paramcount)> Scr_NotifyId{0x4FDC10, 0x4BC0D0};
	WEAK symbol<void(int entnum, unsigned int classnum, unsigned int stringValue, unsigned int paramcount)> Scr_NotifyNum{0x578690, 0x664770};

	WEAK symbol<unsigned int(const char* str, unsigned int user)> SL_GetString{0x602C40, 0x4601E0};
	WEAK symbol<const char*(unsigned int stringValue)> SL_ConvertToString{0x422D10, 0x532230};
	WEAK symbol<unsigned int(const char* str, bool is_static)> SL_GetCanonicalString{0x692740, 0x546640};

	WEAK symbol<int(scriptInstance_t inst)> Scr_GetNumParam{0x42E990, 0x680EA0};
	WEAK symbol<gentity_s*(scriptInstance_t inst, int index)> Scr_GetEntity{0x48F250, 0x489100};
	WEAK symbol<float(scriptInstance_t inst, int index)> Scr_GetFloat{0x633400, 0x625EE0};
	WEAK symbol<int(scriptInstance_t inst, int index)> Scr_GetInt{0x45D840, 0x49A060};
	WEAK symbol<const char*(scriptInstance_t inst, int index)> Scr_GetString{0x67C6A0, 0x488110};
	WEAK symbol<void(scriptInstance_t inst, int index, float* out)> Scr_GetVector{0x4CBCC0, 0x4BB100};
	WEAK symbol<const float*(scriptInstance_t inst, const float* v)> Scr_AllocVector{0x510D30, 0x44D8C0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int threadId)> Scr_GetSelf{0x4682C0, 0x6C4E60};
	WEAK symbol<int(scriptInstance_t inst, const char* filename, 
		const char* name, unsigned int* checksum, bool errorIfMissing)> Scr_GetFunctionHandle{0x416D30, 0x53E5A0};
	WEAK symbol<void(scriptInstance_t inst, unsigned int classnum, char const* name, unsigned int offset)> Scr_AddClassField{0x6B7620, 0x438AD0};
	WEAK symbol<unsigned int(scriptInstance_t inst, int entnum, unsigned int classnum, int clientNum)> Scr_GetEntityId{0x5765B0, 0x488430};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int localId)> GetStartLocalId{0x402760, 0x5D6CD0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int localId)> Scr_TerminateRunningThread{0x8F45A0, 0x8F3300};
	WEAK symbol<void(scriptInstance_t inst, const char* error, bool force_terminal)> Scr_Error{0x403AE0, 0x5E81C0};
	WEAK symbol<void(scriptInstance_t inst, unsigned int paramIndex, const char* error)> Scr_ParamError{0x415A30, 0x617CA0};
	WEAK symbol<void(scriptInstance_t inst, const char* error)> Scr_ObjectError{0x561660, 0x6B67E0};

	WEAK symbol<gentity_s*(scr_entref_t entref)> GetPlayerEntity{0x48B760, 0x4BF6F0};

	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int localId, const char* pos, unsigned int paramcount)> VM_Execute{0x8F9550, 0x8F82B0};

	WEAK symbol<void(int clientNum, const char* reason)> SV_GameDropClient{0x6D2980, 0x677690};
	WEAK symbol<bool(int clientNum)> SV_IsTestClient{0x4E64B0, 0x42C300};
	WEAK symbol<void(int clientNum, int type, const char* command)> SV_GameSendServerCommand{0x45D7D0, 0x40D450};

	WEAK symbol<void*(int valueIndex)> Sys_GetValue{0x5EFBA0, 0x59A740};
	WEAK symbol<int()> Sys_Milliseconds{0x57EC40, 0x6F4C00};

	WEAK symbol<void*(jmp_buf* Buf, int Value)> longjmp{0xA78870, 0xA71AD0};
	WEAK symbol<int(jmp_buf* Buf, int a6, int a7, int a9)> _setjmp{0xA77B10, 0xA70D70};

	// Variables

	WEAK symbol<int> g_script_error_level{0x2E23BC8, 0x2DF3EC8};
	WEAK symbol<jmp_buf> g_script_error{0x2E22B48, 0x2DF2E48};

	WEAK symbol<scrVmPub_t> scr_VmPub{0x2E1A5D0, 0x2DEA8D0};
	WEAK symbol<scrVarGlob_t> scr_VarGlob{0x2E1A100, 0x2DEA400};
	WEAK symbol<scrVarPub_t> scr_VarPub{0x2E1A500, 0x2DEA800};
	WEAK symbol<int> scr_starttime{0x2E23C48, 0x2DF3F48};
	WEAK symbol<function_stack_t> fs{0x2E23C08, 0x2DF3F08};

	WEAK symbol<unsigned short> sv_configstrings{0x28E67AC, 0x28B70AC};

	WEAK symbol<scr_classStruct_t*> g_classMap{0xD6FFE0, 0xD64E60};

	WEAK symbol<BuiltinFunctionDef> common_functions{0xC75160, 0xC6ACC8};
	WEAK symbol<BuiltinFunctionDef> functions{0xD4DD28, 0xD42BC8};

	WEAK symbol<BuiltinMethodDef> player_methods{0xC726A8, 0xC68218};
	WEAK symbol<BuiltinMethodDef> scriptEnt_methods{0xC76F18, 0xC6CA90};
	WEAK symbol<BuiltinMethodDef> scriptVehicle_methods{0xC77538, 0xC6D0B0};
	WEAK symbol<BuiltinMethodDef> hudElem_methods{0xC74580, 0xC6A0E8};
	WEAK symbol<BuiltinMethodDef> helicopter_methods{0xC74E98, 0xC6AA00};
	WEAK symbol<BuiltinMethodDef> actor_methods{0xC871A0, 0xC7CC70};
	WEAK symbol<BuiltinMethodDef> builtInCommon_methods{0xC76600, 0xC6C168};
	WEAK symbol<BuiltinMethodDef> builtIn_methods{0xD4FC78, 0xD44B18};

	WEAK symbol<gentity_s> g_entities{0x21EF7C0, 0x21C13C0};
	WEAK symbol<unsigned int> levelEntityId{0x2E1A51C, 0x2DEA81C};

	WEAK symbol<client_s> svs_clients{0x291C0C0, 0x28EC9C0};

	namespace plutonium
	{
		WEAK symbol<int(const char* fmt, ...)> printf{0x209F30F0, 0x209F30F0};
	}
}

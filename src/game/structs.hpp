#pragma once

namespace game
{
	typedef float vec_t;
	typedef vec_t vec2_t[2];
	typedef vec_t vec3_t[3];
	typedef vec_t vec4_t[4];

	enum scriptInstance_t
	{
		SCRIPTINSTANCE_SERVER,
		SCRIPTINSTANCE_CLIENT,
	};

	struct scr_entref_t
	{
		unsigned short entnum;
		unsigned short classnum;
		int client;
	};

	struct BuiltinMethodDef
	{
		const char* actionString;
		unsigned int constId;
		int min_args;
		int max_args;
		void(__cdecl* actionFunc)(scr_entref_t);
		int type;
	};

	struct BuiltinFunctionDef
	{
		const char* actionString;
		unsigned int constId;
		int min_args;
		int max_args;
		void(__cdecl* actionFunc)();
		int type;
	};

	enum scriptType_e
	{
		SCRIPT_NONE = 0,
		SCRIPT_OBJECT = 1,
		SCRIPT_STRING = 2,
		SCRIPT_ISTRING = 3,
		SCRIPT_VECTOR = 4,
		SCRIPT_FLOAT = 6,
		SCRIPT_INTEGER = 7,
		SCRIPT_CODEPOS = 8,
		SCRIPT_END = 9,
		SCRIPT_FUNCTION = 10,
		SCRIPT_THREAD = 0xE,
		SCRIPT_NOTIFY_THREAD = 0xF,
		SCRIPT_TIME_THREAD = 0x10,
		SCRIPT_STRUCT = 18,
		SCRIPT_ENTITY = 20,
		SCRIPT_ARRAY = 21,
		SCRIPT_FREE = 0x17,
	};

	struct VariableStackBuffer
	{
		const char* pos;
		unsigned __int16 size;
		unsigned __int16 bufLen;
		unsigned __int16 localId;
		char time;
		char buf[1];
	};

	union VariableUnion
	{
		int intValue;
		unsigned int uintValue;
		float floatValue;
		unsigned int stringValue;
		const float* vectorValue;
		const char* codePosValue;
		unsigned int pointerValue;
		VariableStackBuffer* stackValue;
		unsigned int entityOffset;
	};

	struct VariableValue
	{
		int type;
		VariableUnion u;
	};

	struct function_stack_t
	{
		char* pos;
		VariableValue* top;
		unsigned int localId;
		unsigned int localVarCount;
		VariableValue* startTop;
	};

	struct function_frame_t
	{
		function_stack_t fs;
	};

	struct scrVmPub_t
	{
		unsigned int* localVars;
		VariableValue* maxstack;
		int function_count;
		function_frame_t* function_frame;
		VariableValue* top;
		bool abort_on_error;
		bool terminal_error;
		bool block_execution;
		unsigned int inparamcount;
		unsigned int outparamcount;
		function_frame_t function_frame_start[32];
		VariableValue stack[2048];
		void(__cdecl* notifyListeners[1])(unsigned int, unsigned int);
	};

	struct scrVarPub_t
	{
		const char* fieldBuffer;
		bool developer;
		bool developer_script;
		bool evaluate;
		const char* error_message;
		int error_index;
		unsigned int time;
		unsigned int timeArrayId;
		unsigned int pauseArrayId;
		unsigned int levelId;
		unsigned int gameId;
		unsigned int animId;
		unsigned int freeEntList;
		unsigned int tempVariable;
		bool bInited;
		unsigned __int16 savecount;
		unsigned int checksum;
		unsigned int entId;
		unsigned int entFieldName;
		void* programHunkUser;
		char* programBuffer;
		char* endScriptBuffer;
		unsigned __int16* saveIdMap;
		unsigned __int16* saveIdMapRev;
		unsigned int numScriptThreads;
		unsigned int numScriptValues;
		unsigned int numScriptObjects;
		char* varUsagePos;
		int ext_threadcount;
		int totalObjectRefCount;
		volatile int totalVectorRefCount;
		int allocationCount;
	};

	struct ObjectVariableChildren
	{
		unsigned int firstChild;
		unsigned int lastChild;
	};

	struct ObjectVariableValue_u_f
	{
		unsigned __int16 prev;
		unsigned __int16 next;
	};

	union ObjectVariableValue_u_o_u
	{
		unsigned __int16 size;
		unsigned __int16 entnum;
		unsigned __int16 nextEntId;
		unsigned __int16 self;
	};

	struct	ObjectVariableValue_u_o
	{
		unsigned __int16 refCount;
		ObjectVariableValue_u_o_u u;
	};

	union ObjectVariableValue_w
	{
		unsigned int type;
		unsigned int classnum;
		unsigned int notifyName;
		unsigned int waitTime;
		unsigned int parentLocalId;
	};

	struct ChildVariableValue_u_f
	{
		unsigned __int16 prev;
		unsigned __int16 next;
	};

	union ChildVariableValue_u
	{
		ChildVariableValue_u_f f;
		VariableUnion u;
	};

	struct ChildBucketMatchKeys_keys
	{
		unsigned __int16 name_hi;
		unsigned __int16 parentId;
	};

	union ChildBucketMatchKeys
	{
		ChildBucketMatchKeys_keys keys;
		unsigned int match;
	};

	struct	ChildVariableValue
	{
		ChildVariableValue_u u;
		unsigned int next;
		char pad[4];
		char type;
		char name_lo;
		char _pad[2];
		ChildBucketMatchKeys k;
		unsigned int nextSibling;
		unsigned int prevSibling;
	};

	union ObjectVariableValue_u
	{
		ObjectVariableValue_u_f f;
		ObjectVariableValue_u_o o;
	};

	struct ObjectVariableValue
	{
		ObjectVariableValue_u u;
		ObjectVariableValue_w w;
	};

	struct scrVarGlob_t
	{
		ObjectVariableValue* objectVariableValue;
		__declspec(align(128)) ObjectVariableChildren* objectVariableChildren;
		__declspec(align(128)) unsigned __int16* childVariableBucket;
		__declspec(align(128)) ChildVariableValue* childVariableValue;
	};

	struct scr_classStruct_t
	{
		unsigned __int16 id;
		unsigned __int16 entArrayId;
		char charId;
		const char* name;
	};

	struct gclient_s
	{
		char __pad0[0x18];
		int eflags;
		char __pad1[0x5668];
		int flags;
	};

	struct gentity_s
	{
		int entity_num;
		char __pad0[0x150];
		gclient_s* client;
		char __pad1[0x1C4];
	};

	enum clientState_t
	{
		CS_FREE,
		CS_ZOMBIE,
		CS_RECONNECTING,
		CS_CONNECTED,
		CS_CLIENTLOADING,
		CS_ACTIVE,
	};

	enum netsrc_t
	{
		NS_CLIENT1 = 0x0,
		NS_CLIENT2 = 0x1,
		NS_CLIENT3 = 0x2,
		NS_CLIENT4 = 0x3,
		NS_SERVER = 0x4,
		NS_PACKET = 0x5,
		NS_NULL = -1,
	};

	enum netadrtype_t
	{
		NA_BOT = 0x0,
		NA_BAD = 0x1,
		NA_LOOPBACK = 0x2,
		NA_BROADCAST = 0x3,
		NA_IP = 0x4,
	};

	struct netadr_t
	{
		union
		{
			unsigned char ip[4];
			unsigned int inaddr;
		};
		unsigned __int16 port;
		netadrtype_t type;
		netsrc_t localNetID;
		unsigned __int16 serverID;
	};

	static_assert(sizeof(netadr_t) == 0x14);

	struct netProfileInfo_t
	{
		unsigned char __pad0[0x5E0];
	};

	struct netchan_t
	{
		int outgoingSequence;
		netsrc_t sock;
		int dropped;
		int incomingSequence;
		netadr_t remoteAddress;
		int qport;
		int fragmentSequence;
		int fragmentLength;
		unsigned char* fragmentBuffer;
		int fragmentBufferSize;
		int unsentFragments;
		int unsentOnLoan;
		int unsentFragmentStart;
		int unsentLength;
		unsigned char* unsentBuffer;
		int unsentBufferSize;
		int reliable_fragments;
		unsigned char fragment_send_count[128];
		unsigned int fragment_ack[4];
		int lowest_send_count;
		netProfileInfo_t prof;
	};

	static_assert(sizeof(netchan_t) == 0x6C8);

	struct PredictedVehicleDef
	{
		bool fullPhysics;
		vec3_t origin;
		vec3_t angles;
		vec3_t tVel;
		vec3_t aVel;
		int serverTime;
	};

	static_assert(sizeof(PredictedVehicleDef) == 0x38);

	struct clientHeader_t
	{
		clientState_t state;
		int sendAsActive;
		int deltaMessage;
		int rateDelayed;
		int hasAckedBaselineData;
		int hugeSnapshotSent;
		netchan_t netchan;
		vec3_t predictedOrigin;
		int predictedOriginServerTime;
		int migrationState;
		PredictedVehicleDef predictedVehicle;
	};

	static_assert(sizeof(clientHeader_t) == 0x72C);

	struct client_s
	{
		clientHeader_t header;
		const char* dropReason;
		char userinfo[1024];
		unsigned char __pad0[0x3F75C];
		int bIsTestClient;
		unsigned char __pad1[0xDEF0];
	};

	static_assert(sizeof(client_s) == 0x4E180);

	struct cmd_function_t
	{
		cmd_function_t* next;
		const char* name;
		const char* autoCompleteDir;
		const char* autoCompleteExt;
		void(__cdecl* function)();
		int flags;
	};

	struct CmdArgs
	{
		int nesting;
		int localClientNum[8];
		int controllerIndex[8];
		void* itemDef[8];
		int argshift[8];
		int argc[8];
		const char** argv[8];
		char textPool[8192];
		const char* argvPool[512];
		int usedTextPool[8];
		int totalUsedArgvPool;
		int totalUsedTextPool;
	};

	enum dvarType_t
	{
		DVAR_TYPE_INVALID = 0x0,
		DVAR_TYPE_BOOL = 0x1,
		DVAR_TYPE_FLOAT = 0x2,
		DVAR_TYPE_FLOAT_2 = 0x3,
		DVAR_TYPE_FLOAT_3 = 0x4,
		DVAR_TYPE_FLOAT_4 = 0x5,
		DVAR_TYPE_INT = 0x6,
		DVAR_TYPE_ENUM = 0x7,
		DVAR_TYPE_STRING = 0x8,
		DVAR_TYPE_COLOR = 0x9,
		DVAR_TYPE_INT64 = 0xA,
		DVAR_TYPE_LINEAR_COLOR_RGB = 0xB,
		DVAR_TYPE_COLOR_XYZ = 0xC,
		DVAR_TYPE_COUNT = 0xD,
	};

	union DvarValue
	{
		bool enabled;
		int integer;
		unsigned int unsignedInt;
		__int64 integer64;
		unsigned __int64 unsignedInt64;
		float value; 
		vec4_t vector;
		const char* string;
		char color[4];
	};

	struct $A37BA207B3DDD6345C554D4661813EDD
	{
		int stringCount;
		const char* const* strings;
	};

	struct $9CA192F9DB66A3CB7E01DE78A0DEA53D
	{
		int min;
		int max;
	};

	struct $251C2428A496074035CACA7AAF3D55BD
	{
		float min;
		float max;
	};

	union DvarLimits
	{
		$A37BA207B3DDD6345C554D4661813EDD enumeration;
		$9CA192F9DB66A3CB7E01DE78A0DEA53D integer;
		$251C2428A496074035CACA7AAF3D55BD value;
		$251C2428A496074035CACA7AAF3D55BD vector;
	};

	struct dvar_t
	{
		const char* name;
		const char* description;
		int hash;
		unsigned int flags;
		dvarType_t type;
		bool modified;
		DvarValue current;
		DvarValue latched;
		DvarValue reset;
		DvarLimits domain;
		dvar_t* hashNext;
	};

	struct GSC_OBJ
	{
		char magic[8];
		unsigned int source_crc;
		unsigned int include_offset;
		unsigned int animtree_offset;
		unsigned int cseg_offset;
		unsigned int stringtablefixup_offset;
		unsigned int exports_offset;
		unsigned int imports_offset;
		unsigned int fixup_offset;
		unsigned int profile_offset;
		unsigned int cseg_size;
		unsigned __int16 name;
		unsigned __int16 stringtablefixup_count;
		unsigned __int16 exports_count;
		unsigned __int16 imports_count;
		unsigned __int16 fixup_count;
		unsigned __int16 profile_count;
		char include_count;
		char animtree_count;
		char flags;
	};

	struct GSC_EXPORT_ITEM
	{
		unsigned int checksum;
		unsigned int address;
		unsigned __int16 name;
		char param_count;
		char flags;
	};

	// gsc-tool
	enum class opcode : std::uint8_t
	{
		OP_End = 0x0,
		OP_Return = 0x1,
		OP_GetUndefined = 0x2,
		OP_GetZero = 0x3,
		OP_GetByte = 0x4,
		OP_GetNegByte = 0x5,
		OP_GetUnsignedShort = 0x6,
		OP_GetNegUnsignedShort = 0x7,
		OP_GetInteger = 0x8,
		OP_GetFloat = 0x9,
		OP_GetString = 0xA,
		OP_GetIString = 0xB,
		OP_GetVector = 0xC,
		OP_GetLevelObject = 0xD,
		OP_GetAnimObject = 0xE,
		OP_GetSelf = 0xF,
		OP_GetLevel = 0x10,
		OP_GetGame = 0x11,
		OP_GetAnim = 0x12,
		OP_GetAnimation = 0x13,
		OP_GetGameRef = 0x14,
		OP_GetFunction = 0x15,
		OP_CreateLocalVariable = 0x16,
		OP_SafeCreateLocalVariables = 0x17,
		OP_RemoveLocalVariables = 0x18,
		OP_EvalLocalVariableCached = 0x19,
		OP_EvalArray = 0x1A,
		OP_EvalLocalArrayRefCached = 0x1B,
		OP_EvalArrayRef = 0x1C,
		OP_ClearArray = 0x1D,
		OP_EmptyArray = 0x1E,
		OP_GetSelfObject = 0x1F,
		OP_EvalFieldVariable = 0x20,
		OP_EvalFieldVariableRef = 0x21,
		OP_ClearFieldVariable = 0x22,
		OP_SafeSetVariableFieldCached = 0x23,
		OP_SafeSetWaittillVariableFieldCached = 0x24,
		OP_ClearParams = 0x25,
		OP_CheckClearParams = 0x26,
		OP_EvalLocalVariableRefCached = 0x27,
		OP_SetVariableField = 0x28,
		OP_CallBuiltin = 0x29,
		OP_CallBuiltinMethod = 0x2A,
		OP_Wait = 0x2B,
		OP_WaitTillFrameEnd = 0x2C,
		OP_PreScriptCall = 0x2D,
		OP_ScriptFunctionCall = 0x2E,
		OP_ScriptFunctionCallPointer = 0x2F,
		OP_ScriptMethodCall = 0x30,
		OP_ScriptMethodCallPointer = 0x31,
		OP_ScriptThreadCall = 0x32,
		OP_ScriptThreadCallPointer = 0x33,
		OP_ScriptMethodThreadCall = 0x34,
		OP_ScriptMethodThreadCallPointer = 0x35,
		OP_DecTop = 0x36,
		OP_CastFieldObject = 0x37,
		OP_CastBool = 0x38,
		OP_BoolNot = 0x39,
		OP_BoolComplement = 0x3A,
		OP_JumpOnFalse = 0x3B,
		OP_JumpOnTrue = 0x3C,
		OP_JumpOnFalseExpr = 0x3D,
		OP_JumpOnTrueExpr = 0x3E,
		OP_Jump = 0x3F,
		OP_JumpBack = 0x40,
		OP_Inc = 0x41,
		OP_Dec = 0x42,
		OP_Bit_Or = 0x43,
		OP_Bit_Xor = 0x44,
		OP_Bit_And = 0x45,
		OP_Equal = 0x46,
		OP_NotEqual = 0x47,
		OP_LessThan = 0x48,
		OP_GreaterThan = 0x49,
		OP_LessThanOrEqualTo = 0x4A,
		OP_GreaterThanOrEqualTo = 0x4B,
		OP_ShiftLeft = 0x4C,
		OP_ShiftRight = 0x4D,
		OP_Plus = 0x4E,
		OP_Minus = 0x4F,
		OP_Multiply = 0x50,
		OP_Divide = 0x51,
		OP_Modulus = 0x52,
		OP_SizeOf = 0x53,
		OP_WaitTillMatch = 0x54,
		OP_WaitTill = 0x55,
		OP_Notify = 0x56,
		OP_EndOn = 0x57,
		OP_VoidCodePos = 0x58,
		OP_Switch = 0x59,
		OP_EndSwitch = 0x5A,
		OP_Vector = 0x5B,
		OP_GetHash = 0x5C,
		OP_RealWait = 0x5D,
		OP_VectorConstant = 0x5E,
		OP_IsDefined = 0x5F,
		OP_VectorScale = 0x60,
		OP_AnglesToUp = 0x61,
		OP_AnglesToRight = 0x62,
		OP_AnglesToForward = 0x63,
		OP_AngleClamp180 = 0x64,
		OP_VectorToAngles = 0x65,
		OP_Abs = 0x66,
		OP_GetTime = 0x67,
		OP_GetDvar = 0x68,
		OP_GetDvarInt = 0x69,
		OP_GetDvarFloat = 0x6A,
		OP_GetDvarVector = 0x6B,
		OP_GetDvarColorRed = 0x6C,
		OP_GetDvarColorGreen = 0x6D,
		OP_GetDvarColorBlue = 0x6E,
		OP_GetDvarColorAlpha = 0x6F,
		OP_FirstArrayKey = 0x70,
		OP_NextArrayKey = 0x71,
		OP_ProfileStart = 0x72,
		OP_ProfileStop = 0x73,
		OP_SafeDecTop = 0x74,
		OP_Nop = 0x75,
		OP_Abort = 0x76,
		OP_Object = 0x77,
		OP_ThreadObject = 0x78,
		OP_EvalLocalVariable = 0x79,
		OP_EvalLocalVariableRef = 0x7A,
		OP_DevblockBegin = 0x7B,
		OP_DevblockEnd = 0x7C,
		OP_Breakpoint = 0x7D,
		OP_Count = 0x7E,
	};

	enum XAssetType
	{
		ASSET_TYPE_XMODELPIECES = 0x0,
		ASSET_TYPE_PHYSPRESET = 0x1,
		ASSET_TYPE_PHYSCONSTRAINTS = 0x2,
		ASSET_TYPE_DESTRUCTIBLEDEF = 0x3,
		ASSET_TYPE_XANIMPARTS = 0x4,
		ASSET_TYPE_XMODEL = 0x5,
		ASSET_TYPE_MATERIAL = 0x6,
		ASSET_TYPE_TECHNIQUE_SET = 0x7,
		ASSET_TYPE_IMAGE = 0x8,
		ASSET_TYPE_SOUND = 0x9,
		ASSET_TYPE_SOUND_PATCH = 0xA,
		ASSET_TYPE_CLIPMAP = 0xB,
		ASSET_TYPE_CLIPMAP_PVS = 0xC,
		ASSET_TYPE_COMWORLD = 0xD,
		ASSET_TYPE_GAMEWORLD_SP = 0xE,
		ASSET_TYPE_GAMEWORLD_MP = 0xF,
		ASSET_TYPE_MAP_ENTS = 0x10,
		ASSET_TYPE_GFXWORLD = 0x11,
		ASSET_TYPE_LIGHT_DEF = 0x12,
		ASSET_TYPE_UI_MAP = 0x13,
		ASSET_TYPE_FONT = 0x14,
		ASSET_TYPE_FONTICON = 0x15,
		ASSET_TYPE_MENULIST = 0x16,
		ASSET_TYPE_MENU = 0x17,
		ASSET_TYPE_LOCALIZE_ENTRY = 0x18,
		ASSET_TYPE_WEAPON = 0x19,
		ASSET_TYPE_WEAPONDEF = 0x1A,
		ASSET_TYPE_WEAPON_VARIANT = 0x1B,
		ASSET_TYPE_WEAPON_FULL = 0x1C,
		ASSET_TYPE_ATTACHMENT = 0x1D,
		ASSET_TYPE_ATTACHMENT_UNIQUE = 0x1E,
		ASSET_TYPE_WEAPON_CAMO = 0x1F,
		ASSET_TYPE_SNDDRIVER_GLOBALS = 0x20,
		ASSET_TYPE_FX = 0x21,
		ASSET_TYPE_IMPACT_FX = 0x22,
		ASSET_TYPE_AITYPE = 0x23,
		ASSET_TYPE_MPTYPE = 0x24,
		ASSET_TYPE_MPBODY = 0x25,
		ASSET_TYPE_MPHEAD = 0x26,
		ASSET_TYPE_CHARACTER = 0x27,
		ASSET_TYPE_XMODELALIAS = 0x28,
		ASSET_TYPE_RAWFILE = 0x29,
		ASSET_TYPE_STRINGTABLE = 0x2A,
		ASSET_TYPE_LEADERBOARD = 0x2B,
		ASSET_TYPE_XGLOBALS = 0x2C,
		ASSET_TYPE_DDL = 0x2D,
		ASSET_TYPE_GLASSES = 0x2E,
		ASSET_TYPE_EMBLEMSET = 0x2F,
		ASSET_TYPE_SCRIPTPARSETREE = 0x30,
		ASSET_TYPE_KEYVALUEPAIRS = 0x31,
		ASSET_TYPE_VEHICLEDEF = 0x32,
		ASSET_TYPE_MEMORYBLOCK = 0x33,
		ASSET_TYPE_ADDON_MAP_ENTS = 0x34,
		ASSET_TYPE_TRACER = 0x35,
		ASSET_TYPE_SKINNEDVERTS = 0x36,
		ASSET_TYPE_QDB = 0x37,
		ASSET_TYPE_SLUG = 0x38,
		ASSET_TYPE_FOOTSTEP_TABLE = 0x39,
		ASSET_TYPE_FOOTSTEPFX_TABLE = 0x3A,
		ASSET_TYPE_ZBARRIER = 0x3B,
		ASSET_TYPE_COUNT = 0x3C,
		ASSET_TYPE_STRING = 0x3C,
		ASSET_TYPE_ASSETLIST = 0x3D,
		ASSET_TYPE_REPORT = 0x3E,
		ASSET_TYPE_DEPEND = 0x3F,
		ASSET_TYPE_FULL_COUNT = 0x40,
	};

	struct ScriptParseTree
	{
		const char* name;
		int len;
		GSC_OBJ* obj;
	};

	struct objFileInfo_t
	{
		GSC_OBJ* activeVersion;
		char __pad[0x24];
	};

	union XAssetHeader
	{
		ScriptParseTree* scriptParseTree;
	};
}
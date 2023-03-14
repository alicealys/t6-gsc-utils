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
		char __pad0[0x7C];
		ObjectVariableChildren* objectVariableChildren;
		char __pad1[0x7C];
		unsigned __int16* childVariableBucket;
		char __pad2[0x7C];
		ChildVariableValue* childVariableValue;
	};

	struct scr_classStruct_t
	{
		unsigned __int16 id;
		unsigned __int16 entArrayId;
		char charId;
		const char* name;
	};

	enum gclientFlag
	{
		NOCLIP = 1 << 0,
		UFO = 1 << 1,
	};

	enum entityFlag
	{
		FL_GODMODE = 1 << 0,
		FL_DEMI_GODMODE = 1 << 1,
		FL_NOTARGET = 1 << 2,
		FL_SUPPORTS_LINKTO = 1 << 12,
	}; // TODO: Finish

	struct gclient_s
	{
		char __pad0[0x18];
		int eflags;
		char __pad1[0x5668];
		int flags;
	};

	struct EntHandle
	{
		unsigned __int16 number;
		unsigned __int16 infoIndex;
	};

	/* 2885 */
	struct SentientHandle
	{
		unsigned __int16 number;
		unsigned __int16 infoIndex;
	};

	/* 46 */
	enum team_t : __int32
	{
		TEAM_FREE = 0x0,
		TEAM_BAD = 0x0,
		TEAM_ALLIES = 0x1,
		TEAM_AXIS = 0x2,
		TEAM_THREE = 0x3,
		TEAM_FOUR = 0x4,
		TEAM_FIVE = 0x5,
		TEAM_SIX = 0x6,
		TEAM_SEVEN = 0x7,
		TEAM_EIGHT = 0x8,
		TEAM_NUM_PLAYING_TEAMS = 0x9,
		TEAM_SPECTATOR = 0x9,
		TEAM_NUM_TEAMS = 0xA,
		TEAM_LOCALPLAYERS = 0xB,
		TEAM_FIRST_PLAYING_TEAM = 0x1,
		TEAM_LAST_PLAYING_TEAM = 0x8,
	};

	/* 343 */
	enum TurretRotateState : __int32
	{
		TURRET_ROTATE_STOPPED = 0x0,
		TURRET_ROTATE_STOPPING = 0x1,
		TURRET_ROTATE_MOVING = 0x2,
	};

	struct TurretInfo
	{
		bool inuse;
		int state;
		int flags;
		int fireTime;
		EntHandle manualTarget;
		EntHandle target;
		vec3_t targetPos;
		int targetTime;
		vec3_t missOffsetNormalized;
		float arcmin[2];
		float arcmax[2];
		float initialYawmin;
		float initialYawmax;
		float forwardAngleDot;
		float dropPitch;
		float scanningPitch;
		int convergenceTime[2];
		int suppressTime;
		float maxRangeSquared;
		SentientHandle detachSentient;
		int stance;
		int prevStance;
		int fireSndDelay;
		float accuracy;
		vec3_t userOrigin;
		int prevSentTarget;
		float aiSpread;
		float playerSpread;
		team_t eTeam;
		float heatVal;
		bool overheating;
		int fireBarrel;
		float scanSpeed;
		float scanDecelYaw;
		int scanPauseTime;
		vec3_t originError;
		vec3_t anglesError;
		float pitchCap;
		int triggerDown;
		unsigned int fireSnd;
		unsigned int fireSndPlayer;
		unsigned int startFireSnd;
		unsigned int startFireSndPlayer;
		unsigned int loopFireEnd;
		unsigned int loopFireEndPlayer;
		unsigned int rotateLoopSnd;
		unsigned int rotateLoopSndPlayer;
		unsigned int rotateStopSnd;
		unsigned int rotateStopSndPlayer;
		int sndIsFiring;
		vec3_t targetOffset;
		float onTargetAngle;
		TurretRotateState turretRotateState;
		vec3_t previousAngles;
	};

	struct gentity_s
	{
		int number;
		char __pad0[4];
		int eFlags2;
		char __pad1[328];
		gclient_s* client; // 340
		char __pad2[0x4];
		char __pad3[0x4];
		TurretInfo* pTurretInfo;
		char __pad4[0x24];
		int flags; // 392
		char __pad5[0x190];
	};

	static_assert(sizeof(gentity_s) == 0x31C);

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

	struct WeaponDef
	{
		const char* szOverlayName;
		void** gunXModel;
		void* handXModel;
		const char* szModeName;
		unsigned __int16* notetrackSoundMapKeys;
		unsigned __int16* notetrackSoundMapValues;
		int playerAnimType;
		int weapType;
		int weapClass;
		int penetrateType;
		int impactType;
		int inventoryType;
		int fireType;
		int clipType;
		int barrelType;
		int itemIndex;
		const char* parentWeaponName;
		int iJamFireTime;
		int overheatWeapon;
		float overheatRate;
		float cooldownRate;
		float overheatEndVal;
		bool coolWhileFiring;
		bool fuelTankWeapon;
		int iTankLifeTime;
		int offhandClass;
		int offhandSlot;
		int stance;
		const void* viewFlashEffect;
		const void* worldFlashEffect;
		const void* barrelCooldownEffect;
		int barrelCooldownMinCount;
		vec3_t vViewFlashOffset;
		vec3_t vWorldFlashOffset;
		const char* pickupSound;
		const char* pickupSoundPlayer;
		const char* ammoPickupSound;
		const char* ammoPickupSoundPlayer;
		const char* projectileSound;
		const char* pullbackSound;
		const char* pullbackSoundPlayer;
		const char* fireSound;
		const char* fireSoundPlayer;
		const char* fireLoopSound;
		const char* fireLoopSoundPlayer;
		const char* fireLoopEndSound;
		const char* fireLoopEndSoundPlayer;
		const char* fireStartSound;
		const char* fireStopSound;
		const char* fireKillcamSound;
		const char* fireStartSoundPlayer;
		const char* fireStopSoundPlayer;
		const char* fireKillcamSoundPlayer;
		const char* fireLastSound;
		const char* fireLastSoundPlayer;
		const char* emptyFireSound;
		const char* emptyFireSoundPlayer;
		const char* crackSound;
		const char* whizbySound;
		const char* meleeSwipeSound;
		const char* meleeSwipeSoundPlayer;
		const char* meleeHitSound;
		const char* meleeMissSound;
		const char* rechamberSound;
		const char* rechamberSoundPlayer;
		const char* reloadSound;
		const char* reloadSoundPlayer;
		const char* reloadEmptySound;
		const char* reloadEmptySoundPlayer;
		const char* reloadStartSound;
		const char* reloadStartSoundPlayer;
		const char* reloadEndSound;
		const char* reloadEndSoundPlayer;
		const char* rotateLoopSound;
		const char* rotateLoopSoundPlayer;
		const char* rotateStopSound;
		const char* rotateStopSoundPlayer;
		const char* deploySound;
		const char* deploySoundPlayer;
		const char* finishDeploySound;
		const char* finishDeploySoundPlayer;
		const char* breakdownSound;
		const char* breakdownSoundPlayer;
		const char* finishBreakdownSound;
		const char* finishBreakdownSoundPlayer;
		const char* detonateSound;
		const char* detonateSoundPlayer;
		const char* nightVisionWearSound;
		const char* nightVisionWearSoundPlayer;
		const char* nightVisionRemoveSound;
		const char* nightVisionRemoveSoundPlayer;
		const char* altSwitchSound;
		const char* altSwitchSoundPlayer;
		const char* raiseSound;
		const char* raiseSoundPlayer;
		const char* firstRaiseSound;
		const char* firstRaiseSoundPlayer;
		const char* adsRaiseSoundPlayer;
		const char* adsLowerSoundPlayer;
		const char* putawaySound;
		const char* putawaySoundPlayer;
		const char* overheatSound;
		const char* overheatSoundPlayer;
		const char* adsZoomSound;
		const char* shellCasing;
		const char* shellCasingPlayer;
		const char** bounceSound;
		const char* standMountedWeapdef;
		const char* crouchMountedWeapdef;
		const char* proneMountedWeapdef;
		int standMountedIndex;
		int crouchMountedIndex;
		int proneMountedIndex;
		const void* viewShellEjectEffect;
		const void* worldShellEjectEffect;
		const void* viewLastShotEjectEffect;
		const void* worldLastShotEjectEffect;
		vec3_t vViewShellEjectOffset;
		vec3_t vWorldShellEjectOffset;
		vec3_t vViewShellEjectRotation;
		vec3_t vWorldShellEjectRotation;
		void* reticleCenter;
		void* reticleSide;
		int iReticleCenterSize;
		int iReticleSideSize;
		int iReticleMinOfs;
		int activeReticleType;
		vec3_t vStandMove;
		vec3_t vStandRot;
		vec3_t vDuckedOfs;
		vec3_t vDuckedMove;
		vec3_t vDuckedSprintOfs;
		vec3_t vDuckedSprintRot;
		vec2_t vDuckedSprintBob;
		float fDuckedSprintCycleScale;
		vec3_t vSprintOfs;
		vec3_t vSprintRot;
		vec2_t vSprintBob;
		float fSprintCycleScale;
		vec3_t vLowReadyOfs;
		vec3_t vLowReadyRot;
		vec3_t vRideOfs;
		vec3_t vRideRot;
		vec3_t vDtpOfs;
		vec3_t vDtpRot;
		vec2_t vDtpBob;
		float fDtpCycleScale;
		vec3_t vMantleOfs;
		vec3_t vMantleRot;
		vec3_t vSlideOfs;
		vec3_t vSlideRot;
		vec3_t vDuckedRot;
		vec3_t vProneOfs;
		vec3_t vProneMove;
		vec3_t vProneRot;
		vec3_t vStrafeMove;
		vec3_t vStrafeRot;
		float fPosMoveRate;
		float fPosProneMoveRate;
		float fStandMoveMinSpeed;
		float fDuckedMoveMinSpeed;
		float fProneMoveMinSpeed;
		float fPosRotRate;
		float fPosProneRotRate;
		float fStandRotMinSpeed;
		float fDuckedRotMinSpeed;
		float fProneRotMinSpeed;
		void** worldModel;
		void* worldClipModel;
		void* rocketModel;
		void* mountedModel;
		void* additionalMeleeModel;
		void* fireTypeIcon;
		void* hudIcon;
		int hudIconRatio;
		void* indicatorIcon;
		int indicatorIconRatio;
		void* ammoCounterIcon;
		int ammoCounterIconRatio;
		int ammoCounterClip;
		int iStartAmmo;
		int iMaxAmmo;
		int shotCount;
		const char* szSharedAmmoCapName;
		int iSharedAmmoCapIndex;
		int iSharedAmmoCap;
		bool unlimitedAmmo;
		bool ammoCountClipRelative;
		int damage[6];
		float damageRange[6];
		int minPlayerDamage;
		float damageDuration;
		float damageInterval;
		int playerDamage;
		int iMeleeDamage;
		int iDamageType;
		unsigned __int16 explosionTag;
		int iFireDelay;
		int iMeleeDelay;
		int meleeChargeDelay;
		int iDetonateDelay;
		int iSpinUpTime;
		int iSpinDownTime;
		float spinRate;
		const char* spinLoopSound;
		const char* spinLoopSoundPlayer;
		const char* startSpinSound;
		const char* startSpinSoundPlayer;
		const char* stopSpinSound;
		const char* stopSpinSoundPlayer;
		bool applySpinPitch;
		int iFireTime;
		int iLastFireTime;
		int iRechamberTime;
		int iRechamberBoltTime;
		int iHoldFireTime;
		int iDetonateTime;
		int iMeleeTime;
		int iBurstDelayTime;
		int meleeChargeTime;
		int iReloadTimeRight;
		int iReloadTimeLeft;
		int reloadShowRocketTime;
		int iReloadEmptyTimeLeft;
		int iReloadAddTime;
		int iReloadEmptyAddTime;
		int iReloadQuickAddTime;
		int iReloadQuickEmptyAddTime;
		int iReloadStartTime;
		int iReloadStartAddTime;
		int iReloadEndTime;
		int iDropTime;
		int iRaiseTime;
		int iAltDropTime;
		int quickDropTime;
		int quickRaiseTime;
		int iFirstRaiseTime;
		int iEmptyRaiseTime;
		int iEmptyDropTime;
		int sprintInTime;
		int sprintLoopTime;
		int sprintOutTime;
		int lowReadyInTime;
		int lowReadyLoopTime;
		int lowReadyOutTime;
		int contFireInTime;
		int contFireLoopTime;
		int contFireOutTime;
		int dtpInTime;
		int dtpLoopTime;
		int dtpOutTime;
		int crawlInTime;
		int crawlForwardTime;
		int crawlBackTime;
		int crawlRightTime;
		int crawlLeftTime;
		int crawlOutFireTime;
		int crawlOutTime;
		int slideInTime;
		int deployTime;
		int breakdownTime;
		int iFlourishTime;
		int nightVisionWearTime;
		int nightVisionWearTimeFadeOutEnd;
		int nightVisionWearTimePowerUp;
		int nightVisionRemoveTime;
		int nightVisionRemoveTimePowerDown;
		int nightVisionRemoveTimeFadeInStart;
		int fuseTime;
		int aiFuseTime;
		int lockOnRadius;
		int lockOnSpeed;
		bool requireLockonToFire;
		bool noAdsWhenMagEmpty;
		bool avoidDropCleanup;
		unsigned int stackFire;
		float stackFireSpread;
		float stackFireAccuracyDecay;
		const char* stackSound;
		float autoAimRange;
		float aimAssistRange;
		bool mountableWeapon;
		float aimPadding;
		float enemyCrosshairRange;
		bool crosshairColorChange;
		float moveSpeedScale;
		float adsMoveSpeedScale;
		float sprintDurationScale;
		int overlayReticle;
		int overlayInterface;
		float overlayWidth;
		float overlayHeight;
		float fAdsBobFactor;
		float fAdsViewBobMult;
		bool bHoldBreathToSteady;
		float fHipSpreadStandMin;
		float fHipSpreadDuckedMin;
		float fHipSpreadProneMin;
		float hipSpreadStandMax;
		float hipSpreadDuckedMax;
		float hipSpreadProneMax;
		float fHipSpreadDecayRate;
		float fHipSpreadFireAdd;
		float fHipSpreadTurnAdd;
		float fHipSpreadMoveAdd;
		float fHipSpreadDuckedDecay;
		float fHipSpreadProneDecay;
		float fHipReticleSidePos;
		float fAdsIdleAmount;
		float fHipIdleAmount;
		float adsIdleSpeed;
		float hipIdleSpeed;
		float fIdleCrouchFactor;
		float fIdleProneFactor;
		float fGunMaxPitch;
		float fGunMaxYaw;
		float swayMaxAngle;
		float swayLerpSpeed;
		float swayPitchScale;
		float swayYawScale;
		float swayHorizScale;
		float swayVertScale;
		float swayShellShockScale;
		float adsSwayMaxAngle;
		float adsSwayLerpSpeed;
		float adsSwayPitchScale;
		float adsSwayYawScale;
		bool sharedAmmo;
		bool bRifleBullet;
		bool armorPiercing;
		bool bAirburstWeapon;
		bool bBoltAction;
		bool bUseAltTagFlash;
		bool bUseAntiLagRewind;
		bool bIsCarriedKillstreakWeapon;
		bool aimDownSight;
		bool bRechamberWhileAds;
		bool bReloadWhileAds;
		float adsViewErrorMin;
		float adsViewErrorMax;
		bool bCookOffHold;
		bool bClipOnly;
		bool bCanUseInVehicle;
		bool bNoDropsOrRaises;
		bool adsFireOnly;
		bool cancelAutoHolsterWhenEmpty;
		bool suppressAmmoReserveDisplay;
		bool laserSight;
		bool laserSightDuringNightvision;
		bool bHideThirdPerson;
		bool bHasBayonet;
		bool bDualWield;
		bool bExplodeOnGround;
		bool bThrowBack;
		bool bRetrievable;
		bool bDieOnRespawn;
		bool bNoThirdPersonDropsOrRaises;
		bool bContinuousFire;
		bool bNoPing;
		bool bForceBounce;
		bool bUseDroppedModelAsStowed;
		bool bNoQuickDropWhenEmpty;
		bool bKeepCrosshairWhenADS;
		bool bUseOnlyAltWeaoponHideTagsInAltMode;
		bool bAltWeaponAdsOnly;
		bool bAltWeaponDisableSwitching;
		void* killIcon;
		int killIconRatio;
		bool flipKillIcon;
		bool bNoPartialReload;
		bool bSegmentedReload;
		bool bNoADSAutoReload;
		int iReloadAmmoAdd;
		int iReloadStartAdd;
		const char* szSpawnedGrenadeWeaponName;
		const char* szDualWieldWeaponName;
		unsigned int dualWieldWeaponIndex;
		int iDropAmmoMin;
		int iDropAmmoMax;
		int iDropClipAmmoMin;
		int iDropClipAmmoMax;
		int iShotsBeforeRechamber;
		bool blocksProne;
		bool bShowIndicator;
		int isRollingGrenade;
		int useBallisticPrediction;
		int isValuable;
		int isTacticalInsertion;
		bool isReviveWeapon;
		bool bUseRigidBodyOnVehicle;
		int iExplosionRadius;
		int iExplosionRadiusMin;
		int iIndicatorRadius;
		int iExplosionInnerDamage;
		int iExplosionOuterDamage;
		float damageConeAngle;
		int iProjectileSpeed;
		int iProjectileSpeedUp;
		int iProjectileSpeedRelativeUp;
		int iProjectileSpeedForward;
		float fProjectileTakeParentVelocity;
		int iProjectileActivateDist;
		float projLifetime;
		float timeToAccelerate;
		float projectileCurvature;
		void* projectileModel;
		int projExplosion;
		const void* projExplosionEffect;
		bool projExplosionEffectForceNormalUp;
		const void* projExplosionEffect2;
		bool projExplosionEffect2ForceNormalUp;
		const void* projExplosionEffect3;
		bool projExplosionEffect3ForceNormalUp;
		const void* projExplosionEffect4;
		bool projExplosionEffect4ForceNormalUp;
		const void* projExplosionEffect5;
		bool projExplosionEffect5ForceNormalUp;
		const void* projDudEffect;
		const char* projExplosionSound;
		const char* projDudSound;
		const char* mortarShellSound;
		const char* tankShellSound;
		bool bProjImpactExplode;
		bool bProjSentientImpactExplode;
		bool bProjExplodeWhenStationary;
		bool bBulletImpactExplode;
		int stickiness;
		int rotateType;
		bool plantable;
		bool hasDetonator;
		bool timedDetonation;
		bool bNoCrumpleMissile;
		bool rotate;
		bool bKeepRolling;
		bool holdButtonToThrow;
		bool offhandHoldIsCancelable;
		bool freezeMovementWhenFiring;
		float lowAmmoWarningThreshold;
		bool bDisallowAtMatchStart;
		float meleeChargeRange;
		bool bUseAsMelee;
		bool isCameraSensor;
		bool isAcousticSensor;
		bool isLaserSensor;
		bool isHoldUseGrenade;
		float* parallelBounce;
		float* perpendicularBounce;
		const void* projTrailEffect;
		vec3_t vProjectileColor;
		int guidedMissileType;
		float maxSteeringAccel;
		int projIgnitionDelay;
		const void* projIgnitionEffect;
		const char* projIgnitionSound;
		float fAdsAimPitch;
		float fAdsCrosshairInFrac;
		float fAdsCrosshairOutFrac;
		int adsGunKickReducedKickBullets;
		float adsGunKickReducedKickPercent;
		float fAdsGunKickPitchMin;
		float fAdsGunKickPitchMax;
		float fAdsGunKickYawMin;
		float fAdsGunKickYawMax;
		float fAdsGunKickAccel;
		float fAdsGunKickSpeedMax;
		float fAdsGunKickSpeedDecay;
		float fAdsGunKickStaticDecay;
		float fAdsViewKickPitchMin;
		float fAdsViewKickPitchMax;
		float fAdsViewKickMinMagnitude;
		float fAdsViewKickYawMin;
		float fAdsViewKickYawMax;
		float fAdsRecoilReductionRate;
		float fAdsRecoilReductionLimit;
		float fAdsRecoilReturnRate;
		float fAdsViewScatterMin;
		float fAdsViewScatterMax;
		float fAdsSpread;
		int hipGunKickReducedKickBullets;
		float hipGunKickReducedKickPercent;
		float fHipGunKickPitchMin;
		float fHipGunKickPitchMax;
		float fHipGunKickYawMin;
		float fHipGunKickYawMax;
		float fHipGunKickAccel;
		float fHipGunKickSpeedMax;
		float fHipGunKickSpeedDecay;
		float fHipGunKickStaticDecay;
		float fHipViewKickPitchMin;
		float fHipViewKickPitchMax;
		float fHipViewKickMinMagnitude;
		float fHipViewKickYawMin;
		float fHipViewKickYawMax;
		float fHipViewScatterMin;
		float fHipViewScatterMax;
		float fAdsViewKickCenterDuckedScale;
		float fAdsViewKickCenterProneScale;
		float fAntiQuickScopeTime;
		float fAntiQuickScopeScale;
		float fAntiQuickScopeSpreadMultiplier;
		float fAntiQuickScopeSpreadMax;
		float fAntiQuickScopeSwayFactor;
		float fightDist;
		float maxDist;
		const char* accuracyGraphName[2];
		vec2_t* accuracyGraphKnots[2];
		vec2_t* originalAccuracyGraphKnots[2];
		int accuracyGraphKnotCount[2];
		int originalAccuracyGraphKnotCount[2];
		int iPositionReloadTransTime;
		float leftArc;
		float rightArc;
		float topArc;
		float bottomArc;
		float accuracy;
		float aiSpread;
		float playerSpread;
		float minTurnSpeed[2];
		float maxTurnSpeed[2];
		float pitchConvergenceTime;
		float yawConvergenceTime;
		float suppressTime;
		float maxRange;
		float fAnimHorRotateInc;
		float fPlayerPositionDist;
		const char* szUseHintString;
		const char* dropHintString;
		int iUseHintStringIndex;
		int dropHintStringIndex;
		float horizViewJitter;
		float vertViewJitter;
		float cameraShakeScale;
		int cameraShakeDuration;
		int cameraShakeRadius;
		float explosionCameraShakeScale;
		int explosionCameraShakeDuration;
		int explosionCameraShakeRadius;
		const char* szScript;
		float destabilizationRateTime;
		float destabilizationCurvatureMax;
		int destabilizeDistance;
		float* locationDamageMultipliers;
		const char* fireRumble;
		const char* meleeImpactRumble;
		const char* reloadRumble;
		const char* explosionRumble;
		void* tracerType;
		void* enemyTracerType;
		float adsDofStart;
		float adsDofEnd;
		float hipDofStart;
		float hipDofEnd;
		float scanSpeed;
		float scanAccel;
		int scanPauseTime;
		const char* flameTableFirstPerson;
		const char* flameTableThirdPerson;
		void* flameTableFirstPersonPtr;
		void* flameTableThirdPersonPtr;
		const void* tagFx_preparationEffect;
		const void* tagFlash_preparationEffect;
		bool doGibbing;
		float maxGibDistance;
		float altScopeADSTransInTime;
		float altScopeADSTransOutTime;
		int iIntroFireTime;
		int iIntroFireLength;
		const void* meleeSwipeEffect;
		const void* meleeImpactEffect;
		const void* meleeImpactNoBloodEffect;
		const char* throwBackType;
		void* weaponCamo;
		float customFloat0;
		float customFloat1;
		float customFloat2;
		int customBool0;
		int customBool1;
		int customBool2;
	};

	static_assert(sizeof(WeaponDef) == 2448);
}
#pragma once

namespace game
{
	enum scriptInstance_t
	{
		SCRIPTINSTANCE_SERVER,
		SCRIPTINSTANCE_CLIENT,
	};

	struct scr_entref_t
	{
		unsigned short entnum;
		unsigned short classnum;
		unsigned short client;
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

	struct gentity_s
	{
		int entity_num;
		char data[0x318];
	};
}
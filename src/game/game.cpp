#include <stdafx.hpp>

namespace game
{
	gamemode current = gamemode::none;

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

	void init()
	{
		current = reinterpret_cast<const char*>(0xC2F028) == "multiplayer"s
			? gamemode::multiplayer 
			: gamemode::zombies;
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

	int Cmd_Argc()
	{
		auto count = 0;

		for (auto i = 0; strcmp(game::Cmd_Argv(i), "") != 0; i++)
		{
			count++;
		}

		return count;
	}
}

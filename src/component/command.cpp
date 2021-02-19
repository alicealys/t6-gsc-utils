#include "stdafx.hpp"

namespace command
{
	std::unordered_map<std::string, std::function<void(params&)>> handlers;

	void main_handler()
	{
		params params = {};

		const auto command = utils::string::to_lower(params[0]);

		if (handlers.find(command) != handlers.end())
		{
			handlers[command](params);
		}
	}

	int params::size()
	{
		return game::Cmd_Argc();
	}

	const char* params::get(int index)
	{
		return game::Cmd_Argv(index);
	}

	std::string params::join(int index)
	{
		std::string result = {};

		for (int i = index; i < this->size(); i++)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}

		return result;
	}

	void add_raw(const char* name, void (*callback)())
	{
		game::Cmd_AddCommandInternal(name, callback, utils::memory::get_allocator()->allocate<game::cmd_function_t>());
	}

	void add(const char* name, std::function<void(params&)> callback)
	{
		const auto command = utils::string::to_lower(name);

		if (handlers.find(command) == handlers.end())
		{
			add_raw(name, main_handler);
		}

		handlers[command] = callback;
	}

	void init()
	{
		command::add("notifylevel", [](command::params& params)
		{
			if (params.size() < 2)
			{
				printf("Usage: notifylevel <name> [...]\n");
				return;
			}

			if (!(*game::levelEntityId))
			{
				printf("Level not loaded\n");
				return;
			}

			const auto argc = params.size();

			for (auto i = 2; i < argc; i++)
			{
				const auto arg = params[i];
				game::add(arg);
			}

			const auto name = game::SL_GetString(params[1], 0);
			game::Scr_NotifyId(game::SCRIPTINSTANCE_SERVER, 0, *game::levelEntityId, name, argc - 2);
		});

		command::add("notifynum", [](command::params& params)
		{
			if (params.size() < 3)
			{
				printf("Usage: notifynum <num> <name> [...]\n");
				return;
			}

			if (!(*game::levelEntityId))
			{
				printf("Level not loaded\n");
				return;
			}

			const auto client = atoi(params[1]);
			const auto sv_maxclients = game::Dvar_FindVar("sv_maxclients")->current.integer;

			if (client < 0 || client >= sv_maxclients)
			{
				return;
			}

			const auto argc = params.size();

			for (auto i = 3; i < argc; i++)
			{
				const auto arg = params[i];
				game::add(arg);
			}

			const auto name = game::SL_GetString(params[2], 0);
			game::Scr_NotifyNum(client, 0, name, argc - 3);
		});
	}
}
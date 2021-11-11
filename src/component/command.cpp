#include "stdinc.hpp"
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "command.hpp"
#include "gsc.hpp"

#include <utils/string.hpp>
#include <utils/memory.hpp>

namespace command
{
	std::unordered_map<std::string, std::function<void(params&)>> handlers;

	game::CmdArgs* get_cmd_args()
	{
		return reinterpret_cast<game::CmdArgs*>(game::Sys_GetValue(4));
	}

	void main_handler()
	{
		params params = {};

		const auto command = utils::string::to_lower(params[0]);

		if (handlers.find(command) != handlers.end())
		{
			handlers[command](params);
		}
	}

	params::params()
		: nesting_(get_cmd_args()->nesting)
	{
	}

	int params::size() const
	{
		const auto cmd_args = get_cmd_args();
		return cmd_args->argc[cmd_args->nesting];
	}

	const char* params::get(int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		const auto cmd_args = get_cmd_args();
		return cmd_args->argv[this->nesting_][index];
	}

	std::string params::join(int index) const
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

	std::vector<std::string> script_commands;
	utils::memory::allocator allocator;

	void add_script_command(const std::string& name, const std::function<void(const params&)>& callback)
	{
		script_commands.push_back(name);
		const auto _name = allocator.duplicate_string(name);
		add(_name, callback);
	}

	void clear_script_commands()
	{
		for (const auto& name : script_commands)
		{
			handlers.erase(name);
			game::Cmd_RemoveCommand(name.data());
		}

		allocator.clear();
		script_commands.clear();
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
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

				for (auto i = argc - 1; i > 1; i--)
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

				for (auto i = argc - 1; i > 2; i--)
				{
					const auto arg = params[i];
					game::add(arg);
				}

				const auto name = game::SL_GetString(params[2], 0);
				game::Scr_NotifyNum(client, 0, name, argc - 3);
			});

			gsc::function::add("executecommand", [](const gsc::function_args&) -> scripting::script_value
			{
				const auto cmd = game::get<const char*>(0);
				game::Cbuf_InsertText(0, cmd);
				return {};
			});

			gsc::function::add("addcommand", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto name = args[0].as<std::string>();
				const auto function = args[1].as<scripting::function>();
				command::add_script_command(name, [function](const command::params& params)
				{
					scripting::array array;
					for (auto i = 0; i < params.size(); i++)
					{
						array.push(params[i]);
					}

					function({ array.get_raw() });
				});

				return {};
			});
		}
	};
}

REGISTER_COMPONENT(command::component)
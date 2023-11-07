#include "stdinc.hpp"
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "command.hpp"
#include "gsc.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/memory.hpp>

namespace command
{
	std::unordered_map<std::string, std::function<void(params&)>> handlers;
	std::unordered_map<std::string, std::function<void(int, params_sv&)>> handlers_sv;

	std::vector<std::string> script_commands;
	std::vector<std::string> script_sv_commands;
	utils::memory::allocator allocator;

	utils::hook::detour client_command_hook;

	game::CmdArgs* get_cmd_args()
	{
		return static_cast<game::CmdArgs*>(game::Sys_GetValue(4));
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
		return cmd_args->argc[this->nesting_];
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

	params_sv::params_sv()
		: nesting_(game::sv_cmd_args->nesting)
	{
	}

	int params_sv::size() const
	{
		return game::sv_cmd_args->argc[this->nesting_];
	}

	const char* params_sv::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return game::sv_cmd_args->argv[this->nesting_][index];
	}

	std::string params_sv::join(const int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index)
			{
				result.append(" ");
			}

			result.append(this->get(i));
		}

		return result;
	}

	std::vector<std::string> params_sv::get_all() const
	{
		std::vector<std::string> params_;
		for (auto i = 0; i < this->size(); i++)
		{
			params_.emplace_back(this->get(i));
		}
		return params_;
	}

	void add_raw(const char* name, void (*callback)())
	{
		game::Cmd_AddCommandInternal(name, callback, utils::memory::get_allocator()->allocate<game::cmd_function_t>());
	}

	void add(const char* name, const std::function<void(params&)>& callback)
	{
		const auto command = utils::string::to_lower(name);

		if (handlers.find(command) == handlers.end())
		{
			add_raw(name, main_handler);
		}

		handlers[command] = callback;
	}

	void add_sv(const std::string& name, const std::function<void(int, const params_sv&)>& callback)
	{
		const auto command = utils::string::to_lower(name);
		if (handlers_sv.find(command) == handlers_sv.end())
		{
			handlers_sv[command] = callback;
		}
	}

	void add_script_command(const std::string& name, const std::function<void(const params&)>& callback)
	{
		script_commands.push_back(name);
		add(allocator.duplicate_string(name), callback);
	}

	void add_script_sv_command(const std::string& name, const std::function<void(int, const params_sv&)>& callback)
	{
		script_sv_commands.push_back(name);
		add_sv(name, callback);
	}

	void clear_script_commands()
	{
		for (const auto& name : script_commands)
		{
			handlers.erase(name);
			game::Cmd_RemoveCommand(name.data());
		}

		for (const auto& name : script_sv_commands)
		{
			handlers_sv.erase(name);
		}

		allocator.clear();
		script_commands.clear();
		script_sv_commands.clear();
	}

	void execute(std::string command, const bool sync)
	{
		command += "\n";

		if (sync)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, command.data());
		}
		else
		{
			game::Cbuf_AddText(0, command.data());
		}
	}

	void client_command_stub(const int client_num)
	{
		params_sv params = {};

		const auto command = utils::string::to_lower(params[0]);
		if (handlers_sv.find(command) != handlers_sv.end())
		{
			handlers_sv[command](client_num, params);
			return;
		}

		client_command_hook.invoke<void>(client_num);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scripting::on_shutdown(clear_script_commands);

			client_command_hook.create(SELECT(0x47E590, 0x4490C0), client_command_stub);

			add("notifylevel", [](const params& params)
			{
				if (params.size() < 2)
				{
					printf("Usage: notifylevel <name> [...]\n");
					return;
				}

				if (!*game::levelEntityId)
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

			add("notifynum", [](const params& params)
			{
				if (params.size() < 3)
				{
					printf("Usage: notifynum <num> <name> [...]\n");
					return;
				}

				if (!*game::levelEntityId)
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

				if (game::g_entities[client].client == nullptr)
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

			gsc::function::add_multiple([](const std::string& cmd)
			{
				execute(cmd);
			}, "executecommand", "command::execute");

			gsc::function::add_multiple([](const std::string& name, const scripting::function& function)
			{
				add_script_command(name, [function](const params& params)
				{
					scripting::array array;

					for (auto i = 0; i < params.size(); i++)
					{
						array.push(params[i]);
					}

					function({array.get_raw()});
				});
			}, "addcommand", "command::add");

			gsc::function::add_multiple([](const std::string& name, const scripting::function& function)
			{
				add_script_sv_command(name, [function](const int client_num, const params_sv& params)
				{
					const auto params_ = params.get_all();
					scheduler::once([=]()
					{
						const scripting::entity player = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, client_num, 0, 0);

						scripting::array array;

						for (auto i = 0; i < params.size(); i++)
						{
							array.push(params[i]);
						}

						function(player, {array});
					}, scheduler::pipeline::server);
				});
			}, "addclientcommand", "command::add_sv");

		}
	};
}

REGISTER_COMPONENT(command::component)
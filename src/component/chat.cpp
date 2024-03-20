#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "gsc.hpp"
#include "scripting.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/info_string.hpp>
#include <utils/string.hpp>

namespace chat
{
	namespace
	{
		struct chat_command_t
		{
			scripting::function callback;
			bool hide_message;
			bool enabled;
			bool sync;
		};

		struct chat_callback_t
		{
			scripting::function callback;
			bool sync;
		};

		using userinfo_map = std::unordered_map<std::string, std::string>;

		utils::hook::detour sv_get_user_info_hook;
		utils::hook::detour client_connect_hook;
		utils::hook::detour g_say_hook;

		std::unordered_map<int, userinfo_map> userinfo_overrides;
		std::unordered_map<std::string, chat_command_t> chat_commands;
		std::vector<chat_callback_t> chat_callbacks;

		void sv_get_user_info_stub(int index, char* buffer, int buffer_size)
		{
			sv_get_user_info_hook.invoke<void>(index, buffer, buffer_size);

			utils::info_string map(buffer);

			for (const auto& [key, val] : userinfo_overrides[index])
			{
				if (val.empty())
				{
					map.remove(key);
				}
				else
				{
					map.set(key, val);
				}
			}

			const auto userinfo = map.build();
			strncpy_s(buffer, 1024, userinfo.data(), _TRUNCATE);
		}

		const char* client_connect_stub(int client, unsigned int script_pers_id)
		{
			userinfo_overrides[client].clear();
			return client_connect_hook.invoke<const char*>(client, script_pers_id);
		}

		bool handle_chat_command(game::gentity_s* ent, const std::string& text)
		{
			const auto trimmed = utils::string::trim(text);
			const auto args = utils::string::split(trimmed, ' ');
			if (args.size() <= 0)
			{
				return false;
			}

			const auto name = utils::string::to_lower(args[0]);
			const auto iter = chat_commands.find(name);
			if (iter == chat_commands.end())
			{
				return false;
			}

			if (!iter->second.enabled)
			{
				return false;
			}

			if (iter->second.sync)
			{
				const auto entity_id = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, ent->number, 0, 0);
				const auto result = iter->second.callback(entity_id, {args});

				if (result.is<bool>())
				{
					return result.as<bool>();
				}
			}
			else
			{
				scheduler::once([=]
				{
					const auto entity_id = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, ent->number, 0, 0);
					iter->second.callback(entity_id, {args});
				}, scheduler::server);
			}

			return iter->second.hide_message;
		}

		bool handle_chat_callback(game::gentity_s* ent, const std::string& text, const int mode)
		{
			auto hidden = false;

			for (const auto& chat_callback : chat_callbacks)
			{
				if (chat_callback.sync)
				{
					const auto entity_id = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, ent->number, 0, 0);
					const auto result = chat_callback.callback(entity_id, {text, mode});

					if (result.is<bool>())
					{
						hidden = hidden || result.as<bool>();
					}
				}
				else
				{
					scheduler::once([=]
					{
						const auto entity_id = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, ent->number, 0, 0);
						chat_callback.callback(entity_id, {text, mode});
					}, scheduler::server);
				}
			}

			return hidden;
		}

		void g_say_stub(game::gentity_s* ent, game::gentity_s* target, int mode, const char* text)
		{
			auto hidden = false;
			hidden = hidden || handle_chat_command(ent, text);
			hidden = hidden || handle_chat_callback(ent, text, mode);

			if (!hidden)
			{
				g_say_hook.invoke<void>(ent, target, mode, text);	
			}
		}

		void client_clean_name(const char* in, char* out, int out_size)
		{
			utils::hook::invoke<void>(SELECT(0x424F00, 0x655010), out, in, out_size); // I_strncpyz
		}

		__declspec(naked) void client_clean_name_stub()
		{
			__asm
			{
				pushad

				push [esp + 0x20 + 0x4] // outSize
				push edx // out
				push ecx // in
				call client_clean_name
				add esp, 0xC

				popad
				ret
			}
		}

		game::scr_entref_t check_entity(const scripting::entity& entity)
		{
			const auto ent = entity.get_entity_reference();
			if (ent.classnum != 0)
			{
				throw std::runtime_error("invalid entity");
			}

			if (game::g_entities[ent.entnum].client == nullptr)
			{
				throw std::runtime_error("not a player entity");
			}

			return ent;
		}

		template <bool value>
		void enable_chat_command(const std::string& name)
		{
			const auto lower = utils::string::to_lower(name);
			const auto iter = chat_commands.find(lower);
			if (iter == chat_commands.end())
			{
				return;
			}

			iter->second.enabled = value;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			g_say_hook.create(SELECT(0x6A7A40, 0x493DF0), g_say_stub);
			sv_get_user_info_hook.create(SELECT(0x68BB90, 0x4C10F0), sv_get_user_info_stub);
			client_connect_hook.create(SELECT(0x5EF5A0, 0x41BE10), client_connect_stub);

			utils::hook::call(SELECT(0x4ED764, 0x427E84), client_clean_name_stub);
			utils::hook::call(SELECT(0x4ED79F, 0x427EBF), client_clean_name_stub);

			scripting::on_shutdown([]()
			{
				userinfo_overrides.clear();
				chat_callbacks.clear();
				chat_commands.clear();
			});

			gsc::method::add("rename", [](const scripting::entity& entity, const std::string& name)
			{
				const auto ent = check_entity(entity);
				userinfo_overrides[ent.entnum]["name"] = name;
				game::ClientUserInfoChanged(ent.entnum);
			});

			gsc::method::add("setname", [](const scripting::entity& entity, const std::string& name)
			{
				const auto ent = check_entity(entity);
				userinfo_overrides[ent.entnum]["name"] = name;
				game::ClientUserInfoChanged(ent.entnum);
			});

			gsc::method::add("resetname", [](const scripting::entity& entity)
			{
				const auto ent = check_entity(entity);
				userinfo_overrides[ent.entnum].erase("name");
				game::ClientUserInfoChanged(ent.entnum);
			});

			gsc::method::add("resetclantag", [](const scripting::entity& entity)
			{
				const auto ent = check_entity(entity);
				userinfo_overrides[ent.entnum].erase("clantag");
				userinfo_overrides[ent.entnum].erase("clanAbbrev");
				userinfo_overrides[ent.entnum].erase("clanAbbrevEV");
				game::ClientUserInfoChanged(ent.entnum);
			});

			gsc::method::add("setclantag", [](const scripting::entity& entity, const std::string& name)
			{
				const auto ent = check_entity(entity);
				userinfo_overrides[ent.entnum]["clantag"] = name;
				userinfo_overrides[ent.entnum]["clanAbbrev"] = name;
				userinfo_overrides[ent.entnum]["clanAbbrevEV"] = "1";
				game::ClientUserInfoChanged(ent.entnum);
			});

			gsc::method::add("tell", [](const scripting::entity& entity, const std::string& msg)
			{
				const auto ent = check_entity(entity);
				game::SV_GameSendServerCommand(ent.entnum, 0, utils::string::va("j \"%s\"", msg.data()));
			});

			gsc::function::add("say", [](const std::string& msg)
			{
				game::SV_GameSendServerCommand(-1, 0, utils::string::va("j \"%s\"", msg.data()));
			});

			gsc::function::add("sendservercommand", [](const int client, const std::string& cmd)
			{
				game::SV_GameSendServerCommand(client, 0, cmd.data());
			});

			gsc::method::add("sendservercommand", [](const scripting::entity& entity, const std::string& cmd)
			{
				const auto ent = check_entity(entity);
				game::SV_GameSendServerCommand(ent.entnum, 0, cmd.data());
			});

			gsc::function::add_multiple([](const scripting::function& callback, const scripting::variadic_args& va)
			{
				chat_callback_t chat_callback{};
				chat_callback.sync = false;
				chat_callback.callback = callback;

				if (va.size() >= 1)
				{
					chat_callback.sync = va[0].as<bool>();
				}

				chat_callbacks.emplace_back(chat_callback);
			}, "onplayersay", "chat::register_callback");

			gsc::function::add("chat::register_command", [](const scripting::function_argument& names_or_name, 
				const scripting::function& callback, const scripting::variadic_args& va)
			{
				chat_command_t command{};
				command.hide_message = true;
				command.sync = false;
				command.callback = callback;
				command.enabled = true;

				if (va.size() >= 1)
				{
					command.hide_message = va[0].as<bool>();
				}

				if (va.size() >= 2)
				{
					command.sync = va[1].as<bool>();
				}

				if (names_or_name.is<scripting::array>())
				{
					const std::vector<std::string> names = names_or_name;
					for (const auto& name : names)
					{
						const auto lower = utils::string::to_lower(name);
						chat_commands.insert(std::make_pair(lower, command));
					}
				}
				else
				{
					const auto name = names_or_name.as<std::string>();
					const auto lower = utils::string::to_lower(name);
					chat_commands.insert(std::make_pair(lower, command));
				}
			});
			
			gsc::function::add("chat::disable_command", enable_chat_command<false>);
			gsc::function::add("chat::enable_command", enable_chat_command<true>);
		}
	};
}

REGISTER_COMPONENT(chat::component)

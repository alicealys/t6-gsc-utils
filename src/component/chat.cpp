#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "gsc.hpp"
#include "scripting.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace chat
{
	namespace
	{
		using userinfo_map = std::unordered_map<std::string, std::string>;
		std::unordered_map<int, userinfo_map> userinfo_overrides;
		utils::hook::detour sv_get_user_info_hook;
		utils::hook::detour client_connect_hook;

		std::vector<scripting::function> say_callbacks;
		utils::hook::detour g_say_hook;

		userinfo_map userinfo_to_map(const std::string& userinfo)
		{
			userinfo_map map{};
			const auto args = utils::string::split(userinfo, '\\');

			if (args.size() > 0)
			{
				for (auto i = 1; i < args.size() - 1; i += 2)
				{
					map[args[i]] = args[i + 1];
				}
			}

			return map;
		}

		std::string map_to_userinfo(const userinfo_map& map)
		{
			std::string buffer{};

			for (const auto& value : map)
			{
				buffer.append("\\");
				buffer.append(value.first);
				buffer.append("\\");
				buffer.append(value.second);
			}

			return buffer;
		}

		void sv_get_user_info_stub(int index, char* buffer, int bufferSize)
		{
			sv_get_user_info_hook.invoke<void>(index, buffer, bufferSize);
			auto map = userinfo_to_map(buffer);

			for (const auto& values : userinfo_overrides[index])
			{
				if (map.find(values.first) == map.end())
				{
					continue;
				}

				if (values.second.empty())
				{
					map.erase(values.first);
				}
				else
				{
					map[values.first] = values.second;
				}
			}

			const auto userinfo = map_to_userinfo(map);
			strcpy_s(buffer, 1024, userinfo.data());
		}

		const char* client_connect_stub(int client, unsigned int scriptPersId)
		{
			userinfo_overrides[client].clear();
			return client_connect_hook.invoke<const char*>(client, scriptPersId);
		}

		void g_say_stub(game::gentity_s* ent, game::gentity_s* target, int mode, const char* chatText)
		{
			auto hidden = false;

			for (const auto& callback : say_callbacks)
			{
				const auto entity_id = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, ent->entity_num, 0, 0);
				const auto result = callback(entity_id, {chatText, mode});

				if (result.is<int>())
				{
					hidden = result.as<int>() == 0;
				}
			}

			if (!hidden)
			{
				g_say_hook.invoke<void>(ent, target, mode, chatText);
			}
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

			scripting::on_shutdown([]()
			{
				userinfo_overrides.clear();
				say_callbacks.clear();
			});

			gsc::method::add("rename", [](const scripting::entity& entity, const gsc::function_args& args) -> scripting::script_value
			{
				const auto ent = entity.get_entity_reference();
				const auto name = args[0].as<std::string>();

				if (ent.classnum != 0 || ent.entnum > 17)
				{
					throw std::runtime_error("Invalid entity");
				}

				userinfo_overrides[ent.entnum]["name"] = name;
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("setname", [](const scripting::entity& entity, const gsc::function_args& args) -> scripting::script_value
			{
				const auto ent = entity.get_entity_reference();
				const auto name = args[0].as<std::string>();

				if (ent.classnum != 0 || ent.entnum > 17)
				{
					throw std::runtime_error("Invalid entity");
				}

				userinfo_overrides[ent.entnum]["name"] = name;
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("resetname", [](const scripting::entity& entity, const gsc::function_args&) -> scripting::script_value
			{
				const auto ent = entity.get_entity_reference();
				if (ent.classnum != 0 || ent.entnum > 17)
				{
					throw std::runtime_error("Invalid entity");
				}

				userinfo_overrides[ent.entnum].erase("name");
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("resetclantag", [](const scripting::entity& entity, const gsc::function_args&) -> scripting::script_value
			{
				const auto ent = entity.get_entity_reference();

				if (ent.classnum != 0 || ent.entnum > 17)
				{
					throw std::runtime_error("Invalid entity");
				}

				userinfo_overrides[ent.entnum].erase("clantag");
				userinfo_overrides[ent.entnum].erase("clanAbbrev");
				userinfo_overrides[ent.entnum].erase("clanAbbrevEV");
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("setclantag", [](const scripting::entity& entity, const gsc::function_args& args) -> scripting::script_value
			{
				const auto ent = entity.get_entity_reference();
				const auto name = args[0].as<std::string>();

				if (ent.classnum != 0 || ent.entnum > 17)
				{
					throw std::runtime_error("Invalid entity");
				}

				userinfo_overrides[ent.entnum]["clantag"] = name;
				userinfo_overrides[ent.entnum]["clanAbbrev"] = name;
				userinfo_overrides[ent.entnum]["clanAbbrevEV"] = "1";
				game::ClientUserInfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("tell", [](const scripting::entity& entity, const gsc::function_args& args) -> scripting::script_value
			{
				const auto ent = entity.get_entity_reference();
				if (ent.classnum != 0)
				{
					return {};
				}

				const auto client = ent.entnum;
				const auto msg = args[0].as<std::string>();
				game::SV_GameSendServerCommand(client, 0, utils::string::va("j \"%s\"", msg.data()));

				return {};
			});

			gsc::function::add("say", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto msg = args[0].as<std::string>();
				game::SV_GameSendServerCommand(-1, 0, utils::string::va("j \"%s\"", msg.data()));
				return {};
			});

			gsc::function::add("cmdexecute", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto cmd = args[0].as<std::string>();
				game::Cbuf_InsertText(0, cmd.data());
				return {};
			});

			gsc::function::add("sendservercommand", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto client = args[0].as<int>();
				const auto cmd = args[1].as<std::string>();
				game::SV_GameSendServerCommand(client, 0, cmd.data());
				return {};
			});

			gsc::method::add("sendservercommand", [](const scripting::entity& entity, const gsc::function_args& args) -> scripting::script_value
			{
				const auto client = entity.get_entity_reference().entnum;
				const auto cmd = args[0].as<std::string>();
				game::SV_GameSendServerCommand(client, 0, cmd.data());
				return {};
			});

			gsc::function::add("onplayersay", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto function = args[0].as<scripting::function>();
				say_callbacks.push_back(function);
				return {};
			});
		}
	};
}

REGISTER_COMPONENT(chat::component)

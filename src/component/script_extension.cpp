#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

#include <utils/string.hpp>

namespace script_extension
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			add_gsc_funcs();
		}

	private:
		static void add_gsc_funcs()
		{
			gsc::method::add("noclip", [](const scripting::entity& entity, [[maybe_unused]] const gsc::function_args& args) -> scripting::script_value
			{
				const auto entref = entity.get_entity_reference();

				const auto* ent = game::GetPlayerEntity(entref);

				if (ent->client->flags & game::gclientFlag::NOCLIP)
				{
					ent->client->flags &= ~game::gclientFlag::NOCLIP;
				}
				else
				{
					ent->client->flags |= game::gclientFlag::NOCLIP;
				}

				game::SV_GameSendServerCommand(entref.entnum, 0, utils::string::va("%c \"%s\"", 0x4F,
					(ent->client->flags & game::gclientFlag::NOCLIP) ? "GAME_NOCLIPON" : "GAME_NOCLIPOFF"));

				return {};
			});

			gsc::method::add("ufo", [](const scripting::entity& entity, [[maybe_unused]] const gsc::function_args& args) -> scripting::script_value
			{
				const auto entref = entity.get_entity_reference();

				const auto* ent = game::GetPlayerEntity(entref);

				if (ent->client->flags & game::gclientFlag::UFO)
				{
					ent->client->flags &= ~game::gclientFlag::UFO;
				}
				else
				{
					ent->client->flags |= game::gclientFlag::UFO;
				}

				game::SV_GameSendServerCommand(entref.entnum, 0, utils::string::va("%c \"%s\"", 0x4F,
					(ent->client->flags & game::gclientFlag::UFO) ? "GAME_UFOON" : "GAME_UFOOFF"));

				return {};
			});

			gsc::method::add("god", [](const scripting::entity& entity, [[maybe_unused]] const gsc::function_args& args) -> scripting::script_value
			{
				const auto entref = entity.get_entity_reference();

				auto* ent = game::GetEntity(entref);

				if (ent->flags & game::entityFlag::FL_GODMODE)
				{
					ent->flags &= ~game::entityFlag::FL_GODMODE;
				}
				else
				{
					ent->flags |= game::entityFlag::FL_GODMODE;
				}

				game::SV_GameSendServerCommand(entref.entnum, 0, utils::string::va("%c \"%s\"", 0x4F,
					(ent->flags & game::entityFlag::FL_GODMODE) ? "GAME_GODMODE_ON" : "GAME_GODMODE_OFF"));

				return {};
			});

			gsc::method::add("demigod", [](const scripting::entity& entity, [[maybe_unused]] const gsc::function_args& args) -> scripting::script_value
			{
				const auto entref = entity.get_entity_reference();

				auto* ent = game::GetEntity(entref);

				if (ent->flags & game::entityFlag::FL_DEMI_GODMODE)
				{
					ent->flags &= ~game::entityFlag::FL_DEMI_GODMODE;
				}
				else
				{
					ent->flags |= game::entityFlag::FL_DEMI_GODMODE;
				}

				game::SV_GameSendServerCommand(entref.entnum, 0, utils::string::va("%c \"%s\"", 0x4F,
					(ent->flags & game::entityFlag::FL_DEMI_GODMODE) ? "GAME_DEMI_GODMODE_ON" : "GAME_DEMI_GODMODE_OFF"));

				return {};
			});

			gsc::method::add("notarget", [](const scripting::entity& entity, [[maybe_unused]] const gsc::function_args& args) -> scripting::script_value
			{
				const auto entref = entity.get_entity_reference();

				auto* ent = game::GetEntity(entref);

				if (ent->flags & game::entityFlag::FL_NOTARGET)
				{
					ent->flags &= ~game::entityFlag::FL_NOTARGET;
				}
				else
				{
					ent->flags |= game::entityFlag::FL_NOTARGET;
				}

				game::SV_GameSendServerCommand(entref.entnum, 0, utils::string::va("%c \"%s\"", 0x4F,
					(ent->flags & game::entityFlag::FL_NOTARGET) ? "GAME_NOTARGETON" : "GAME_NOTARGETOFF"));

				return {};
			});
		}
	};
}

REGISTER_COMPONENT(script_extension::component)

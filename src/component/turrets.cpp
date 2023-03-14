#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "gsc.hpp"
#include "game/structs.hpp"
#include "game/game.hpp"

#include <utils/io.hpp>
#include <utils/hook.hpp>

#define TURRET_DUMP_FIELD(__field__) \
	data[#__field__] = turret->__field__; \

#define TURRET_ADD_FIELD_INTERNAL(__name__, __getter__, __setter__) \
	{utils::string::to_lower(#__name__), turret_field{offsetof(game::TurretInfo, __name__), __getter__, __setter__}} \

#define TURRET_ADD_FIELD_GETTER(__name__, __getter__) \
	TURRET_ADD_FIELD_INTERNAL(__name__, __getter__, get_default_setter(dummy_info.__name__, #__name__, offsetof(game::TurretInfo, __name__))) \

#define TURRET_ADD_FIELD_SETTER(__name__, __setter__) \
	TURRET_ADD_FIELD_INTERNAL(__name__, get_default_getter(dummy_info.__name__, offsetof(game::TurretInfo, __name__)), __setter__) \

#define TURRET_ADD_FIELD(__name__) \
	TURRET_ADD_FIELD_INTERNAL(__name__, get_default_getter(dummy_info.__name__, offsetof(game::TurretInfo, __name__)), get_default_setter(dummy_info.__name__, #__name__, offsetof(game::TurretInfo, __name__))) \

namespace turret
{
	namespace
	{
		game::TurretInfo dummy_info{};

		using getter_t = std::function<scripting::script_value(game::TurretInfo* info)>;
		using setter_t = std::function<void(game::TurretInfo* info, const scripting::value_wrap& value)>;

		struct turret_field
		{
			std::size_t offset;
			getter_t getter;
			setter_t setter;
		};

		template <typename T>
		getter_t get_default_getter([[maybe_unused]] T, const size_t offset)
		{
			return [=](game::TurretInfo* info)
			{
				return *reinterpret_cast<T*>(reinterpret_cast<size_t>(info) + offset);
			};
		}

		template <typename T> 
		std::enable_if<std::is_same<T, float*>::value, setter_t>::type
		get_default_setter([[maybe_unused]] T, const std::string& name, const size_t offset)
		{
			return [=](game::TurretInfo* info, const scripting::value_wrap& value)
			{
				if (!value.is<T>())
				{
					const auto type_name = value.type_name();
					throw std::runtime_error(utils::string::va("type '%s' is not valid for field '%s'", type_name.data(), name.data()));
				}

				const auto vec = value.as<scripting::vector>();
				const auto val = reinterpret_cast<float*>(reinterpret_cast<size_t>(info) + offset);
				val[0] = vec.get_x();
				val[1] = vec.get_y();
				val[2] = vec.get_z();
			};
		}

		template <typename T> 
		std::enable_if<!std::is_same<T, float*>::value, setter_t>::type
		get_default_setter([[maybe_unused]] T, const std::string& name, const size_t offset)
		{
			return [=](game::TurretInfo* info, const scripting::value_wrap& value)
			{
				if (!value.is<T>())
				{
					const auto type_name = value.type_name();
					throw std::runtime_error(utils::string::va("type '%s' is not valid for field '%s'", type_name.data(), name.data()));
				}

				*reinterpret_cast<T*>(reinterpret_cast<size_t>(info) + offset) = value.as<T>();
			};
		}

		scripting::script_value turret_get_team(game::TurretInfo* info)
		{
			return utils::hook::invoke<const char*>(0x4D7A30, info->eTeam);
		}

		void turret_set_team(game::TurretInfo* info, const scripting::value_wrap& value)
		{
			const auto str = value.as<std::string>();
			const auto team_names = reinterpret_cast<const char**>(0xD39970);
			for (auto i = 0; i < game::TEAM_NUM_TEAMS; i++)
			{
				if (team_names[i] == str)
				{
					info->eTeam = static_cast<game::team_t>(i);
					return;
				}
			}

			throw std::runtime_error(utils::string::va("invalid team '%s'", str.data()));
		}

		std::unordered_map<std::string, turret_field> turret_fields =
		{
			TURRET_ADD_FIELD(inuse),
			TURRET_ADD_FIELD(state),
			TURRET_ADD_FIELD(flags),
			TURRET_ADD_FIELD(fireTime),
			TURRET_ADD_FIELD(manualTarget.infoIndex),
			TURRET_ADD_FIELD(manualTarget.number),
			TURRET_ADD_FIELD(target.infoIndex),
			TURRET_ADD_FIELD(target.number),
			TURRET_ADD_FIELD(targetPos),
			TURRET_ADD_FIELD(targetTime),
			TURRET_ADD_FIELD(missOffsetNormalized),
			TURRET_ADD_FIELD(arcmin[0]),
			TURRET_ADD_FIELD(arcmin[1]),
			TURRET_ADD_FIELD(arcmax[0]),
			TURRET_ADD_FIELD(arcmax[1]),
			TURRET_ADD_FIELD(initialYawmin),
			TURRET_ADD_FIELD(initialYawmax),
			TURRET_ADD_FIELD(forwardAngleDot),
			TURRET_ADD_FIELD(dropPitch),
			TURRET_ADD_FIELD(scanningPitch),
			TURRET_ADD_FIELD(convergenceTime[0]),
			TURRET_ADD_FIELD(convergenceTime[1]),
			TURRET_ADD_FIELD(suppressTime),
			TURRET_ADD_FIELD(maxRangeSquared),
			TURRET_ADD_FIELD(detachSentient.infoIndex),
			TURRET_ADD_FIELD(detachSentient.number),
			TURRET_ADD_FIELD(stance),
			TURRET_ADD_FIELD(prevStance),
			TURRET_ADD_FIELD(fireSndDelay),
			TURRET_ADD_FIELD(accuracy),
			TURRET_ADD_FIELD(userOrigin),
			TURRET_ADD_FIELD(prevSentTarget),
			TURRET_ADD_FIELD(aiSpread),
			TURRET_ADD_FIELD(playerSpread),
			TURRET_ADD_FIELD_INTERNAL(eTeam, turret_get_team, turret_set_team),
			TURRET_ADD_FIELD(heatVal),
			TURRET_ADD_FIELD(overheating),
			TURRET_ADD_FIELD(fireBarrel),
			TURRET_ADD_FIELD(scanSpeed),
			TURRET_ADD_FIELD(scanDecelYaw),
			TURRET_ADD_FIELD(scanPauseTime),
			TURRET_ADD_FIELD(originError),
			TURRET_ADD_FIELD(anglesError),
			TURRET_ADD_FIELD(pitchCap),
			TURRET_ADD_FIELD(triggerDown),
			TURRET_ADD_FIELD(fireSnd),
			TURRET_ADD_FIELD(fireSndPlayer),
			TURRET_ADD_FIELD(startFireSnd),
			TURRET_ADD_FIELD(startFireSndPlayer),
			TURRET_ADD_FIELD(loopFireEnd),
			TURRET_ADD_FIELD(loopFireEndPlayer),
			TURRET_ADD_FIELD(rotateLoopSnd),
			TURRET_ADD_FIELD(rotateLoopSndPlayer),
			TURRET_ADD_FIELD(rotateStopSnd),
			TURRET_ADD_FIELD(rotateStopSndPlayer),
			TURRET_ADD_FIELD(sndIsFiring),
			TURRET_ADD_FIELD(targetOffset),
			TURRET_ADD_FIELD(onTargetAngle),
			TURRET_ADD_FIELD(previousAngles),
			TURRET_ADD_FIELD(previousAngles),
		};

		utils::hook::detour bg_get_weapon_def_hook;
		game::WeaponDef* bg_get_weapon_def_stub(const unsigned int weapon)
		{
			auto asset = bg_get_weapon_def_hook.invoke<game::WeaponDef*>(weapon);

			if (asset->maxTurnSpeed[0] == 0.f && asset->maxTurnSpeed[1] == 0.f)
			{
				asset->maxTurnSpeed[0] = 360.f;
				asset->maxTurnSpeed[1] = 360.f;
				asset->minTurnSpeed[0] = 1.f;
				asset->minTurnSpeed[1] = 1.f;
			}

			return asset;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (!game::environment::t6zm())
			{
				return;
			}

			gsc::method::add("setturretfield", [](const scripting::entity& entity, const gsc::function_args& args)
				-> scripting::script_value
			{
				const auto ent = entity.get_entity_reference();
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				const auto turret = game::g_entities[ent.entnum].pTurretInfo;
				if (turret == nullptr)
				{
					throw std::runtime_error("Not a valid turret entity");
				}
				const auto field_name = utils::string::to_lower(args[0].as<std::string>());
				const auto itr = turret_fields.find(field_name);

				if (itr == turret_fields.end())
				{
					throw std::runtime_error(utils::string::va("Invalid field %s", field_name.data()));
				}

				itr->second.setter(turret, scripting::value_wrap(args[1], 1));
				return {};
			});

			gsc::method::add("getturretfield", [](const scripting::entity& entity, const gsc::function_args& args)->scripting::script_value
			{
				const auto ent = entity.get_entity_reference();
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				const auto turret = game::g_entities[ent.entnum].pTurretInfo;
				if (turret == nullptr)
				{
					throw std::runtime_error("Not a valid turret entity");
				}

				const auto field_name = utils::string::to_lower(args[0].as<std::string>());
				const auto itr = turret_fields.find(field_name);

				if (itr == turret_fields.end())
				{
					throw std::runtime_error(utils::string::va("Invalid field %s", field_name.data()));
				}

				return itr->second.getter(turret);
			});

			gsc::function::add("dumpturret", [](const gsc::function_args& args) 
				-> scripting::script_value
			{
				const auto ent = args[0].as<scripting::entity>().get_entity_reference();
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				const auto turret = game::g_entities[ent.entnum].pTurretInfo;
				if (turret == nullptr)
				{
					throw std::runtime_error("Not a valid turret entity");
				}

				nlohmann::ordered_json data;

				TURRET_DUMP_FIELD(inuse);
				TURRET_DUMP_FIELD(state);
				TURRET_DUMP_FIELD(flags);
				TURRET_DUMP_FIELD(fireTime);
				data["manualTarget"]["infoIndex"] = turret->manualTarget.infoIndex;
				data["manualTarget"]["number"] = turret->manualTarget.number;
				data["target"]["infoIndex"] = turret->target.infoIndex;
				data["target"]["number"] = turret->target.number;
				TURRET_DUMP_FIELD(targetPos);
				TURRET_DUMP_FIELD(targetTime);
				TURRET_DUMP_FIELD(missOffsetNormalized);
				TURRET_DUMP_FIELD(arcmin);
				TURRET_DUMP_FIELD(arcmax);
				TURRET_DUMP_FIELD(initialYawmin);
				TURRET_DUMP_FIELD(initialYawmax);
				TURRET_DUMP_FIELD(forwardAngleDot);
				TURRET_DUMP_FIELD(dropPitch);
				TURRET_DUMP_FIELD(scanningPitch);
				TURRET_DUMP_FIELD(convergenceTime);
				TURRET_DUMP_FIELD(suppressTime);
				TURRET_DUMP_FIELD(maxRangeSquared);
				data["detachSentient"]["infoIndex"] = turret->detachSentient.infoIndex;
				data["detachSentient"]["number"] = turret->detachSentient.number;
				TURRET_DUMP_FIELD(stance);
				TURRET_DUMP_FIELD(prevStance);
				TURRET_DUMP_FIELD(fireSndDelay);
				TURRET_DUMP_FIELD(accuracy);
				TURRET_DUMP_FIELD(userOrigin);
				TURRET_DUMP_FIELD(prevSentTarget);
				TURRET_DUMP_FIELD(aiSpread);
				TURRET_DUMP_FIELD(playerSpread);
				TURRET_DUMP_FIELD(eTeam);
				TURRET_DUMP_FIELD(heatVal);
				TURRET_DUMP_FIELD(overheating);
				TURRET_DUMP_FIELD(fireBarrel);
				TURRET_DUMP_FIELD(scanSpeed);
				TURRET_DUMP_FIELD(scanDecelYaw);
				TURRET_DUMP_FIELD(scanPauseTime);
				TURRET_DUMP_FIELD(originError);
				TURRET_DUMP_FIELD(anglesError);
				TURRET_DUMP_FIELD(pitchCap);
				TURRET_DUMP_FIELD(triggerDown);
				TURRET_DUMP_FIELD(fireSnd);
				TURRET_DUMP_FIELD(fireSndPlayer);
				TURRET_DUMP_FIELD(startFireSnd);
				TURRET_DUMP_FIELD(startFireSndPlayer);
				TURRET_DUMP_FIELD(loopFireEnd);
				TURRET_DUMP_FIELD(loopFireEndPlayer);
				TURRET_DUMP_FIELD(rotateLoopSnd);
				TURRET_DUMP_FIELD(rotateLoopSndPlayer);
				TURRET_DUMP_FIELD(rotateStopSnd);
				TURRET_DUMP_FIELD(rotateStopSndPlayer);
				TURRET_DUMP_FIELD(sndIsFiring);
				TURRET_DUMP_FIELD(targetOffset);
				TURRET_DUMP_FIELD(onTargetAngle);
				TURRET_DUMP_FIELD(turretRotateState);
				TURRET_DUMP_FIELD(previousAngles);

				const std::string buffer = data.dump(4);
				const auto name = args[1].as<std::string>();
				const std::string turret_entity_path = utils::string::va("entity_dumps/turrets/%s.json", name.data());

				return utils::io::write_file(turret_entity_path, buffer, false);
			});

			// Check for whether a weapon can be spawned as a turret
			utils::hook::nop(0x60AF4E, 5);

			// Don't use weapon def value for turrets turnspeed
			bg_get_weapon_def_hook.create(0x5906B0, bg_get_weapon_def_stub);
		}
	};
}

REGISTER_COMPONENT(turret::component)

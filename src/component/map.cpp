#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"
#include "json.hpp"
#include "debug.hpp"

#include "scheduler.hpp"
#include "scripting.hpp"
#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace gsc_map
{
    namespace
    {
        using gsc_map_index_type = std::variant<std::monostate, int, std::string>;
        using gsc_map_value_type = scripting::script_value;
        using gsc_map_type = std::unordered_map<gsc_map_index_type, gsc_map_value_type>;

        std::unordered_map<unsigned int, gsc_map_type> maps;

        utils::hook::detour remove_ref_to_object_hook;
        utils::hook::detour scr_eval_size_value_hook;
        utils::hook::detour scr_first_array_key_hook;
        utils::hook::detour scr_next_array_key_hook;
        utils::hook::detour scr_eval_array_hook;

        gsc_map_value_type variant_to_script_value(const gsc_map_index_type& index)
        {
            if (index.index() == 1)
            {
                return std::get<int>(index);
            }
            
            if (index.index() == 2)
            {

                return std::get<std::string>(index);
            }

            return {};
        }

        gsc_map_index_type script_value_to_variant(const gsc_map_value_type& value)
        {
            if (value.is<int>())
            {
                return {value.as<int>()};
            }

            if (value.is<std::string>())
            {
                return {value.as<std::string>()};
            }

            return {};
        }

        unsigned int remove_ref_to_object_stub(int inst, unsigned int id)
        {
            const auto ref_count = game::scr_VarGlob->objectVariableValue[id].u.f.prev;
            if (maps.find(id) != maps.end())
            {
                printf("destructor called %i\n", ref_count);
                if (ref_count == 0)
                {
                    maps.erase(id);
                }
            }

            return remove_ref_to_object_hook.invoke<unsigned int>(inst, id);
        }

        void scr_eval_size_value_stub(int inst, game::VariableValue* value)
        {
            const auto id = value->u.uintValue;
            if (value->type != game::SCRIPT_OBJECT || maps.find(id) == maps.end())
            {
                return scr_eval_size_value_hook.invoke<void>(inst, value);
            }

            value->type = game::SCRIPT_INTEGER;
            value->u.uintValue = static_cast<int>(maps[id].size());

            game::RemoveRefToObject(game::SCRIPTINSTANCE_SERVER, id);
        }

        game::VariableValue* scr_first_array_key_stub(game::VariableValue* result, int inst, unsigned int id)
        {
            if (maps.find(id) == maps.end())
            {
                return scr_first_array_key_hook.invoke<game::VariableValue*>(result, inst, id);
            }

            const auto map = &maps[id];
            const auto key = variant_to_script_value(map->begin()->first);
            *result = key.get_raw();

            return result;
        }

        game::VariableValue* scr_next_array_key_stub(game::VariableValue* result, int inst, unsigned int id, game::VariableValue* key)
        {
            if (maps.find(id) == maps.end())
            {
                return scr_next_array_key_hook.invoke<game::VariableValue*>(result, inst, id, key);
            }

            result->type = 0;
            result->u.uintValue = 0;

            const auto key_value = script_value_to_variant(*key);
            if (key_value.index() == 0)
            {
                return result;
            }

            const auto map = &maps[id];
            if (map->find(key_value) == map->end() || map->size() < 2)
            {
                return result;
            }

            for (auto i = map->begin(); std::next(i) != map->end(); ++i)
            {
                if (i->first != key_value)
                {
                    continue;
                }

                const auto value = variant_to_script_value(std::next(i)->first);
                *result = value.get_raw();
                return result;
            }

            return result;
        }

        void scr_eval_array_stub(int inst, game::VariableValue* value, game::VariableValue* index)
        {
            return scr_eval_array_hook.invoke<void>(inst, value, index);
        }

        gsc_map_type* get_map_from_ref(const scripting::array& map_ref)
        {
            const auto id = map_ref.get_entity_id();
            if (maps.find(id) == maps.end())
            {
                throw std::runtime_error("object is not a map");
            }

            return &maps[id];
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            remove_ref_to_object_hook.create(game::RemoveRefToObject.get(), remove_ref_to_object_stub);
            scr_eval_size_value_hook.create(SELECT(0x0, 0x6D4430), scr_eval_size_value_stub);
            scr_first_array_key_hook.create(SELECT(0, 0x60C170), scr_first_array_key_stub);
            scr_next_array_key_hook.create(SELECT(0, 0x6F8A10), scr_next_array_key_stub);
            scr_eval_array_hook.create(SELECT(0, 0x525A70), scr_eval_array_stub);

            gsc::function::add("unordered_map", [](const gsc::function_args&)
            {
                const scripting::array map_ref;
                const auto id = map_ref.get_entity_id();
                maps[id] = {};

                return map_ref;
            });

            gsc::function::add("unordered_map_set", [](const gsc::function_args& args) -> scripting::script_value
            {
                const auto map_ref = args[0].as<scripting::array>();
                const auto key = script_value_to_variant(args[1]);

                auto map = get_map_from_ref(map_ref);
                map->insert({key, args[2]});

                return {};
            });

            gsc::function::add("unordered_map_erase", [](const gsc::function_args& args) -> scripting::script_value
            {
                const auto map_ref = args[0].as<scripting::array>();
                const auto key = script_value_to_variant(args[1]);

                auto map = get_map_from_ref(map_ref);
                if (map->find(key) == map->end())
                {
                    return {};
                }

                map->erase(key);
                return {};
            });

            gsc::function::add("unordered_map_clear", [](const gsc::function_args& args) -> scripting::script_value
            {
                const auto map_ref = args[0].as<scripting::array>();

                auto map = get_map_from_ref(map_ref);
                map->clear();

                return {};
            });

            gsc::function::add("unordered_map_get", [](const gsc::function_args& args) -> scripting::script_value
            {
                const auto map_ref = args[0].as<scripting::array>();
                const auto key = script_value_to_variant(args[1]);

                auto map = get_map_from_ref(map_ref);
                if (map->find(key) == map->end())
                {
                    return {};
                }

                return map->at(key);
            });

            gsc::function::add("unordered_map_first", [](const gsc::function_args& args) -> scripting::script_value
            {
                const auto map_ref = args[0].as<scripting::array>();

                auto map = get_map_from_ref(map_ref);
                if (map->size() == 0)
                {
                    return {};
                }

                return variant_to_script_value(map->begin()->first);
            });

            gsc::function::add("unordered_map_next", [](const gsc::function_args& args) -> scripting::script_value
            {
                const auto map_ref = args[0].as<scripting::array>();
                const auto key = script_value_to_variant(args[1]);

                auto map = get_map_from_ref(map_ref);

                if (map->find(key) == map->end() || map->size() < 2)
                {
                    return {};
                }

                for (auto i = map->begin(); std::next(i) != map->end(); ++i)
                {
                    if (i->first == key)
                    {
                        return variant_to_script_value(std::next(i)->first);
                    }
                }
            });
        }
    };
}

REGISTER_COMPONENT(gsc_map::component)
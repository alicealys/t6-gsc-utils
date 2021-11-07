#pragma once
#include "game/game.hpp"

namespace scripting::safe_execution
{
	using script_function = void(*)(game::scr_entref_t);

	bool call(script_function function, const game::scr_entref_t& entref);

	bool set_entity_field(const game::scr_entref_t& entref, int offset);
	bool get_entity_field(const game::scr_entref_t& entref, int offset, game::VariableValue* value);
}

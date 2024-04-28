#pragma once
#include "../plugin.hpp"

class component_interface
{
public:
	virtual ~component_interface()
	{
	}

	virtual void on_startup([[maybe_unused]] plugin::plugin* plugin)
	{
	}

	virtual void on_dvar_init([[maybe_unused]] plugin::plugin* plugin)
	{
	}

	virtual void on_after_dvar_init([[maybe_unused]] plugin::plugin* plugin)
	{
	}

	virtual void on_game_init([[maybe_unused]] plugin::plugin* plugin)
	{
	}

	virtual void on_shutdown([[maybe_unused]] plugin::plugin* plugin)
	{
	}

	virtual bool is_supported()
	{
		return true;
	}
};

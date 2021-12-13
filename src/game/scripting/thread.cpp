#include <stdinc.hpp>
#include "thread.hpp"
#include "execution.hpp"
#include "../../component/scripting.hpp"

namespace scripting
{
	thread::thread(unsigned int id)
		: id_(id)
		 , type_(game::scr_VarGlob->objectVariableValue[id].w.type & 0x7F)
	{
	}

	script_value thread::get_raw() const
	{
		game::VariableValue value;
		value.type = game::SCRIPT_OBJECT;
		value.u.uintValue = this->id_;

		return value;
	}

	unsigned int thread::get_entity_id() const
	{
		return this->id_;
	}

	unsigned int thread::get_type() const
	{
		return this->type_;
	}

	unsigned int thread::get_wait_time() const
	{
		return game::scr_VarGlob->objectVariableValue[this->id_].w.waitTime >> 8;
	}

	unsigned int thread::get_notify_name_id() const
	{
		return game::scr_VarGlob->objectVariableValue[this->id_].w.notifyName >> 8;
	}

	unsigned int thread::get_self() const
	{
		return game::Scr_GetSelf(game::SCRIPTINSTANCE_SERVER, this->id_);
	}

	std::string thread::get_notify_name() const
	{
		return game::SL_ConvertToString(this->get_notify_name_id());
	}

	const char* thread::get_pos() const
	{
		const auto start_local_id = game::GetStartLocalId(game::SCRIPTINSTANCE_SERVER, this->id_);

		if (this->type_ == game::SCRIPT_THREAD)
		{
			for (auto frame = game::scr_VmPub->function_frame; frame != game::scr_VmPub->function_frame_start; --frame)
			{
				if (frame->fs.localId == this->id_)
				{
					return frame->fs.pos;
				}
			}
		}

		unsigned int stack_id = 0;

		if (this->type_ == game::SCRIPT_TIME_THREAD)
		{
			const auto waittime = this->get_wait_time();
			const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, game::scr_VarPub->timeArrayId, waittime);
			const auto object_id = game::scr_VarGlob->childVariableValue[variable_id].u.f.prev;

			stack_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, object_id, start_local_id + 0x10000);
		}

		if (this->type_ == game::SCRIPT_NOTIFY_THREAD)
		{
			const auto notify_name = this->get_notify_name_id();

			if (!notify_name)
			{
				stack_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, start_local_id, 0x16001);
			}
			else
			{
				const auto self = game::Scr_GetSelf(game::SCRIPTINSTANCE_SERVER, start_local_id);

				const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, game::scr_VarPub->pauseArrayId, self + 0x10000);
				const auto object_id = game::scr_VarGlob->childVariableValue[variable_id].u.f.prev;

				const auto variable_id2 = game::FindVariable(game::SCRIPTINSTANCE_SERVER, object_id, start_local_id + 0x10000);
				const auto variable = &game::scr_VarGlob->childVariableValue[variable_id2];

				const auto variable_id3 = game::FindVariable(game::SCRIPTINSTANCE_SERVER, variable->u.u.uintValue, 0x18000);
				const auto object_id2 = game::scr_VarGlob->childVariableValue[variable_id3].u.f.prev;

				const auto variable_id4 = game::FindVariable(game::SCRIPTINSTANCE_SERVER, object_id2, notify_name);
				const auto object_id3 = game::scr_VarGlob->childVariableValue[variable_id4].u.f.prev;

				stack_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, object_id3, start_local_id + 0x10000);
			}
		}

		const auto stack = &game::scr_VarGlob->childVariableValue[stack_id];
		if (stack && stack->type == 11)
		{
			return stack->u.u.stackValue->pos;
		}

		return 0;
	}

	const char* thread::get_start_pos() const
	{
		const auto current = this->get_pos();
		return scripting::find_function_start(current);
	}

	void thread::kill() const
	{
		const auto start_local_id = game::GetStartLocalId(game::SCRIPTINSTANCE_SERVER, this->id_);

		if (this->type_ == game::SCRIPT_THREAD)
		{
			game::Scr_TerminateRunningThread(game::SCRIPTINSTANCE_SERVER, this->id_);
		}

		if (this->type_ == game::SCRIPT_TIME_THREAD)
		{
			game::Scr_TerminateWaitThread(game::SCRIPTINSTANCE_SERVER, this->id_, start_local_id);
		}

		if (this->type_ == game::SCRIPT_NOTIFY_THREAD)
		{
			game::Scr_TerminateWaittillThread(game::SCRIPTINSTANCE_SERVER, this->id_, start_local_id);
		}
	}
}

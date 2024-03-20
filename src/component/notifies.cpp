#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/scripting/stack_isolation.hpp"
#include "gsc.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"

#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>

namespace notifies
{
	namespace
	{
		struct notify_group
		{
			unsigned int owner_id;
			unsigned int notify_target;
			unsigned int main_notify;
			std::unordered_set<unsigned int> sub_notifies;
		};

		using notify_groups_t = std::vector<notify_group>;

		bool in_notify_queue;
		notify_groups_t notify_groups_queue;

		utils::hook::detour vm_notify_hook;

		void push_sl_string(unsigned int value)
		{
			game::VariableValue var{};
			var.type = game::SCRIPT_STRING;
			var.u.stringValue = value;
			scripting::push_value(var);
		}

		void vm_notify_stub(game::scriptInstance_t inst, unsigned int notify_list_owner_id, 
			unsigned int string_value, game::VariableValue* top)
		{
			vm_notify_hook.invoke<void>(inst, notify_list_owner_id, string_value, top);

			if (in_notify_queue)
			{
				return;
			}

			const auto disconnect_str = *reinterpret_cast<unsigned short*>(0x24B71CE);

			in_notify_queue = true;
			const auto _0 = gsl::finally([]
			{
				in_notify_queue = false;
			});

			std::unordered_map<unsigned int, std::unordered_set<unsigned int>> notified;

			for (auto i = notify_groups_queue.begin(); i != notify_groups_queue.end(); )
			{
				if (i->owner_id == notify_list_owner_id)
				{
					const auto should_notify = i->sub_notifies.find(string_value) != i->sub_notifies.end();
					const auto should_delete = string_value == disconnect_str || notified[i->notify_target].contains(i->main_notify);
					const auto object_type = game::scr_VarGlob->objectVariableValue[i->notify_target].w.type;

					if (object_type != game::SCRIPT_FREE && should_notify)
					{
						scripting::stack_isolation _1;
						push_sl_string(string_value);
						game::Scr_NotifyId(inst, 0, i->notify_target, i->main_notify, 1);
						notified[i->notify_target].insert(i->main_notify);
					}

					if (should_notify || should_delete)
					{
						i = notify_groups_queue.erase(i);
						continue;
					}
				}

				++i;
			}
		}

		void clear_notify_group(unsigned int id)
		{
			for (auto i = notify_groups_queue.begin(); i != notify_groups_queue.end(); )
			{
				if (i->owner_id == id)
				{
					i = notify_groups_queue.erase(i);
				}
				else
				{
					++i;
				}
			}
		}

		void free_variable_stub_1(utils::hook::assembler& a)
		{
			a.pushad();
			a.push(ebx);
			a.call(clear_notify_group);
			a.pop(ebx);
			a.popad();
			
			a.movzx(ecx, word_ptr(edi, 2));
			a.xor_(edx, edx);
			a.jmp(0x550EA5);
		}

		void free_variable_stub_2(utils::hook::assembler& a)
		{
			a.pushad();
			a.push(edi);
			a.call(clear_notify_group);
			a.pop(edi);
			a.popad();

			a.mov(word_ptr(esi, 2), di);
			a.pop(edi);
			a.jmp(0x415EA2);
		}

		void* client_connect_stub(int client_num, int script_pers_id)
		{
			const scripting::entity player = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, client_num, 0, 0);
			scripting::notify(*game::levelEntityId, "direct_connect", {player});
			return utils::hook::invoke<void*>(0x41BE10, client_num, script_pers_id);
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

			vm_notify_hook.create(0x8F3620, vm_notify_stub);

			utils::hook::jump(0x550E9F, utils::hook::assemble(free_variable_stub_1));
			utils::hook::jump(0x415E9D, utils::hook::assemble(free_variable_stub_2));

			utils::hook::call(0x501F0F, client_connect_stub);

			scripting::on_shutdown([]
			{
				notify_groups_queue.clear();
			});

			gsc::function::add("createnotifygroup", [](const scripting::variadic_args& args) 
				-> scripting::script_value
			{
				std::unordered_set<unsigned int> notifies;

				const auto entity = args[0].get_raw();
				if (entity.type != game::SCRIPT_OBJECT)
				{
					throw std::runtime_error("argument 1 must be a script object");
				}

				const auto entity_target = args[1].get_raw();
				if (entity_target.type != game::SCRIPT_OBJECT)
				{
					throw std::runtime_error("argument 1 must be a script object");
				}

				const auto main_notify_str = args[2].as<std::string>();
				const auto main_notify = game::SL_GetString(main_notify_str.data(), 0);

				for (auto i = 3u; i < args.size(); i++)
				{
					const auto& arg = args[i];
					if (arg.is<scripting::array>())
					{
						const auto arr = arg.as<scripting::array>();
						for (const auto& [key, value] : arr)
						{
							const auto str = value.as<std::string>();
							notifies.insert(game::SL_GetString(str.data(), 0));
						}
					}
					else if (arg.is<std::string>())
					{
						const auto str = arg.as<std::string>();
						notifies.insert(game::SL_GetString(str.data(), 0));
					}
				}

				if (notifies.size() <= 0)
				{
					throw std::runtime_error("must pass atleast 1 notify");
				}

				notify_groups_queue.emplace_back(entity.u.uintValue, entity_target.u.uintValue,
					main_notify, notifies);

				return {};
			});
		}
	};
}

REGISTER_COMPONENT(notifies::component)

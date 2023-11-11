#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"
#include "scripting.hpp"

#include <utils/hook.hpp>

namespace config_strings
{
	namespace
	{
		std::unordered_set<std::uint32_t> reserved_config_strings;
		std::unordered_map<game::game_hudelem_s*, std::uint32_t> hudelem_config_strings;

		bool is_string_reserved(const std::uint32_t offset)
		{
			return reserved_config_strings.contains(offset);
		}

		std::uint32_t get_config_string()
		{
			auto index = -1;
			const auto scr_const_ = *reinterpret_cast<short*>(SELECT(0x24E5588, 0x24B7188));
			for (auto i = 1; i < 512; i++)
			{
				const auto string_value = game::sv_configstrings[i + 488];
				if (string_value == scr_const_ && !is_string_reserved(i))
				{
					index = i;
					break;
				}
			}

			if (index == -1)
			{
				throw std::runtime_error("out of config strings");
			}

			reserved_config_strings.insert(index);

			return index;
		}

		utils::hook::detour scr_free_hud_elem_hook;
		void scr_free_hud_elem_stub(game::game_hudelem_s* hud)
		{
			const auto _0 = gsl::finally([&]
			{
				scr_free_hud_elem_hook.invoke<void>(hud);
			});

			const auto iter = hudelem_config_strings.find(hud);
			if (iter == hudelem_config_strings.end())
			{
				return;
			}

			game::SV_SetConfigString(iter->second + 488, 0);
			reserved_config_strings.erase(iter->second);
			hudelem_config_strings.erase(iter);
		}

		void g_localized_string_index_stub_1(utils::hook::assembler& a)
		{
			const auto not_reserved = a.newLabel();

			a.add(esp, 4);

			a.pushad();
			a.push(esi);
			a.call(is_string_reserved);
			a.cmp(al, 0);
			a.jz(not_reserved);

			a.pop(esi);
			a.popad();
			a.jmp(SELECT(0x6D657E, 0x467D6E));

			a.bind(not_reserved);
			a.pop(esi);
			a.popad();

			a.cmp(eax, ecx);
			a.jmp(SELECT(0x6D6578, 0x467D68));
		}

		void g_localized_string_index_stub_2(utils::hook::assembler& a)
		{
			const auto not_reserved = a.newLabel();

			a.add(esp, 4);

			a.pushad();
			a.push(esi);
			a.call(is_string_reserved);
			a.cmp(al, 0);
			a.jz(not_reserved);

			a.pop(esi);
			a.popad();
			a.jmp(SELECT(0x6D6638, 0x467E2E));

			a.bind(not_reserved);
			a.pop(esi);
			a.popad();

			a.cmp(eax, ecx);
			a.jmp(SELECT(0x6D663E, 0x467E28));
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			utils::hook::jump(SELECT(0x6D6573, 0x467D63), utils::hook::assemble(g_localized_string_index_stub_1));
			utils::hook::jump(SELECT(0x6D6633, 0x467E23), utils::hook::assemble(g_localized_string_index_stub_2));
			scr_free_hud_elem_hook.create(SELECT(0x56E800, 0x6186A0), scr_free_hud_elem_stub);

			scripting::on_shutdown([&]
			{
				reserved_config_strings.clear();
				hudelem_config_strings.clear();
			});

			gsc::method::add("hudelem::set_text", [](const scripting::entity& entity, const std::string& text)
			{
				const auto ent = entity.get_entity_reference();
				const auto hudelem = &game::g_hudelems[ent.entnum];
				const auto iter = hudelem_config_strings.find(hudelem);

				if (iter == hudelem_config_strings.end())
				{
					const auto index = get_config_string();
					printf("reserve string %i\n", index);
					hudelem->elem.text = static_cast<short>(index);
					hudelem_config_strings.insert(std::make_pair(hudelem, index));
				}
				else
				{
					hudelem->elem.text = static_cast<short>(iter->second);
				}

				game::SV_SetConfigString(hudelem->elem.text + 488, text.data());
			});
		}
	};
}

REGISTER_COMPONENT(config_strings::component)

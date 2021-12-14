#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"

#include "scripting.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace scripting
{
	std::unordered_map<int, std::unordered_map<std::string, int>> fields_table;

	std::unordered_map<std::string, game::BuiltinMethodDef> method_map;
	std::unordered_map<std::string, game::BuiltinFunctionDef> function_map;

	std::unordered_map<std::string, std::unordered_map<std::string, const char*>> script_function_table;
	std::unordered_map<std::string, std::vector<std::pair<std::string, const char*>>> script_function_table_sort;
	std::unordered_map<const char*, std::pair<std::string, std::string>> script_function_table_rev;

	namespace
	{
		utils::hook::detour scr_add_class_field_hook;
		utils::hook::detour g_shutdown_game_hook;

		void scr_add_class_field_stub(game::scriptInstance_t inst, unsigned int classnum, char const* name, unsigned int offset)
		{
			if (fields_table[classnum].find(name) == fields_table[classnum].end())
			{
				fields_table[classnum][name] = offset;
			}

			scr_add_class_field_hook.invoke<void>(inst, classnum, name, offset);
		}

		game::BuiltinMethodDef get_method(const std::string& name)
		{
			game::BuiltinMethodDef method{};

			auto pName = name.data();
			int arg = 0;

			const auto func = game::Scr_GetMethod(&pName, &arg, &arg, &arg);
		
			if (func)
			{
				method.actionFunc = reinterpret_cast<script_function>(func);
				method.actionString = pName;
			}

			return method;
		}

		void load_functions()
		{
			for (auto i = 0; i < 278; i++)
			{
				const auto method = game::player_methods[i];
				method_map[method.actionString] = method;
			}

			for (auto i = 0; i < 22; i++)
			{
				const auto method = game::scriptEnt_methods[i];
				method_map[method.actionString] = method;
			}

			for (auto i = 0; i < 114; i++)
			{
				const auto method = game::scriptVehicle_methods[i];
				method_map[method.actionString] = method;
			}

			for (auto i = 0; i < 29; i++)
			{
				const auto method = game::hudElem_methods[i];
				method_map[method.actionString] = method;
			}

			for (auto i = 0; i < 29; i++)
			{
				const auto method = game::helicopter_methods[i];
				method_map[method.actionString] = method;
			}

			for (auto i = 0; i < 109; i++)
			{
				const auto method = game::actor_methods[i];
				method_map[method.actionString] = method;
			}

			for (auto i = 0; i < 95; i++)
			{
				const auto method = game::builtInCommon_methods[i];
				method_map[method.actionString] = method;
			}

			for (auto i = 0; i < 286; i++)
			{
				const auto method = game::builtIn_methods[i];
				method_map[method.actionString] = method;
			}

			for (auto i = 0; i < 334; i++)
			{
				const auto function = game::functions[i];
				function_map[function.actionString] = function;
			}

			for (auto i = 0; i < 220; i++)
			{
				const auto function = game::common_functions[i];
				function_map[function.actionString] = function;
			}

			method_map["notifyonplayercommand"] = get_method("notifyonplayercommand");
		}

		utils::hook::detour scr_load_script_hook;

		void extract_obj_functions(game::GSC_OBJ* obj)
		{
			const auto exported = reinterpret_cast<game::GSC_EXPORT_ITEM*>(&obj->magic[obj->exports_offset]);
			const auto count = obj->exports_count;
			auto iter = &obj->magic[obj->exports_offset + 8];

			std::string filename = &obj->magic[obj->name];
			if (filename.ends_with(".gsc"))
			{
				filename = filename.substr(0, filename.size() - 4);
			}

			for (auto i = 0; i < count; i++)
			{
				const auto index = *reinterpret_cast<unsigned __int16*>(iter);
				const auto function = &obj->magic[index];
				const auto address = reinterpret_cast<char*>(reinterpret_cast<unsigned int>(obj) + exported[i].address);

				script_function_table[filename][function] = address;
				script_function_table_sort[filename].push_back({function, address});

				iter += 12;
			}
		}

		utils::hook::detour scr_post_load_scripts_hook;
		unsigned int post_load_scripts_stub()
		{
			const auto script_count = *reinterpret_cast<unsigned int*>(SELECT(0x2DB9F18, 0x2D8A218));
			const auto scripts = reinterpret_cast<game::objFileInfo_t*>(SELECT(0x2DA2FE8, 0x2D732E8));

			for (auto i = 0; i < script_count; i++)
			{
				const auto obj = scripts[i].activeVersion;
				extract_obj_functions(obj);
			}

			return scr_post_load_scripts_hook.invoke<unsigned int>();
		}

		std::vector<std::function<void()>> shutdown_callbacks;
		void g_shutdown_game_stub(const int free_scripts)
		{
			for (const auto& callback : shutdown_callbacks)
			{
				callback();
			}

			return g_shutdown_game_hook.invoke<void>(free_scripts);
		}
	}

	script_function find_function_ptr(const std::string& _name)
	{
		const auto name = utils::string::to_lower(_name);

		if (function_map.find(name) != function_map.end())
		{
			const auto func = function_map[name].actionFunc;
			return reinterpret_cast<script_function>(func);
		}

		if (method_map.find(name) != method_map.end())
		{
			const auto func = method_map[name].actionFunc;
			return reinterpret_cast<script_function>(func);
		}

		return nullptr;
	}

	std::string find_function(const char* pos)
	{
		for (const auto& file : script_function_table_sort)
		{
			for (auto i = file.second.begin(); std::next(i) != file.second.end(); ++i)
			{
				const auto next = std::next(i);
				if (pos >= i->second && pos < next->second)
				{
					return utils::string::va("%s::%s", file.first.data(), i->first.data());
				}
			}
		}

		return "unknown function";
	}

	std::optional<std::pair<std::string, std::string>> find_function_pair(const char* pos)
	{
		for (const auto& file : script_function_table_sort)
		{
			for (auto i = file.second.begin(); std::next(i) != file.second.end(); ++i)
			{
				const auto next = std::next(i);
				if (pos >= i->second && pos < next->second)
				{
					return {{file.first, i->first}};
				}
			}
		}

		return {};
	}

	const char* find_function_start(const char* pos)
	{
		for (const auto& file : script_function_table_sort)
		{
			for (auto i = file.second.begin(); std::next(i) != file.second.end(); ++i)
			{
				const auto next = std::next(i);
				if (pos >= i->second && pos < next->second)
				{
					return i->second;
				}
			}
		}

		return 0;
	}

	void on_shutdown(const std::function<void()>& callback)
	{
		shutdown_callbacks.push_back(callback);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			g_shutdown_game_hook.create(SELECT(0x60DCF0, 0x688A40), g_shutdown_game_stub);

			scr_add_class_field_hook.create(SELECT(0x6B7620, 0x438AD0), scr_add_class_field_stub);
			scr_post_load_scripts_hook.create(SELECT(0x642EB0, 0x425F80), post_load_scripts_stub);

			load_functions();
		}
	};
}

REGISTER_COMPONENT(scripting::component)

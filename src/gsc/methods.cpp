#include <stdafx.hpp>

namespace method
{
    namespace
    {
        std::unordered_map<std::string, game::BuiltinMethodDef> registered_methods;
    }

    void add(const char* name, int min_args, int max_args, void(*f)(game::scr_entref_t))
    {
        if (find(name) != nullptr)
        {
            return;
        }

        registered_methods[name] = {};
        auto itr = registered_methods.find(name);

        itr->second.actionString = name;
        itr->second.type = 1;
        itr->second.min_args = min_args;
        itr->second.max_args = max_args;
        itr->second.actionFunc = f;
    }

    game::BuiltinMethodDef* find(const std::string& name)
    {
        auto itr = registered_methods.find(name);

        if (itr != registered_methods.end())
        {
            return &itr->second;
        }

        return nullptr;
    }
}
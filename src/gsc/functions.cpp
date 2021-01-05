#include <stdafx.hpp>

namespace function
{
    namespace
    {
        std::unordered_map<std::string, game::BuiltinFunctionDef> registered_functions;
    }

    void add(const char* name, int min_args, int max_args, void(*f)())
    {
        if (find(name) != nullptr)
        {
            return;
        }

        registered_functions[name] = {};
        auto itr = registered_functions.find(name);

        itr->second.actionString = name;
        itr->second.type = 1;
        itr->second.min_args = min_args;
        itr->second.max_args = max_args;
        itr->second.actionFunc = f;
    }

    game::BuiltinFunctionDef* find(const std::string& name)
    {
        auto itr = registered_functions.find(name);

        if (itr != registered_functions.end())
        {
            return &itr->second;
        }

        return nullptr;
    }
}
#include <map>

#include "LeftMouseHook.h"

std::map<std::string, LeftMouseHook> hooks;

void installHook(std::string name, LeftMouseHook hook)
{
    hooks[name] = hook;
}

void removeHook(std::string name)
{
    hooks.erase(name);
}

bool lMouseDown(FOClient* client)
{
    for (auto hook: hooks) {
        if (hook.second(client)) {
            return true;
        }
    }

    return false;
}

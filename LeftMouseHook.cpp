#include <map>

#include "State.h"
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

bool hookInstalled(std::string name)
{
    return hooks.find(name) != std::end(hooks);
}

bool lMouseDown(FOClient* client)
{
    if (!state) {
        return false;
    }

    state->complexAction = nullptr;

    for (auto hook: hooks) {
        if (hook.second(client)) {
            return true;
        }
    }

    return false;
}

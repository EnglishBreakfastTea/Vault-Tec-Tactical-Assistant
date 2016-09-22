#include <windows.h>
#include <cstdio>
#include <algorithm>
#include <map>

#include "Hotkeys.h"

Hotkey::Hotkey(bool ctrl, bool alt, bool shift, char ch)
    : hotkey(ch | ctrl << 8 | alt << 9 | shift << 10)
{
}

bool operator <(Hotkey const& x, Hotkey const& y)
{
    return x.hotkey < y.hotkey;
}

std::map<Hotkey, HotkeyHook> hooks;

void installHotkeyHook(Hotkey hotkey, HotkeyHook hook)
{
    hooks[hotkey] = hook;
}

void removeHotkeyHook(Hotkey hotkey, HotkeyHook hook)
{
    hooks.erase(hotkey);
}

bool parseKeyboard(FOClient* client)
{
    auto constexpr numKeys = 256;
    std::array<bool, numKeys> static lastKeysDown = {0};

    if (!mainWindow()->windowActive) {
        return false;
    }

    std::array<bool, numKeys> keysDown;
    for (auto i = 0; i < numKeys; ++i) {
        keysDown[i] = GetAsyncKeyState(i) & (1 << 16);
    }

    // Keys that were pressed just now.
    std::array<bool, numKeys> keysPressed;
    std::transform(std::begin(keysDown), std::end(keysDown), std::begin(lastKeysDown),
                   std::begin(keysPressed), [](auto newState, auto oldState) { return newState && !oldState; });

    std::copy(std::begin(keysDown), std::end(keysDown), std::begin(lastKeysDown));

    for (auto vk = 0; vk < numKeys; ++vk) {
        if (!keysPressed[vk]) {
            continue;
        }

        auto ch = static_cast<char>(255 & MapVirtualKeyA(vk, MAPVK_VK_TO_CHAR));
        if (!ch) {
            continue;
        }

        auto ctrl = keysDown[VK_CONTROL];
        auto alt = keysDown[VK_MENU];
        auto shift = keysDown[VK_SHIFT];
        auto it = hooks.find({ctrl, alt, shift, ch});
        if (it == std::end(hooks)) {
            continue;
        }

        it->second(client);

        return true;
    }

    return false;
}

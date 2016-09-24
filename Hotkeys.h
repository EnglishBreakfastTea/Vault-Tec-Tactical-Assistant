#ifndef _HOTKEYS_H
#define _HOTKEYS_H

#include <functional>

#include "FOClient.h"

/* HotkeyHook is a function which, if installed, will be called when the associated hotkey is used.
 *
 * Each hotkey can have at most one function associated with it. The function can be arbitrarily complex though.
 *
 * If a HotkeyHook is executed, the game code for keyboard handling won't be executed. That means that installed hooks
 * override normal in-game hotkeys.
 */

using HotkeyHook = std::function<void(FOClient*)>;

struct Hotkey {
    Hotkey(bool ctrl, bool alt, bool shift, char ch);

    friend bool operator <(Hotkey const&, Hotkey const&);
private:
    /* Lower 8 bits: char; 9th bit: ctrl modifier; 10th bit: alt modifier; 11th bit: shift modifier. */
    uint16_t hotkey;

};

/* Install a hook. If there was a hook with the given hotkey installed, it will be removed. */
void installHotkeyHook(Hotkey, HotkeyHook);

/* Remove a hook. Does nothing if there was no hook with the given hotkey. */
void removeHotkeyHook(Hotkey, HotkeyHook);

/* Check for pressed hotkeys and call the associated hooks. If no hook is executed, lets the game perform
 * its normal check for hotkeys. Otherwise it modifies the game state as if nothing was pressed. */
void parseKeyboard(FOClient*);

#endif //_HOTKEYS_H

#ifndef _LEFTMOUSEHOOK_H
#define _LEFTMOUSEHOOK_H

#include <functional>

#include "FOClient.h"

/* LeftMouseHook is a function which, if enabled, will be called on a left mouse click.
 *
 * The return value indicates if the hook executed, e.g. a hook may execute only when clicking on a critter
 * in attack mode. In that situation, the hook returns true. Otherwise it returns false.
 *
 * Multiple hooks may be installed. If a hook was executed, no more hooks will be executed and the game code for
 * clicking the left mouse button won't be executed. If no hooks were executed, normal game code will be executed.
 *
 * The installed hooks are called in an arbitrary order. Because of this, you should not install conflicting hooks,
 * e.g. two hooks which are called when clicking in move mode, because you won't know which hook will be executed
 * and which won't.
 *
 * Every hook has a name, which uniquely identifies it. Installing a hook whose name is already used by another hook
 * removes the old hook.
 *
 * Hooks are executed only inside encounters (cities and other maps qualify, but World Map doesn't).
 */

using LeftMouseHook = std::function<bool(FOClient*)>;

/* Install a hook. If there was a hook with the given name installed, it will be removed. */
void installHook(std::string name, LeftMouseHook);

/* Remove a hook. Does nothing if there was no hook with the given name. */
void removeHook(std::string name);

/* Checks if a hook with the given name is installed. */
bool hookInstalled(std::string name);

/* Call the installed hooks. If no hook is executed, returns false, indicating that the normal game code for left mouse
 * click should be performed. Otherwise returns true. */
bool lMouseDown(FOClient*);


#endif //_LEFTMOUSEHOOK_H

#include "InterfaceOverlay.h"
#include "LeftMouseHook.h"

void drawIface(FOClient* client)
{
    if (client->gameMode != IN_ENCOUNTER) {
        return;
    }

    if (mouseHookInstalled("1hex")) {
        drawString("Hexbot enabled, you cheating cuck", 10, 50, 130, 30, GREEN, NORMAL, BORDER);
    }
}

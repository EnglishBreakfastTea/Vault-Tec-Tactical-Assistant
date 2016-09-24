#include "LeftMouseHook.h"
#include "BackgroundJob.h"
#include "InterfaceOverlay.h"

void drawIface(FOClient* client)
{
    if (client->gameMode != IN_ENCOUNTER) {
        return;
    }

    if (mouseHookInstalled("1hex")) {
        drawString("Hexbot enabled", 10, 50, 130, 30, GREEN, NORMAL, BORDER);
    }

    if (jobRunning("center")) {
        drawString("Centering", 10, 60, 130, 30, GREEN, NORMAL, BORDER);
    }
}

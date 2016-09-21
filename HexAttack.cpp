#include <cstdio>

#include "FOClient.h"

#include "HexAttack.h"
#include "State.h"

HexAttack::HexAttack(uint32_t critterId)
    : critterId(critterId)
{
    printf("starting HexAttack\n");
}

bool HexAttack::frame(FOClient* client)
{
    auto critter = getCritter(&client->hexManager, critterId);

    if (client->gameMode != IN_ENCOUNTER) {
        printf("HexAttack: not inside encounter.\n");
        return true;
    }

    if (!critter) {
        printf("HexAttack: lost the critter.\n");
        return true;
    }

    if (client->hexManager.playerNear(critter)) {
        printf("HexAttack: near, attacking!\n");
        client->attack(critter->critterId);
        return true;
    }

    client->move(critter->x, critter->y);

    return false;
}

LeftMouseHook HexAttackHook = [](FOClient* client)
{
    if (client->mouseMode != MOUSE_ATTACK) {
        printf("HexAttackHook: not in attack mode\n");
        return false;
    }

    if (client->playerCritter->hand->mode > 1) {
        printf("HexAttackHook: not in non-aimed mode\n");
        return false;
    }

    auto critter = client->hexManager.critterUnderMouse();
    if (!critter) {
        printf("HexAttackHook: no critter under mouse\n");
        return false;
    }

    state->complexAction = std::make_unique<HexAttack>(critter->critterId);

    return true;
};

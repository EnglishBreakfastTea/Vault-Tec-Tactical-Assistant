#include <cstdio>

#include "FOClient.h"

#include "HexAttack.h"

HexAttack::HexAttack(uint32_t critterId)
    : critterId(critterId)
{
    printf("starting HexAttack\n");
}

bool HexAttack::frame(FOClient* client)
{
    auto critter = getCritter(&client->hexManager, critterId);

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

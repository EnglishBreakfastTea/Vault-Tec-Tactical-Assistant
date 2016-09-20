#include <cstdio>

#include "State.h"
#include "HexAttack.h"
#include "mainLoop.h"

void mainLoop(FOClient* client)
{
    auto static state = std::unique_ptr<State>();

    if (!state) {
        setup(state);

        printf("\nFOClient address: %d\n", reinterpret_cast<uint32_t>(client));
    }

    auto msg = state->getMessage();

    if (!msg) {
        if (state->complexAction) {
            auto actionFinished = state->complexAction->frame(client);
            if (actionFinished) {
                printf("Complex action finished\n");
                state->complexAction = nullptr;
            }
        }

        return;
    }

    // Any received messages stop the current ComplexAction. For now...
    state->complexAction = nullptr;

    printf("received message: %s\n", msg->data());

    // TODO: smarter message handling dispatch.
    {
        uint32_t x, y;
        if (sscanf(msg->data(), "move %u %u", &x, &y) == 2) {
            printf("a move message\n");
            client->move(x, y);
            printf("called move\n");
            return;
        }
    }

    {
        if (*msg == "attack") {
            printf("attack message\n");

            auto critter = client->hexManager.critterUnderMouse();
            if (!critter) {
                return;
            }

            client->attack(critter->critterId);

            return;
        }
    }

    {
        if (*msg == "check near") {
            auto critter = client->hexManager.critterUnderMouse();
            if (!critter) {
                printf("no critter under mouse\n");
                return;
            }

            if (client->hexManager.playerNear(critter)) {
                printf("near!\n");
            } else {
                printf("not near\n");
            }

            return;
        }
    }

    {
        if (*msg == "1hex") {
            auto critter = client->hexManager.critterUnderMouse();
            if (!critter) {
                return;
            }

            state->complexAction = std::make_unique<HexAttack>(critter->critterId);

            return;
        }
    }
}

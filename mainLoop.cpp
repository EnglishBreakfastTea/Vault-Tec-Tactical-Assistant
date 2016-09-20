#define BOOST_DATE_TIME_NO_LIB

#include <cstdio>
#include <cstdint>
#include <memory>
#include <experimental/optional>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "messageQueue.h"
#include "mainLoop.h"

using namespace boost::interprocess;
using std::experimental::optional;

int const mqMaxMsgSize = 100;

auto fastTick = reinterpret_cast<FastTick>(FastTickAddr);
auto isAction = reinterpret_cast<IsAction>(IsActionAddr);
auto setAction = reinterpret_cast<SetAction>(SetActionAddr);
auto getSmthPixel = reinterpret_cast<GetSmthPixel>(GetSmthPixelAddr);
auto getCritter = reinterpret_cast<GetCritter>(GetCritterAddr);

auto mouseX = reinterpret_cast<uint32_t*>(MouseXAddr);
auto mouseY = reinterpret_cast<uint32_t*>(MouseYAddr);

void move(FOClient* client, uint32_t x, uint32_t y)
{
    auto time = fastTick();
    auto moving = isAction(client, 1);
    auto ACTION_MOVE = 1u;
    auto run = 1u;
    // reverse x, y?
    setAction(client, ACTION_MOVE, x, y, run, 0, !moving, time);
}

/* Attack the given critter using the in-game attack mode (e.g. burst) */
void attack(FOClient* client, uint32_t critterId)
{
    auto ACTION_ATTACK = 6u;

    auto playerCritter = client->playerCritter;
    if (!playerCritter) {
        printf("no player critter!\n");
        return;
    }

    auto hand = playerCritter->hand;
    if (!hand) {
        printf("what, no hand\n");
        return;
    }

    auto mode = hand->mode;

    auto heldItem = hand->heldItem;
    if (!heldItem) {
        // an empty hand should also be an item
        printf("no hand item\n");
        return;
    }

    setAction(client, ACTION_ATTACK, hand->handItemId, heldItem->itemId, 2, critterId, mode, 0);
}

Critter* critterUnderMouse(HexManager* hexManager)
{
    Object* obj = nullptr;
    Critter* critter = nullptr;
    getSmthPixel(hexManager, *mouseX, *mouseY, obj, critter);

    if (!critter) {
        return nullptr;
    }

    return critter;
}

bool crittersNeighbours(Critter* critter1, Critter* critter2)
{
    assert(critter1);
    assert(critter2);

    // A critter will always be near itself.
    auto static evenXDeltas = std::array<std::pair<int, int>, 7>
            {{{0, 0}, {1,0}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}}};
    auto static oddXDeltas = std::array<std::pair<int, int>, 7>
            {{{0, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {0, 1}, {1, 0}}};

    if (critter1->x % 2 == 0) {
        return std::any_of(std::begin(evenXDeltas), std::end(evenXDeltas), [=](auto delta) {
            return critter1->x + delta.first == critter2->x
                && critter1->y + delta.second == critter2->y;
        });
    }

    return std::any_of(std::begin(oddXDeltas), std::end(oddXDeltas), [=](auto delta) {
        return critter1->x + delta.first == critter2->x
            && critter1->y + delta.second == critter2->y;
    });
}

bool playerNear(HexManager* hexManager, Critter* critter)
{
    assert(critter);

    auto playerCritter = getCritter(hexManager, hexManager->playerCritterId);
    return crittersNeighbours(critter, playerCritter);
}

/* A ComplexAction is a (non-trivial) algorithm that performs (multiple) FOnline actions.
 * Constructing a ComplexAction initiates the sequence of actions to be performed, starting with the next frame.
 * Destructing a ComplexAction stops any new actions in the sequence from being performed in the upcoming frames.
 * The constructor must not start any action. That comes later in the frame. */
class ComplexAction {
public:
    /* Returns true when the action is finished. */
    virtual bool frame(FOClient*) = 0;
};

class HexAttack : public ComplexAction {
public:
    HexAttack(uint32_t critterId)
        : critterId(critterId)
    {
        printf("starting HexAttack\n");
    }

    bool frame(FOClient* client) override
    {
        auto critter = getCritter(&client->hexManager, critterId);

        if (!critter) {
            printf("HexAttack: lost the critter.\n");
            return true;
        }

        if (playerNear(&client->hexManager, critter)) {
            printf("HexAttack: near, attacking!\n");
            attack(client, critter->critterId);
            return true;
        }

        move(client, critter->x, critter->y);
        return false;
    }

private:
    uint32_t critterId;
};

struct State {
    State()
        : mq(create_only, mqName, 100, mqMaxMsgSize)
        , complexAction(nullptr)
    {
    }

    ~State()
    {
        message_queue::remove(mqName);
    }

    message_queue mq;
    std::unique_ptr<ComplexAction> complexAction;
};

optional<std::string> getMessage(State& state)
{
    static char buffer[mqMaxMsgSize];
    size_t msgSize;
    unsigned msgPriority;
    auto received = state.mq.try_receive(buffer, mqMaxMsgSize, msgSize, msgPriority);

    if (received) {
        return std::string(buffer, msgSize);
    } else {
        return {};
    }
}

void setup(std::unique_ptr<State>& state)
{
    // TODO: smarter handling of an existing queue.
    message_queue::remove(mqName);
    state = std::make_unique<State>();
}

void mainLoop(FOClient* client)
{
    auto static state = std::unique_ptr<State>();

    if (!state) {
        setup(state);

        printf("\nFOClient address: %d\n", reinterpret_cast<uint32_t>(client));
    }

    auto msg = getMessage(*state);

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
            move(client, x, y);
            printf("called move\n");
            return;
        }
    }

    {
        if (*msg == "attack") {
            printf("attack message\n");

            auto critter = critterUnderMouse(&client->hexManager);
            if (!critter) {
                return;
            }

            attack(client, critter->critterId);

            return;
        }
    }

    {
        if (*msg == "check near") {
            auto critter = critterUnderMouse(&client->hexManager);
            if (!critter) {
                printf("no critter under mouse\n");
                return;
            }

            if (playerNear(&client->hexManager, critter)) {
                printf("near!\n");
            } else {
                printf("not near\n");
            }

            return;
        }
    }

    {
        if (*msg == "1hex") {
            auto critter = critterUnderMouse(&client->hexManager);
            if (!critter) {
                return;
            }

            state->complexAction = std::make_unique<HexAttack>(critter->critterId);

            return;
        }
    }
}

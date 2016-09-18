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

auto mouseX = reinterpret_cast<uint32_t*>(MouseXAddr);
auto mouseY = reinterpret_cast<uint32_t*>(MouseYAddr);

void move(FOClient* client, uint32_t x, uint32_t y)
{
    auto time = fastTick();
    auto moving = isAction(client, 1);
    auto ACTION_MOVE = 1u;
    auto run = 1;
    // reverse x, y?
    setAction(client, ACTION_MOVE, x, y, run, 0, !moving, time);
}

void attack(FOClient* client, uint32_t critterId)
{
    auto ACTION_ATTACK = 6u;
    auto NO_AIM = 0u;
    setAction(client, ACTION_ATTACK, 0, 0x3e9, 2, critterId, NO_AIM, 0);
}

struct State {
    State()
        : mq(create_only, mqName, 100, mqMaxMsgSize)
        , stuff(5)
    {
    }

    ~State()
    {
        message_queue::remove(mqName);
    }

    int stuff;
    message_queue mq;
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
    message_queue::remove(mqName); //TODO?
    state = std::make_unique<State>();
}

void mainLoop(FOClient* client)
{
    auto static state = std::unique_ptr<State>();

    if (!state) {
        setup(state);

        printf("\nFOClient address: %d\n", reinterpret_cast<uint32_t>(client));
        printf("state stuff: %d\n", state->stuff);
    }

    auto msg = getMessage(*state);
    if (!msg) {
        return;
    }

    printf("received message: %s\n", msg->data());

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

            Object* obj = nullptr;
            Critter* critt = nullptr;
            getSmthPixel(&client->hexManager, *mouseX, *mouseY, obj, critt);

            if (obj) {
                printf("mouse points at an object\n");
            }

            if (critt) {
                printf("mouse points at a critter\n");
            }

            if (!critt) {
                return;
            }

            attack(client, critt->critterId);
        }
    }
}

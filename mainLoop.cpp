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

void move(FOClient* client, uint32_t x, uint32_t y)
{
    auto time = fastTick();
    auto moving = isAction(client, 1);
    auto ACTION_MOVE = 1u;
    auto run = 1;
    // reverse x, y?
    setAction(client, ACTION_MOVE, x, y, run, 0, !moving, time);
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
    if (msg) {
        printf("received message: %s\n", msg->data());

        uint32_t x, y;
        if (sscanf(msg->data(), "move %u %u", &x, &y) == 2) {
            printf("a move message\n");
            move(client, x, y);
            printf("called move\n");
        }
    }
}

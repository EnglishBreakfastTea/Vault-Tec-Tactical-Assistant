#include "State.h"

using std::experimental::optional;
using namespace boost::interprocess;

int constexpr mqMaxMsgSize = 100;

State::State()
        : mq(create_only, mqName, 100, mqMaxMsgSize)
        , complexAction(nullptr)
{
}

State::~State()
{
    message_queue::remove(mqName);
}

optional<std::string> State::getMessage()
{
    static char buffer[mqMaxMsgSize];
    size_t msgSize;
    unsigned msgPriority;
    auto received = mq.try_receive(buffer, mqMaxMsgSize, msgSize, msgPriority);

    if (received) {
        return std::string(buffer, msgSize);
    } else {
        return {};
    }
}

std::unique_ptr<State> state = nullptr;

void setup(std::unique_ptr<State>& state)
{
    // TODO: smarter handling of an existing queue.
    message_queue::remove(mqName);
    state = std::make_unique<State>();
}

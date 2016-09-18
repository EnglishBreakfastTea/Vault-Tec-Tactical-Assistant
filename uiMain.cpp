#include <iostream>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "messageQueue.h"

using namespace boost::interprocess;

int main()
{
    message_queue mq(open_only, mqName);

    std::string msg;
    for (;;) {
        std::getline(std::cin, msg);

        if (msg == "quit") {
            return 0;
        }

        mq.send(msg.data(), msg.size(), 0);
    }
}

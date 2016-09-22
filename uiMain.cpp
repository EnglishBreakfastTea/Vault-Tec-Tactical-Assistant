#include <iostream>
#include <memory>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "messageQueue.h"

using namespace boost::interprocess;

int main()
{
    std::unique_ptr<message_queue> mq = nullptr;

    try {
        mq = std::make_unique<message_queue>(open_only, mqName);
    } catch (std::exception const&){
        std::cout << "Couldn't connect to the game. Make sure the game is running and try again." << std::endl;
        std::cout << "Press any key..." << std::endl;
        std::getchar();
        return 0;
    }

    std::string msg;
    for (;;) {
        std::getline(std::cin, msg);

        if (msg == "quit") {
            return 0;
        }

        mq->send(msg.data(), msg.size(), 0);
    }
}

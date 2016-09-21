#define BOOST_DATE_TIME_NO_LIB

#ifndef _STATE_H
#define _STATE_H

#include <experimental/optional>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "messageQueue.h"
#include "ComplexAction.h"

/* Global state shared between various hooks. */

class State {
public:
    State();
    ~State();

    /* Retrieves a message from the UI if there is one. */
    std::experimental::optional<std::string> getMessage();

    /* The currently performed complexAction or nullptr if there is none. */
    std::unique_ptr<ComplexAction> complexAction;
private:
    /* The message queue used to communicate with the UI. */
    boost::interprocess::message_queue mq;
};

extern std::unique_ptr<State> state;

/* Create the state. Must be run only once and from the mainLoop hook. */
void setup(std::unique_ptr<State>&);

#endif //_STATE_H

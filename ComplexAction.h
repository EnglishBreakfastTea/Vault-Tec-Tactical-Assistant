#ifndef _COMPLEXACTION_H
#define _COMPLEXACTION_H

#include "FOClient.h"

/* A ComplexAction is a (non-trivial) algorithm that performs (multiple) FOnline actions.
 * Constructing a ComplexAction initiates the sequence of actions to be performed, starting with the next frame.
 * Destructing a ComplexAction stops any new actions in the sequence from being performed in the upcoming frames.
 * The constructor must not start any action. That comes later in the frame. */

class ComplexAction {
public:
    /* Returns true when the action is finished. */
    virtual bool frame(FOClient*) = 0;
};

#endif //_COMPLEXACTION_H

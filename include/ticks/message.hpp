#ifndef INCLUDE_TICKS_MESSAGE_HPP_
#define INCLUDE_TICKS_MESSAGE_HPP_

#include "ticks/ids.hpp"

struct TickMessage {
    TickMessage(TickID id, int msg)
        :id {id}, msg {msg} {}

    TickID id;
    int msg;
};

#endif  // INCLUDE_TICKS_MESSAGE_HPP_

#pragma once

#include "model/StampedDMXFrame.hpp"

/*
a SACNReceiver is responsible for listening to the network and being able to ALWAYS give a copy of the last packet received
for now it will own and hardcode the connection resource, in the future maybe that should be injected in
*/
class SACNReceiver
{
public:
    // start listening for sacn
    virtual bool begin() = 0;
    // return true if something new and mutate callers obj, otherwise false and dont mutate
    virtual bool poll(StampedDMXFrame &out) = 0;

    virtual ~SACNReceiver() = default;
};
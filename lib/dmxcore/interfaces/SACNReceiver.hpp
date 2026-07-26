#pragma once

#include "model/DMXFrame.hpp"

/*
a SACNReceiver is responsible for listening to the network and being able to ALWAYS give a copy of the last packet received
for now it will own and hardcode the connection resource, in the future maybe that should be injected in
*/
class SACNReceiver
{
public:
    // start listening for sacn
    virtual bool begin() = 0;
    // serve the latest frame by value, 512 byte copy is microseconds, optimize later once main behavior is solved
    virtual DMXFrame latest() = 0;

    virtual ~SACNReceiver() = default;
};
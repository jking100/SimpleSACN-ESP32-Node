#pragma once

#include "model/StampedDMXFrame.hpp"

/*
base class for a thing that can take a dmx frame and send it out to fixtures
*/
class DMXSender
{
public:
    // init whatever is needed for send() to function
    virtual void begin(dataLossOption) = 0;
    // push the frame to hardware
    virtual bool send(const StampedDMXFrame &) = 0;

    virtual ~DMXSender() = default;
};

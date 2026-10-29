#pragma once

#include "model/StampedDMXFrame.hpp"

/*
base class for a thing that can take a dmx frame and send it out to fixtures
*/
class DMXSender
{
public:
    // todo: this needs to move out of here becuase deciding how to hold the frame in here is more complicated that this just pushing whatever it gets
    enum class dataLossOption
    {
        HOLD,
        BLACKOUT
    };

    // init whatever is needed for send() to function
    virtual void begin(dataLossOption) = 0;
    // push the frame to hardware
    virtual bool send(const StampedDMXFrame &) = 0;

    virtual ~DMXSender() = default;
};

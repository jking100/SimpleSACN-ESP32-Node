#pragma once

#include "model/DMXFrame.hpp"

/*
base class for a thing that can take a dmx frame and send it out to fixtures
*/
class DMXSender
{
    // todo :: "what happens when you delete through a base-class pointer with a non-virtual destructor? Then ask whether that scenario can even occur in your statically-allocated firmware, and what the zero-cost convention is anyway (virtual ~X() = default;)"
public:
    // init whatever is needed for send() to function
    virtual void begin() = 0;
    // push the frame to hardware
    virtual bool send(const DMXFrame &) = 0;

    virtual ~DMXSender() = default;
};

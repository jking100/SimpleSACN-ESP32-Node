#pragma once

#include "model/DMXFrame.hpp"

// thread safe frame mailbox; will always return the last frame given to publish, or nothing if not published ever
class DMXMailbox
{
public:
    virtual bool begin() = 0;
    virtual void publish(const DMXFrame &f) = 0;
    virtual bool latest(DMXFrame &out) = 0;
    virtual ~DMXMailbox() = default;
};
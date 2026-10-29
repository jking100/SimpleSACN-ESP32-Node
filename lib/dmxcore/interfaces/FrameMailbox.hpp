#pragma once

#include "model/StampedDMXFrame.hpp"

// thread safe frame mailbox; will always return the last frame given to publish, or nothing if not published ever
class FrameMailbox
{
public:
    virtual bool begin() = 0;
    virtual void publish(const StampedDMXFrame &f) = 0;
    virtual bool latest(StampedDMXFrame &out) = 0;
    virtual ~FrameMailbox() = default;
};
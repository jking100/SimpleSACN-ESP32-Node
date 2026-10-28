#pragma once

#include "model/DMXFrame.hpp"

// frame mailbox will always return the last frame given to publish
// it needs to be thread safe
class DMXMailbox
{
public:
    virtual void publish(const DMXFrame &f) = 0;
    virtual bool latest(DMXFrame &out) = 0;
    virtual ~DMXMailbox() = 0;
};
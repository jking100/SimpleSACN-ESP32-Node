#pragma once

#include <cstring>

#include "model/StampedDMXFrame.hpp"
#include "interfaces/FrameMailbox.hpp"

// Dummy nonthreadsafe Mailbox implemented using a single dmx frame
class FakeDMXMailbox : public FrameMailbox
{
public:
    bool begin() override { return true; }

    void publish(const StampedDMXFrame &f) override
    {
        if (!_published)
            _published = true;
        std::memcpy(&_frame, &f, sizeof(_frame));
    }

    bool latest(StampedDMXFrame &out) override
    {
        if (!_published)
            return false;
        std::memcpy(&out, &_frame, sizeof(out));
        return true;
    }

    // ~DMXMailbox() = default;
private:
    StampedDMXFrame _frame;
    bool _published{false};
};
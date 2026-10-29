#pragma once

#include <cstring>

#include "model/DMXFrame.hpp"
#include "interfaces/DMXMailbox.hpp"

// Dummy nonthreadsafe Mailbox implemented using a single dmx frame
class FakeDMXMailbox : public DMXMailbox
{
public:
    bool begin() override { return true; }

    void publish(const DMXFrame &f) override
    {
        if (!_published)
            _published = true;
        std::memcpy(&_frame, &f, sizeof(_frame));
    }

    bool latest(DMXFrame &out) override
    {
        if (!_published)
            return false;
        std::memcpy(&out, &_frame, sizeof(out));
        return true;
    }

    // ~DMXMailbox() = default;
private:
    DMXFrame _frame;
    bool _published{false};
};
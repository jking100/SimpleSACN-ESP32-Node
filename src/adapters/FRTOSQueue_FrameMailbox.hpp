#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "model/StampedDMXFrame.hpp"
#include "interfaces/FrameMailbox.hpp"

// Threadsafe Mailbox implemented using a size 1 free rtos queue of DMXFrame objects
class FRTOSQueue_FrameMailbox : public FrameMailbox
{
public:
    bool begin() override
    {
        _frameMailbox = xQueueCreate(1, sizeof(StampedDMXFrame));
        return _frameMailbox != nullptr;
    }

    void publish(const StampedDMXFrame &f) override
    {
        xQueueOverwrite(_frameMailbox, &f);
    }

    bool latest(StampedDMXFrame &out) override
    {
        return xQueuePeek(_frameMailbox, &out, 0);
    }

    // ~DMXMailbox() = default;
private:
    QueueHandle_t _frameMailbox{nullptr};
};
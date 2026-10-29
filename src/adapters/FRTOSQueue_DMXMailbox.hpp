#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "model/DMXFrame.hpp"
#include "interfaces/DMXMailbox.hpp"

// Threadsafe Mailbox implemented using a size 1 free rtos queue of DMXFrame objects
class FRTOSQueue_DMXMailbox : public DMXMailbox
{
public:
    bool begin() override
    {
        _frameMailbox = xQueueCreate(1, sizeof(DMXFrame));
        return _frameMailbox != nullptr;
    }

    void publish(const DMXFrame &f) override
    {
        xQueueOverwrite(_frameMailbox, &f);
    }

    bool latest(DMXFrame &out) override
    {
        return xQueuePeek(_frameMailbox, &out, 0);
    }

    // ~DMXMailbox() = default;
private:
    QueueHandle_t _frameMailbox{nullptr};
};
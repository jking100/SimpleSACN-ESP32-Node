#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "interfaces/DMXSender.hpp"
#include "interfaces/SACNReceiver.hpp"

/*
handles the wiring and overall behavioral interactions
*/
class MasterControlProgram
{
public:
    MasterControlProgram(SACNReceiver *SACNReceiver, DMXSender *DMXSender)
        : _DMXSender(DMXSender), _SACNReceiver(SACNReceiver) {};

    // END OF LINE.
    void begin()
    {
        _SACNReceiver->begin();
        _DMXSender->begin();
    };

    void netPoll()
    {
        DMXFrame frame;
        if (_SACNReceiver->poll(frame))
        {
            xQueueOverwrite(_frameMailbox, &frame);
        }
    }

    void pushUart()
    {
        DMXFrame frame;
        if (!xQueuePeek(_frameMailbox, &frame, 0))
            return;

        _DMXSender->send(frame);
    };

private:
    DMXSender *_DMXSender{};
    SACNReceiver *_SACNReceiver{};

    QueueHandle_t _frameMailbox{xQueueCreate(1, sizeof(DMXFrame))};
};

#pragma once

#include "interfaces/DMXSender.hpp"
#include "interfaces/SACNReceiver.hpp"
#include "interfaces/DMXMailbox.hpp"

/*
handles the wiring and overall behavioral interactions
*/
class MasterControlProgram
{
public:
    MasterControlProgram(SACNReceiver *SACNReceiver, DMXSender *DMXSender, DMXMailbox *DMXMailbox)
        : _DMXSender(DMXSender), _SACNReceiver(SACNReceiver), _DMXMailbox(DMXMailbox) {};

    // END OF LINE.
    void begin()
    {
        _SACNReceiver->begin();
        _DMXSender->begin();
        _DMXMailbox->begin();
    };

    void netPoll()
    {
        DMXFrame frame;
        if (_SACNReceiver->poll(frame))
        {
            _DMXMailbox->publish(frame);
        }
    }

    void pushUart()
    {
        DMXFrame frame;
        if (!_DMXMailbox->latest(frame))
            return;

        _DMXSender->send(frame);
    };

private:
    DMXSender *_DMXSender{};
    SACNReceiver *_SACNReceiver{};
    DMXMailbox *_DMXMailbox{};
};

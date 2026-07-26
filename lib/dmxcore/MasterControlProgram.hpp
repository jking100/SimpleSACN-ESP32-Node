#pragma once

#include "interfaces/DMXSender.hpp"
#include "interfaces/SACNReceiver.hpp"

/*
handles the wiring and overall behavioral interactions
*/
class MasterControlProgram
{
public:
    MasterControlProgram(SACNReceiver *SACNReceiver, DMXSender *DMXSender)
        : _SACNReceiver(SACNReceiver), _DMXSender(DMXSender) {};

    // END OF LINE.
    void begin()
    {
        _SACNReceiver->begin();
        _DMXSender->begin();
    };

    void tick()
    {
        _DMXSender->send(_SACNReceiver->latest());
    };

private:
    DMXSender *_DMXSender{};
    SACNReceiver *_SACNReceiver{};
};
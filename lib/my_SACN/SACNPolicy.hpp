#pragma once

#include "model/StampedDMXFrame.hpp"

//todo finish me
class SACNPolicy {
public:
    SACNPolicy(bool hold) : _hold(hold) {}

    bool acceptable(const StampedDMXFrame& frame){
        if (_hold ) return true;
        //return (frame.networkRxTime - now() < 2.5s)
    }

private:
    bool _hold; // hold last on data loss
}
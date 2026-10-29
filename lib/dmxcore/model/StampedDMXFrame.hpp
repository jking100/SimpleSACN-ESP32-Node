#pragma once

#include "model/DMXFrame.hpp"

struct StampedDMXFrame
{
    DMXFrame frame;
    uint64_t networkRxTime;
    uint64_t UARTTxStartTime;
    uint64_t UARTTxCompleteTime;
};
#pragma once

#include "model/DMXFrame.hpp"

struct StampedDMXFrame
{
    DMXFrame frame;
    bool streamTerminated; // stream terminated; should not be pushed; see spec
    bool previewData;      // preview frames should not be pushed to fixtures; see spec

    uint64_t networkRxTime;
    uint64_t UARTTxStartTime;
    uint64_t UARTTxCompleteTime;
};
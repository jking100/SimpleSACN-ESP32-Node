#pragma once
#include <cstdint>

#include "sacnData.hpp"

namespace sacnPolicy
{
    enum class timeoutPolicy
    {
        HOLD,
        STOP
    };

    // 2500ms or longer is defined as data loss by e131 6.7.1
    constexpr bool dataLoss(uint32_t nowMs, uint32_t rxMs) { return nowMs - rxMs >= 2500; }

    // reject the same packet or packets up to 19 behind, handle wrap
    inline bool outOfOrder(uint8_t seqNow, uint8_t seqPrior)
    {
        int8_t diff = static_cast<int8_t>(seqNow - seqPrior);
        return (diff <= 0 && diff > -20);
    }

    inline bool relevantPacket(const sacnData &p, uint16_t universe, uint8_t lastSequence, bool alrStarted)
    {
        return p.universe == universe && !(alrStarted && outOfOrder(p.sequenceNumber, lastSequence));
    }

    inline bool shouldTerminate(const sacnData &p)
    {
        return p.isTerminated();
    }

    inline bool shouldNotDisplay(const sacnData &p)
    {
        return p.isPreview();
    }

    inline bool shouldServeFrames(timeoutPolicy tP, uint32_t nowMs, uint32_t lastPacketMs)
    {
        return tP == timeoutPolicy::HOLD || !dataLoss(nowMs, lastPacketMs);
    }
}
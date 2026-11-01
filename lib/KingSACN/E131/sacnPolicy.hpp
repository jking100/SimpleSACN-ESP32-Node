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

    // displayable packets are well formed e131 data packets without stream terminated or preview flags, with correct universe and sequence
    inline bool displayablePacket(const sacnData &p, uint16_t universe, uint8_t lastSequence, bool alrStarted)
    {
        // todo: CID
        // todo: terminated should cut the output of dmx frames
        if (p.isTerminated())
            return false;
        if (p.isPreview())
            return false;

        if (p.universe != universe)
            return false;

        if (alrStarted && outOfOrder(p.sequenceNumber, lastSequence))
            return false;

        return true;
    }

    inline bool shouldServeFrames(timeoutPolicy tP, uint32_t nowMs, uint32_t lastPacketMs)
    {
        return tP == timeoutPolicy::HOLD || !dataLoss(nowMs, lastPacketMs);
    }
}
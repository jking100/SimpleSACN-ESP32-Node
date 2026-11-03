#pragma once
#include <cstdint>
#include <cstring>

#include "sacnData.hpp"

namespace sacnPolicy
{
    enum class timeoutPolicy
    {
        HOLD,
        STOP
    };

    // 2500ms or longer is defined as data loss by e131 6.7.1
    // signed diff: a now slightly before rx (clock read before the lock by another thread) is not loss
    constexpr bool dataLoss(uint32_t nowMs, uint32_t rxMs) { return int32_t(nowMs - rxMs) >= 2500; }

    // reject the same packet or packets up to 19 behind, handle wrap
    inline bool outOfOrder(uint8_t seqNow, uint8_t seqPrior)
    {
        int8_t diff = static_cast<int8_t>(seqNow - seqPrior);
        return (diff <= 0 && diff > -20);
    }

    inline bool sameSource(const sacnData &a, const sacnData &b) { return memcmp(a.CID, b.CID, sizeof(a.CID)) == 0; }

    // relevant if right universe and, once started: same source in order, or a higher priority source,
    // or same priority with a greater cid. cid tiebreak is deterministic, unlike first-come which depends on network jitter
    inline bool relevantPacket(const sacnData &p, uint16_t universe, bool alrStarted, const sacnData &last)
    {
        if (p.universe != universe)
            return false;
        if (!alrStarted)
            return true;

        int cidCmp = memcmp(p.CID, last.CID, sizeof(p.CID));
        if (cidCmp == 0)
            return !outOfOrder(p.sequenceNumber, last.sequenceNumber); // sequence is per source
        if (p.priority != last.priority)
            return p.priority > last.priority;
        return cidCmp > 0;
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
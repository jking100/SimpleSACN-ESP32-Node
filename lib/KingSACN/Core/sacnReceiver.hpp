#pragma once

#include <cstring>

#include "../E131/sacnPolicy.hpp"
#include "../E131/DMXFrame.hpp"

namespace Core
{
    // A sacnReceiver takes in well formed sacn data packets, applies sacn policy rules like data loss timeout... and from that maintains a latest snapshot of a dmx universe
    // it has a couple of states it can step thru like waiting, streaming, data loss depending on what input it has received, these states determine if it will give output
    // it is not threadsafe natively and it is its owners responsibiliy to use it correctly/wire it
    // it also does not track time internally and requires its owner to inform it of time
    class sacnReceiver
    {
    public:
        enum class rxState
        {
            WAITING,   // waiting for a cid to lock onto
            STREAMING, // locked onto a CID and not in data loss
            LOST,      // entered after detecting data loss, stream lock released (CID), will accept from any sender to move back to streaming, output based on timeoutpolicy
        };

        struct rxConfig
        {
            uint16_t universe;
            sacnPolicy::timeoutPolicy lossPolicy;
        };

        struct rxSnapshot
        {
            rxState state;
            DMXFrame currentFrame;
            uint32_t currentFrameRxTime;
            sacnData currentPacket;
        };

        sacnReceiver(rxConfig c) : _config(c) { _snapshot.state = rxState::WAITING; }

        bool onSacnPacket(sacnData p, uint32_t timeNowMs);

        bool getCurrentFrame(DMXFrame *out, uint32_t timeNowMs);

        rxState getCurrentState() { return _snapshot.state; }

    private:
        rxConfig _config{};
        rxSnapshot _snapshot{};

        void update(uint32_t timeNowMs)
        {
            if (checkState(rxState::STREAMING) && sacnPolicy::dataLoss(timeNowMs, _snapshot.currentFrameRxTime))
                _snapshot.state = rxState::LOST;
        }

        constexpr bool checkState(rxState s)
        {
            return s == _snapshot.state;
        }
    };

    inline bool sacnReceiver::onSacnPacket(sacnData p, uint32_t timeNowMs)
    {
        update(timeNowMs);

        // cid lock on to source of last good packet
        if (checkState(rxState::STREAMING) && memcmp(p.CID, _snapshot.currentPacket.CID, sizeof(p.CID)) != 0)
            return false;

        if (!sacnPolicy::relevantPacket(p, _config.universe, _snapshot.currentPacket.sequenceNumber, checkState(rxState::STREAMING)))
            return false;

        // todo: in lost rxState should we only terminate from last cid(relates to hold frame behavior)
        if (sacnPolicy::shouldTerminate(p))
        {
            _snapshot.state = rxState::WAITING;
            return false;
        }

        // todo: fix this: we will go into data loss even if we are receiving preview frames
        if (sacnPolicy::shouldNotDisplay(p))
            return false;

        _snapshot.currentPacket = p;
        _snapshot.currentFrameRxTime = timeNowMs;

        // todo this is sacn knowledge the +1, and the 512 less so but still
        memcpy(_snapshot.currentFrame.data, p.propertyValues + 1, 512);

        if (checkState(rxState::LOST) || checkState(rxState::WAITING))
            _snapshot.state = rxState::STREAMING;

        return true;
    }

    inline bool sacnReceiver::getCurrentFrame(DMXFrame *out, uint32_t timeNowMs)
    {
        update(timeNowMs);

        if (checkState(rxState::WAITING))
            return false;

        if (!sacnPolicy::shouldServeFrames(_config.lossPolicy, timeNowMs, _snapshot.currentFrameRxTime))
            return false;

        *out = _snapshot.currentFrame;
        return true;
    }
}
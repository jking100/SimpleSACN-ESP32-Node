#pragma once

#include <cstdint>
#include <cstddef>
#include <mutex>

// todo: these are brittle
#include "../E131/sacnData.hpp"
#include "../E131/sacnParse.hpp"
#include "../E131/sacnPolicy.hpp"
#include "../E131/DMXFrame.hpp"

// single universe sacnNode with configurable data loss behavior, and a basic in/out api
// given raw bytes, this node will process and serve frames of dmx data according to sacn operation policy
// its input and output functions are thread safe, output/imput may wait on a copy of sacnData + accoutrements
//
//
// seems liks a lot of concerns in this class
// worrying about how to serve the data safely for two threads
// maybe this could be some sort of thinner state machine that just coordinates between the sacn policy and its current stage plus some sort of storage mechanism
class singleUniverseNode
{
public:
    enum class state
    {
        WAITING,   // waiting for a cid to lock onto
        STREAMING, // locked onto a CID and not in data loss
        LOST,      // entered after detecting data loss, stream lock released (CID), will accept from any sender to move back to streaming, output based on timeoutpolicy
    };

    struct nodeConfig
    {
        uint16_t universe;
        sacnPolicy::timeoutPolicy timeoutPolicy;
    };

    struct nodeStatus
    {
        state nodeState;
        sacnData lastPacket;
        uint32_t lastPacketTimeMs;
        DMXFrame lastFrame;
    };

    singleUniverseNode(nodeConfig c)
        : _config(c)
    {
        _status.nodeState = state::WAITING;
    }

    // input
    bool processE131Packet(const uint8_t *buf, size_t len, uint32_t timeNow);

    // return the last processed packet
    bool getLastPacket(sacnData *out);

    // return the last valid frame
    bool getLastFrame(DMXFrame *out, uint32_t timeNow);

    state getState() const
    {
        return _status.nodeState;
    }

private:
    nodeConfig _config{};
    nodeStatus _status{};

    std::mutex guard{};
};

inline bool singleUniverseNode::processE131Packet(const uint8_t *buf, size_t len, uint32_t timeNow)
{
    sacnData tmp{};
    if (!sacnParse::parse(buf, len, tmp))
        return false;

    std::lock_guard<std::mutex> lock(guard);

    if (getState() == state::STREAMING && sacnPolicy::dataLoss(timeNow, _status.lastPacketTimeMs))
        _status.nodeState = state::LOST;

    if (tmp.isTerminated())
    {
        _status.nodeState = state::WAITING;
        return false;
    }

    // cid lock on to last good packet
    if (getState() == state::STREAMING && memcmp(tmp.CID, _status.lastPacket.CID, sizeof(tmp.CID)) != 0)
        return false;

    if (!sacnPolicy::displayablePacket(tmp, _config.universe, _status.lastPacket.sequenceNumber, (getState() == state::STREAMING)))
        return false;

    _status.lastPacket = tmp;
    _status.lastPacketTimeMs = timeNow;

    // todo this is sacn knowledge the +1, and the 512 less so but still
    memcpy(_status.lastFrame.data, tmp.propertyValues + 1, 512);

    if (getState() == state::LOST || getState() == state::WAITING)
        _status.nodeState = state::STREAMING;

    return true;
}

inline bool singleUniverseNode::getLastPacket(sacnData *out)
{
    std::lock_guard<std::mutex> lock(guard);

    if (getState() == state::WAITING)
        return false;

    *out = _status.lastPacket;

    return true;
}

inline bool singleUniverseNode::getLastFrame(DMXFrame *out, uint32_t timeNow)
{
    std::lock_guard<std::mutex> lock(guard);

    if (getState() == state::WAITING)
        return false;

    // if we check last and its too old we can move into lost here

    if (sacnPolicy::dataLoss(timeNow, _status.lastPacketTimeMs))
        _status.nodeState = state::LOST;

    if (!sacnPolicy::shouldServeFrames(_config.timeoutPolicy, timeNow, _status.lastPacketTimeMs))
        return false;

    *out = _status.lastFrame;
    return true;
}
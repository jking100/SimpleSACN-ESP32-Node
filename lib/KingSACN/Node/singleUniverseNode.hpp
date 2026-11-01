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
// ideas: state machine to move thru listening -> dataloss (state mentioned in spec) -> listening -> configuring -> listening ... ???
// how do i log statistics of function (timing/fps/...)??
//
// seems liks a lot of concerns in this class
// --- applying sacn policy to make decisions (dataloss, universe, sequence)
// worrying about how to serve the data safely for two threads
// maybe this could be some sort of thinner state machine that just coordinates between the sacn policy and its current stage plus some sort of storage mechanism
class singleUniverseNode
{
public:
    enum class state
    {
        WAITING,
        TERMINATED,
        DATALOSS,
        STOPPED,
        STREAMING
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

    singleUniverseNode(nodeConfig c);

    // input
    bool processE131Packet(const uint8_t *buf, size_t len, uint32_t timeNow);

    // return the last processed packet
    bool getLastPacket(sacnData *out);

    // return the last valid frame
    bool getLastFrame(DMXFrame *out, uint32_t timeNow);

    // timing, latency etc
    // todo: does this even belong here?
    void getStatistics();

private:
    nodeConfig _config{};
    nodeStatus _status{};

    std::mutex guard{};
};

inline singleUniverseNode::singleUniverseNode(nodeConfig c)
    : _config(c)
{
}

inline bool singleUniverseNode::processE131Packet(const uint8_t *buf, size_t len, uint32_t timeNow)
{
    sacnData tmp{};
    if (!sacnParse::parse(buf, len, tmp))
        return false;

    std::lock_guard<std::mutex> lock(guard);

    if (!sacnPolicy::displayablePacket(tmp, _config.universe, _status.lastPacket.sequenceNumber, (_status.nodeState == state::STREAMING)))
        return false;

    _status.lastPacket = tmp;
    _status.lastPacketTimeMs = timeNow;
    memcpy(_status.lastFrame.data, tmp.propertyValues + 1, 512);

    if ((_status.nodeState != state::STREAMING))
        _status.nodeState = state::STREAMING;

    return true;
}

inline bool singleUniverseNode::getLastPacket(sacnData *out)
{
    std::lock_guard<std::mutex> lock(guard);

    if (_status.nodeState != state::STREAMING)
        return false;

    *out = _status.lastPacket;

    return true;
}

inline bool singleUniverseNode::getLastFrame(DMXFrame *out, uint32_t timeNow)
{
    std::lock_guard<std::mutex> lock(guard);

    if (_status.nodeState != state::STREAMING)
        return false;

    if (!sacnPolicy::shouldServeFrames(_config.timeoutPolicy, timeNow, _status.lastPacketTimeMs))
        return false;

    *out = _status.lastFrame;
    return true;
}

inline void singleUniverseNode::getStatistics()
{
    return;
}
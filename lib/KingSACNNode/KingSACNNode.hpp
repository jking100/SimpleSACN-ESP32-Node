#pragma once

#include <cstdint>
#include <cstddef>
#include <mutex>

// todo: these are brittle
#include "../KingSACN/sacnData.hpp"
#include "../model/DMXFrame.hpp"

// single universe sacnNode with configurable data loss behavior, and a basic in/out api
// given raw bytes, this node will process and serve frames of dmx data
// its input and output functions are thread safe, output/imput may wait on a copy of sacnData + accoutrements
//
// ideas: state machine to move thru listening -> dataloss (state mentioned in spec) -> listening -> configuring -> listening ... ???
// how do i log statistics of function (timing/fps/...)??
class KingSACNNode
{
public:
    // should this logic actually live at the layer that owns and orchastrates one of these?
    enum class state
    {
        LISTENING,
        DATALOSS,
        CONFIG
    };

    enum class timeoutPolicy
    {
        HOLD,
        STOP
    };

    KingSACNNode(int16_t universe, timeoutPolicy tPolicy);

    // input
    bool processE131Packet(const uint8_t *buf, size_t len, uint64_t timeNow);

    // return the last processed packet
    bool getLastPacket(sacnData *out);

    // return the last valid frame
    bool getLastFrame(DMXFrame *out, uint64_t timeNow);

    // timing, latency etc
    // todo: does this even belong here?
    void getStatistics();

private:
    // check packet against policy/config
    bool validatePacket(const sacnData &p);

    uint16_t _universe{};
    timeoutPolicy _tPolicy{};

    // can be used by two seperate threads
    sacnData lastValidPacket{};
    DMXFrame lastValidFrame{};
    uint64_t lastValidPacketTime{};

    bool transmissionStarted{};

    std::mutex guard{};
};

KingSACNNode::KingSACNNode(int16_t univserse, timeoutPolicy tPolicy)
    : _universe(univserse), _tPolicy(tPolicy)
{
}

bool KingSACNNode::processE131Packet(const uint8_t *buf, size_t len, uint64_t timeNow)
{
    sacnData tmp{};
    if (!parse(buf, len, tmp))
        return false;

    if (tmp.universe != _universe)
        return false;

    std::lock_guard<std::mutex> lock(guard);

    lastValidPacket = tmp;
    memcpy(lastValidFrame.data, lastValidPacket.propertyValues + 1, 512);
    lastValidPacketTime = timeNow;

    if (!transmissionStarted)
        transmissionStarted = true;

    return true;
}

bool KingSACNNode::getLastPacket(sacnData *out)
{
    std::lock_guard<std::mutex> lock(guard);

    if (!transmissionStarted)
        return false;

    *out = lastValidPacket;

    return true;
}

bool KingSACNNode::getLastFrame(DMXFrame *out, uint64_t timeNow)
{
    std::lock_guard<std::mutex> lock(guard);

    if (!transmissionStarted)
        return false;

    if (_tPolicy == timeoutPolicy::STOP && timeNow - lastValidPacketTime >= 2500)
    {
        memset(out->data, 0, sizeof(out->data));
        return false;
    }

    *out = lastValidFrame;
    return true;
}

void KingSACNNode::getStatistics()
{
    return;
}
#pragma once

#include <cstdint>
#include <cstddef>
#include <mutex>

// todo: these are brittle
#include "../E131/sacnData.hpp"
#include "../E131/sacnParse.hpp"
#include "../E131/DMXFrame.hpp"

#include "../Core/sacnReceiver.hpp"

// single universe sacnNode with configurable data loss behavior, and a basic in/out api
// given raw bytes, this node will process and serve frames of dmx data according to sacn operation policy
// its input and output functions are thread safe, output/imput may wait on a copy of sacnData + accoutrements
//
//
// seems liks a lot of concerns in this class
// worrying about how to serve the data safely for two threads
// maybe this could be some sort of thinner state machine that just coordinates between the sacn policy and its current stage plus some sort of storage mechanism
//
// 11/1: maybe the node can be split such that a node owns a sacnreceiver. a sacnreceiver is not threadaware, but it owns application of sacn policy and can give out dmxframes
//      the sacn node ensures thread safety when operating the sacnreceiver, and is the public api for take these bytes and give me frames.
//      i think this could break the current concerns of this class up, and allow for more direct testing/self documentation of the code
//
//

namespace Node
{
    class safeNode
    {
    public:
        safeNode(Core::sacnReceiver::rxConfig c) : _rx(c) {}

        bool onPacket(const uint8_t *buf, size_t len, uint32_t timeNowMs);

        bool latestFrame(DMXFrame *out, uint32_t timeNowMs);

    private:
        std::mutex _guard{};
        Core::sacnReceiver _rx;
    };

    inline bool safeNode::onPacket(const uint8_t *buf, size_t len, uint32_t timeNowMs)
    {
        sacnData tmp;
        if (!sacnParse::parse(buf, len, tmp))
            return false;

        const std::lock_guard<std::mutex> lock(_guard);

        return _rx.onSacnPacket(tmp, timeNowMs);
    }

    inline bool safeNode::latestFrame(DMXFrame *out, uint32_t timeNowMs)
    {
        const std::lock_guard<std::mutex> lock(_guard);

        return _rx.getCurrentFrame(out, timeNowMs);
    }
}
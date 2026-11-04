#pragma once

#include <cstdint>
#include <cstddef>
#include <mutex>

// todo: these are brittle
#include "../E131/sacnData.hpp"
#include "../E131/sacnParse.hpp"
#include "../E131/DMXFrame.hpp"

#include "../Core/sacnReceiver.hpp"

namespace Node
{
    using Config = Core::sacnReceiver::rxConfig;
    using DataLossPolicy = sacnPolicy::timeoutPolicy;

    // single universe sacnNode with configurable data loss behavior, and a basic in/out api
    // given raw bytes, this node will process and serve frames of dmx data according to sacn operation policy
    // its input and output functions are thread safe, output/imput may wait on a copy of sacnData + accoutrements
    class safeNode
    {
    public:
        safeNode(Config c) : _rx(c) {}

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
#pragma once

// parse bytes into sacn packet
#include <cassert>
#include <cstring>

#include "SACNPacket.hpp"

namespace SACNParser
{
    using SACNByteOrder::ntoh16;
    using SACNByteOrder::ntoh32;

    enum class parseStatus
    {
        FAILED_PREAMBLE,
        FAILED_POSTAMBLE,
        FAILED_ACN_ID,
        FAILED_ROOT_VECTOR,
        FAILED_FRAMING_VECTOR,
        FAILED_CHANNEL_START_CODE,
        INCORRECT_BUFFER_SIZE,
        GOOD
    };

    inline const char *parseStatus_to_string(parseStatus s)
    {
        switch (s)
        {
        case parseStatus::FAILED_PREAMBLE:
            return "FAILED_PREAMBLE";
        case parseStatus::FAILED_POSTAMBLE:
            return "FAILED_POSTAMBLE";
        case parseStatus::FAILED_ACN_ID:
            return "FAILED_ACN_ID";
        case parseStatus::FAILED_ROOT_VECTOR:
            return "FAILED_ROOT_VECTOR";
        case parseStatus::FAILED_FRAMING_VECTOR:
            return "FAILED_FRAMING_VECTOR";
        case parseStatus::FAILED_CHANNEL_START_CODE:
            return "FAILED_CHANNEL_START_CODE";
        case parseStatus::INCORRECT_BUFFER_SIZE:
            return "INCORRECT_BUFFER_SIZE";
        case parseStatus::GOOD:
            return "GOOD";
        }
        return "Unknown";
    }

    /* Buffer must be 638 bytes */
    inline parseStatus parseToSACN(const uint8_t *buf, int16_t buf_size, sacnPacket *out)
    {
        if (buf_size != SACN_PACKET_SIZE)
            return parseStatus::INCORRECT_BUFFER_SIZE;

        sacnPacket tmp{}; // use a tmp before we know if the packet is good, dont touch caller's packets until we are sure
        memcpy(&tmp, buf, SACN_PACKET_SIZE);

        // validate
        {
            // watch out for network byte ordering
            //  E1.31 table 5-4
            if (ntoh16(tmp.preambleSize) != 0x0010)
                return parseStatus::FAILED_PREAMBLE;
            if (ntoh16(tmp.postambleSize) != 0x0000)
                return parseStatus::FAILED_POSTAMBLE;
            if (memcmp(tmp.ACNPacketIdentifier, ACNPacketIdentifier, ACN_PACKET_ID_SIZE) != 0)
                return parseStatus::FAILED_ACN_ID;
            if (ntoh32(tmp.rootVector) != VECTOR_ROOT_E131_DATA) // check root validity
                return parseStatus::FAILED_ROOT_VECTOR;
            if (ntoh32(tmp.framingVector) != VECTOR_E131_DATA_PACKET) // check frame validity
                return parseStatus::FAILED_FRAMING_VECTOR;
            if (tmp.propertyValues[0] != 0x0) // we dont care about non zero start codes now, so just drop em
                return parseStatus::FAILED_CHANNEL_START_CODE;
        }

        memcpy(out, &tmp, SACN_PACKET_SIZE); // packet good, mutate callers packet
        return parseStatus::GOOD;
    }

    // host order reads of the multi byte fields
    inline uint16_t universe(const sacnPacket &p) { return ntoh16(p.universe); }
    inline uint16_t slotCount(const sacnPacket &p) { return ntoh16(p.propertyValueCount); } // start code + channels, 1 - 513

    class SequenceTracker
    {
    public:
        bool accept(uint8_t seq)
        {
            if (!initialized)
            {
                initialized = true;
                lastSequence = seq;
                return true;
            }

            // E131 sequence validation
            //  E1.31 6.7.2: out of order if 0 to 19 behind the last accepted packet. int8 cast handles 255 -> 0 wrap
            int8_t diff = static_cast<int8_t>(seq - lastSequence);
            if (diff <= 0 && diff > -20)
            {
                return false;
            }

            lastSequence = seq;
            return true;
        }

    private:
        uint8_t lastSequence{};
        bool initialized{};
    };

    enum class validateStatus
    {
        WRONG_UNIVERSE,
        OUT_OF_ORDER,
        GOOD
    };

    inline const char *validateStatus_to_string(validateStatus s)
    {
        switch (s)
        {
        case validateStatus::WRONG_UNIVERSE:
            return "WRONG_UNIVERSE";
        case validateStatus::OUT_OF_ORDER:
            return "OUT_OF_ORDER";
        case validateStatus::GOOD:
            return "GOOD";
        }
        return "Unknown";
    }

    inline validateStatus packetValidator(sacnPacket const &p, const uint16_t *universe, SequenceTracker *_tracker)
    {
        if (universe && SACNParser::universe(p) != *universe)
            return validateStatus::WRONG_UNIVERSE;
        if (_tracker && !_tracker->accept(p.sequenceNumber))
            return validateStatus::OUT_OF_ORDER;

        return validateStatus::GOOD;
    }

}
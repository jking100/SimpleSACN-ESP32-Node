#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

#include "sacnData.hpp"

namespace sacnParse
{

    namespace util
    {
        // Swap byte order in a 16 bit number
        constexpr uint16_t swap16(uint16_t d)
        {
            return ((d >> 8) | (d << 8));
        }

        // Swap byte order in a 32 bit number
        constexpr uint32_t swap32(uint32_t d)
        {
            return ((d >> 24) | (d >> 8 & 0x0000FF00) | (d << 8 & 0x00FF0000) | (d << 24));
        }

        // Read 2 bytes from a buffer in network order into host order
        //  @param buf buffer of bytes in network order
        //  @param offset starting offset, must be no more than sizeof(buf) - 2
        constexpr uint16_t read16ntoh(const uint8_t *buf, size_t offset)
        {
            return (uint16_t(buf[offset]) << 8) | (buf[offset + 1]);
        }

        // Read 4 bytes from a buffer in network order into host order
        //  @param buf buffer of bytes in network order
        //  @param offset starting offset, must be no more than sizeof(buf) - 4
        constexpr uint32_t read32ntoh(const uint8_t *buf, size_t offset)
        {
            return (uint32_t(buf[offset]) << 24) | (uint32_t(buf[offset + 1]) << 16) | (uint32_t(buf[offset + 2]) << 8) | (buf[offset + 3]);
        }
    }

    // attempts to parse a buffer into an e131 data packet;
    // will mutate the callers packet, even on failure;
    // will immediately fail on: buffer too small, bad preamble/postamble, incorrect acnPacketId,
    // root/framing/dmp vector, universe outside 1-63999,
    // dmp address fields, property value count / len mismatch, non-zero start code
    // @return parse into data packet succesful
    inline bool parse(const uint8_t *buf, size_t len, sacnData &out)
    {
        using namespace util;
        namespace off = constants::offsets;

        if (len < off::startCode + 1)
            // buffer is smaller than e131 data packet header + 1 channel
            return false;

        // root
        out.preambleSize = read16ntoh(buf, off::preamble);
        out.postambleSize = read16ntoh(buf, off::postamble);
        if (out.preambleSize != constants::PREAMBLE_SIZE || out.postambleSize != constants::POSTAMBLE_SIZE)
            return false;
        memcpy(out.ACNPacketIdentifier, buf + off::acnId, 12);
        for (size_t i{}; i < 12; ++i)
        {
            if (out.ACNPacketIdentifier[i] != constants::ACNPacketIdentifier[i])
                return false;
        }
        out.rootFlagsAndLength = read16ntoh(buf, off::rootFlagsAndLength);
        out.rootVector = read32ntoh(buf, off::rootVector);
        if (out.rootVector != constants::VECTOR_ROOT_E131_DATA)
            return false;
        memcpy(out.CID, buf + off::cid, 16);

        // framing
        out.framingFlagsAndLength = read16ntoh(buf, off::framingFlagsAndLength);
        out.framingVector = read32ntoh(buf, off::framingVector);
        if (out.framingVector != constants::VECTOR_E131_DATA_PACKET)
            return false;
        memcpy(out.sourceName, buf + off::sourceName, 64);
        out.priority = buf[off::priority];
        out.synchronizationAddress = read16ntoh(buf, off::synchronizationAddress);
        out.sequenceNumber = buf[off::sequenceNumber];
        out.options = buf[off::options];
        out.universe = read16ntoh(buf, off::universe);
        // todo: does this really count as a parse failure? i dont think so, should move to node logic area
        if (out.universe < constants::MIN_UNIVERSE || out.universe > constants::MAX_UNIVERSE)
            return false;

        // dmp
        out.dmpFlagsAndLength = read16ntoh(buf, off::dmpFlagsAndLength);
        out.dmpVector = buf[off::dmpVector];
        out.addressTypeAndDataType = buf[off::addressTypeAndDataType];
        out.firstPropertyAddress = read16ntoh(buf, off::firstPropertyAddress);
        out.addressIncrement = read16ntoh(buf, off::addressIncrement);
        if (out.dmpVector != constants::VECTOR_DMP_SET_PROPERTY || out.addressTypeAndDataType != constants::ADDRESS_TYPE_AND_DATA_TYPE ||
            out.firstPropertyAddress != constants::FIRST_PROPERTY_ADDRESS || out.addressIncrement != constants::ADDRESS_INCREMENT)
            return false;
        out.propertyValueCount = read16ntoh(buf, off::propertyValueCount);
        if (out.propertyValueCount > constants::MAX_PROPERTY_VALUE_COUNT || len != off::startCode + out.propertyValueCount)
            return false;
        if (buf[off::startCode] != constants::NULL_START_CODE)
            return false;
        memcpy(out.propertyValues, buf + off::startCode, out.propertyValueCount);
        memset(out.propertyValues + out.propertyValueCount, 0, constants::MAX_PROPERTY_VALUE_COUNT - out.propertyValueCount); // zero out the rest

        return true;
    }

    // todo: sync this to covered test cases and implement
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
}
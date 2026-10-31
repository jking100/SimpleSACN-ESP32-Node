#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace constants
{
    // E1.31 appendix a
    constexpr uint32_t VECTOR_E131_DATA_PACKET{0x00000002};
    constexpr uint32_t VECTOR_ROOT_E131_DATA{0x00000004};
    constexpr uint32_t E131_NETWORK_DATA_LOSS_TIMEOUT_MS{2500};
    constexpr uint16_t ACN_SDT_MULTICAST_PORT{5568};

    constexpr uint8_t ACNPacketIdentifier[12] = {0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00};

    namespace offsets
    {
        // root layer
        constexpr size_t preamble{0};
        constexpr size_t postamble{2};
        constexpr size_t acnId{4};
        constexpr size_t rootFlagsAndLength{16};
        constexpr size_t rootVector{18};
        constexpr size_t cid{22};
        // framing layer
        constexpr size_t framingFlagsAndLength{38};
        constexpr size_t framingVector{40};
        constexpr size_t sourceName{44};
        constexpr size_t priority{108};
        constexpr size_t synchronizationAddress{109};
        constexpr size_t sequenceNumber{111};
        constexpr size_t options{112};
        constexpr size_t universe{113};
        // dmp layer
        constexpr size_t dmpFlagsAndLength{115};
        constexpr size_t dmpVector{117};
        constexpr size_t addressTypeAndDataType{118};
        constexpr size_t firstPropertyAddress{119};
        constexpr size_t addressIncrement{121};
        constexpr size_t propertyValueCount{123};
        constexpr size_t startCode{125}; // propertyValues[0]; also the header size
    }
}

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

    // Read 2 bytes from a buffer in network order into little endian hardware order
    //  @param buf buffer of bytes in network order
    //  @param offset starting offset, must be no more than sizeof(buf) - 2
    constexpr uint16_t read16ntoh(const uint8_t *buf, uint16_t offset)
    {
        return (buf[offset] << 8) | (buf[offset + 1]);
    }

    // Read 4 bytes from a buffer in network order into little endian hardware order
    //  @param buf buffer of bytes in network order
    //  @param offset starting offset, must be no more than sizeof(buf) - 4
    constexpr uint32_t read32ntoh(const uint8_t *buf, uint16_t offset)
    {
        return (buf[offset] << 24) | (buf[offset + 1] << 16) | (buf[offset + 2] << 8) | (buf[offset + 3]);
    }
}

// 4.1 E1.31 Data Packet
// we care about: packetid, root vector, framing vector, property values count <= 513, buf size > 126 [ahve to have propertyvalue[0] at least]
struct sacnData
{
    // Root Layer                    // Field Description; Field Contents
    uint16_t preambleSize;           // Define RLP Preamble Size; 0x0010
    uint16_t postambleSize;          // RLP Post-amble Size; 0x0000
    uint8_t ACNPacketIdentifier[12]; // Identifies this packet as E1.17; 0x41 0x53 0x43 0x2d 0x45 0x31 0x2e 0x31 0x37 0x00 0x00 0x00
    uint16_t rootFlagsAndLength;     // Protocol flags and length; Low 12 bits = PDU length, High 4 bits = 0x7
    uint32_t rootVector;             // Identifies RLP Data as 1.31 Protocol PDU; VECTOR_ROOT_E131_DATA
    uint8_t CID[16];                 // Sender's CID; Senders unique ID
    // Framing Layer
    uint16_t framingFlagsAndLength;  // Protocol flags and length; Low 12 bits = PDU length, High 4 bits = 0x7
    uint32_t framingVector;          // Identifies 1.31 data as DMP Protocol PDU; VECTOR_E131_DATA_PACKET
    uint8_t sourceName[64];          // User Assigned Name of Source; UTF-8 encoded string, null-terminated
    uint8_t priority;                // Data priority if multiple sources; 0-200, default of 100
    uint16_t synchronizationAddress; // Universe address on which sync packets will be sent; Universe sync packets are transmitted on
    uint8_t sequenceNumber;          // Sequence Number; To detect duplicate or out of order packets
    uint8_t options;                 // Options Flags; Bit 7 = Preview_Data, Bit 6 = Stream_Terminated, Bit 5 = Force_Synchronization
    uint16_t universe;               // Universe Number; Identifier for a distinct stream of DMX512-A [DMX] Data
    // DMP Layer
    uint16_t dmpFlagsAndLength;     // Protocol flags and length; Low 12 bits = PDU length, High 4 bits = 0x7
    uint8_t dmpVector;              // Identifies DMP Set Property Message PDU; VECTOR_DMP_SET_PROPERTY (from [ACN-DMP] 13.2)
    uint8_t addressTypeAndDataType; // Identifies format of address and data; 0xa1
    uint16_t firstPropertyAddress;  // Indicates DMX512-A START Code is at DMP address 0; 0x0000
    uint16_t addressIncrement;      // Indicates each property is 1 octet; 0x0001
    uint16_t propertyValueCount;    // Indicates 1+ the number of slots in packet; 0x0001 -- 0x0201
    uint8_t propertyValues[513];    // DMX512-A START Code + data; START Code + Data (slot 0 is START Code, slots 1-512 are channels)
};

// attempts to parse a buffer into an e131 data packet;
// will mutate the callers packet;
// will immediately fail on: buffer too small, incorrect acnPacketId or
// root/framing vector, too many property values
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

    // dmp
    out.dmpFlagsAndLength = read16ntoh(buf, off::dmpFlagsAndLength);
    out.dmpVector = buf[off::dmpVector];
    out.addressTypeAndDataType = buf[off::addressTypeAndDataType];
    out.firstPropertyAddress = read16ntoh(buf, off::firstPropertyAddress);
    out.addressIncrement = read16ntoh(buf, off::addressIncrement);
    out.propertyValueCount = read16ntoh(buf, off::propertyValueCount);
    if (out.propertyValueCount > 513 || len != off::startCode + out.propertyValueCount)
        return false;
    memcpy(out.propertyValues, buf + off::startCode, out.propertyValueCount);
    memset(out.propertyValues + out.propertyValueCount, 0, 513 - out.propertyValueCount); // zero out the rest

    return true;
}

constexpr bool isTerminated(const sacnData &p)
{
    return p.options & 0b01000000;
}

constexpr bool isPreview(const sacnData &p)
{
    return p.options & 0b10000000;
}

#pragma once

#include <cstdint>
#include <cstddef>

namespace constants
{
    // E1.31 appendix a
    constexpr uint32_t VECTOR_E131_DATA_PACKET{0x00000002};
    constexpr uint32_t VECTOR_ROOT_E131_DATA{0x00000004};
    constexpr uint8_t VECTOR_DMP_SET_PROPERTY{0x02};
    constexpr uint32_t E131_NETWORK_DATA_LOSS_TIMEOUT_MS{2500};
    constexpr uint16_t ACN_SDT_MULTICAST_PORT{5568};

    // required field values, E1.31 data packet
    constexpr uint16_t PREAMBLE_SIZE{0x0010};
    constexpr uint16_t POSTAMBLE_SIZE{0x0000};
    constexpr uint8_t ACNPacketIdentifier[12] = {0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00};
    constexpr uint8_t ADDRESS_TYPE_AND_DATA_TYPE{0xa1};
    constexpr uint16_t FIRST_PROPERTY_ADDRESS{0x0000};
    constexpr uint16_t ADDRESS_INCREMENT{0x0001};
    constexpr uint16_t MAX_PROPERTY_VALUE_COUNT{513}; // start code + 512 channels
    constexpr uint8_t NULL_START_CODE{0x00};          // DMX level data
    constexpr uint16_t MIN_UNIVERSE{1};               // 0 and 64000+ are reserved
    constexpr uint16_t MAX_UNIVERSE{63999};

    // options bits
    constexpr uint8_t OPTIONS_PREVIEW_DATA{0b10000000};      // bit 7
    constexpr uint8_t OPTIONS_STREAM_TERMINATED{0b01000000}; // bit 6

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

// 4.1 E1.31 Data Packet
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
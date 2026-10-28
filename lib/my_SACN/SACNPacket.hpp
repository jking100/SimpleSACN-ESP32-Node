#pragma once

#include <cstdint>
#include <cstddef>

/*
network (big endian) to host order
*/
namespace SACNByteOrder
{
    static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "SACNByteOrder assumes a little endian host (esp32 + x86 both are)");

    inline constexpr uint16_t ntoh16(uint16_t v) { return __builtin_bswap16(v); }
    inline constexpr uint32_t ntoh32(uint32_t v) { return __builtin_bswap32(v); }
}

constexpr size_t SACN_PACKET_SIZE{638};
constexpr size_t ACN_PACKET_ID_SIZE{12};

// E1.17 - ACN Packet Identifier
constexpr uint8_t ACNPacketIdentifier[12] = {0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00};

// E1.31 - appendix a
constexpr uint32_t VECTOR_ROOT_E131_DATA{0x00000004};
constexpr uint32_t VECTOR_E131_DATA_PACKET{0x00000002};

constexpr uint16_t ACN_SDT_MULTICAST_PORT{5568};
// E131_NETWORK_DATA_LOSS_TIMEOUT = 2.5 seconds

/*
a packet of sACN protocol data, contains 512 bytes of beautiful DMX channels
watch out for network byte order on members larger than a byte, network order is big endian,
i.e. universe of 1 sent as a 2 byte int will be 0x0100 aka <int>256 in hardware order
based on ANSI E1.31 - 2025 spec, table 4 - 1
*/
union sacnPacket
{
    struct
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

    } __attribute__((packed)); // pack it to ensure padding is not added to byte align the members >:(

    uint8_t raw[SACN_PACKET_SIZE];
};

static_assert(sizeof(sacnPacket) == SACN_PACKET_SIZE, "sacnPacket is not 638 bytes, sad :(\n");

/*
notes: ANSI E1.31 - 2025 defines that sacn multicasting is done on 239.255.xxx.xxx, not inclusing 239.255.255.xxx, those top 256 addrs are reserved
see table 9 - 10
239.255.universeHiByte.universeLowByte aka 239.255.1.0 is universe 256 and .0.1 is universe 1; 65535 universe limit
*/
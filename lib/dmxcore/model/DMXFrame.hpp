#pragma once

#include <cstdint>

/*
Representation of a full dmx universe
*/
struct DMXFrame
{
    // first byte is 0x00 always, 512 channels of 0-255 remain
    uint8_t data[513]{};
};
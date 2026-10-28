#pragma once

#include <cstring>

#include "interfaces/SACNReceiver.hpp"

#include "model/DMXFrame.hpp"

class FakeSACNReceiver : public SACNReceiver
{
public:
    FakeSACNReceiver()
    {
        memset(frame.data + 1, (255 / 2), 512); // leave first byte alone, set all channels to half value
    }

    bool begin() override
    {
        return true;
    }

    bool poll(DMXFrame &out) override
    {
        memcpy(&out, frame.data, 513);
        return true;
    }

private:
    DMXFrame frame{};
};
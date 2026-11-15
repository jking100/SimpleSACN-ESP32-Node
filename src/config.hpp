#pragma once

#include <cstdint>
#include <SimpleSACN.hpp>

namespace config
{
    constexpr bool printDebug{true}; // prints stack info of the wifi and dmx tasks if having crashing issues

    constexpr int uartBus{1};
    constexpr int txPin{39}; // uart tx pin
    constexpr uint16_t sacnPort{5568};

    constexpr SimpleSACN::Node::Config rx{1, SimpleSACN::Node::DataLossPolicy::HOLD};
    // constexpr SimpleSACN::Node::Config rx{1, SimpleSACN::Node::DataLossPolicy::STOP};
}

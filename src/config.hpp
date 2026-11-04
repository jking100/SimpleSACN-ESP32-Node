#pragma once

#include <cstdint>
#include "KingSACN/Node/safeNode.hpp"

namespace config
{
    constexpr bool printDebug{true}; // prints stack info of the wifi and dmx tasks if having crashing issues

    constexpr int uartBus{1};
    constexpr int txPin{39}; // uart tx pin
    constexpr uint16_t sacnPort{5568};

    constexpr Node::Config rx{1, Node::DataLossPolicy::HOLD};
    // constexpr Node::Config rx{1, Node::DataLossPolicy::STOP};
}

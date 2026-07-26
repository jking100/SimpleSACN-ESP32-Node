#pragma once

// someweisguy/esp_dmx
#include "esp_dmx.h"
// esp dmx does not include this file itself(seems to assume what framework its on), transient include problem with the DMX_CONFIG_DEFUALT macro
#include "esp_intr_alloc.h"

#include "model/DMXFrame.hpp"
#include "interfaces/DMXSender.hpp"

class ESP_DMXSender : public DMXSender
{
public:
    /*
    a DMXSender implementation powered by the someweisguy/esp_dmx library
    @param txPin what pin to use for UART out
    @param uartBus which UART peripheral to use
    */
    ESP_DMXSender(int txPin, int uartBus) : _txPin(txPin), _uartBus(uartBus) {}

    void begin() override
    {
        // demo code someweisguy/esp_dmx
        if (_hasBeenInit)
            return;

        // First, use the default DMX configuration...
        dmx_config_t config = DMX_CONFIG_DEFAULT;

        // ...declare the driver's DMX personalities...
        const int personality_count = 1;
        dmx_personality_t personalities[] = {
            {1, "Default Personality"}};

        // ...install the DMX driver...
        dmx_driver_install(_uartBus, &config, personalities, personality_count);

        // target the pins and uart periph
        dmx_set_pin(_uartBus, _txPin, DMX_PIN_NO_CHANGE /*rx_pin*/, DMX_PIN_NO_CHANGE /*rts_pin*/);

        _hasBeenInit = true;
    }

    bool send(DMXFrame dmxFrame) override
    {
        if (!_hasBeenInit)
            return false;

        // Write to the packet and send it.
        dmx_write(_uartBus, &dmxFrame, DMX_PACKET_SIZE);
        dmx_send(_uartBus);

        // Block until the packet is finished sending.
        dmx_wait_sent(_uartBus, DMX_TIMEOUT_TICK);

        return true;
    }

private:
    int _txPin;   // where to write to
    int _uartBus; // which uart to use

    bool _hasBeenInit{false};
};

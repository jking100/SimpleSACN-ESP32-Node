#pragma once

// someweisguy/esp_dmx
#include "esp_dmx.h"
// esp dmx does not include this file itself(seems to assume what framework its on), transient include problem with the DMX_CONFIG_DEFUALT macro
#include "esp_intr_alloc.h"

#include "model/StampedDMXFrame.hpp"
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

    void begin(dataLossOption opt) override
    {
        _dataLossOption = opt;

        // demo code someweisguy/esp_dmx
        if (_hasBeenInit)
            return;

        dmx_config_t config = DMX_CONFIG_DEFAULT;

        // ...declare the driver's DMX personalities...
        const int personality_count = 1;
        dmx_personality_t personalities[] = {
            {1, "Default Personality"}};

        dmx_driver_install(_uartBus, &config, personalities, personality_count);

        // target the pins and uart periph
        dmx_set_pin(_uartBus, _txPin, DMX_PIN_NO_CHANGE /*rx_pin*/, DMX_PIN_NO_CHANGE /*rts_pin*/);

        _hasBeenInit = true;
    }

    bool send(const StampedDMXFrame &dmxFrame) override
    {
        if (!_hasBeenInit)
            return false;

        dmx_write(_uartBus, &dmxFrame.frame.data, DMX_PACKET_SIZE);
        dmx_send(_uartBus);

        // Blocking
        dmx_wait_sent(_uartBus, DMX_TIMEOUT_TICK);

        return true;
    }

private:
    int _txPin;   // where to write to
    int _uartBus; // which uart to use

    dataLossOption _dataLossOption;

    bool _hasBeenInit{false};
};

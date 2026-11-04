#pragma once

// someweisguy/esp_dmx
#include "esp_dmx.h"
// esp_dmx's DMX_CONFIG_DEFAULT needs this but esp_dmx doesn't include it
#include "esp_intr_alloc.h"

#include "KingSACN/E131/DMXFrame.hpp"

// sends DMX frames out a uart using someweisguy/esp_dmx
class ESP_DMXSender
{
public:
    // @param txPin pin for uart tx
    // @param uartBus which uart peripheral to use
    ESP_DMXSender(int txPin, int uartBus) : _txPin(txPin), _uartBus(uartBus) {}

    // install the driver and set the pin; safe to call twice
    bool begin()
    {
        if (_initialized)
            return true;

        dmx_config_t config = DMX_CONFIG_DEFAULT;
        dmx_personality_t personalities[] = {{1, "Default Personality"}};
        if (!dmx_driver_install(_uartBus, &config, personalities, 1))
        {
            Serial.printf("Failed to install dmx driver\n");
            return false;
        }

        if (!dmx_set_pin(_uartBus, _txPin, DMX_PIN_NO_CHANGE /*rx*/, DMX_PIN_NO_CHANGE /*rts*/))
        {
            Serial.printf("Failed to set dmx pins\n");
            return false;
        }

        _initialized = true;
        return true;
    }

    // blocks until the frame is on the wire
    bool send(const DMXFrame &frame)
    {
        if (!_initialized)
            return false;

        // slot 0 is the start code, left at 0 by the driver; channels go in 1-512
        dmx_write_offset(_uartBus, 1, frame.data, 512);
        dmx_send(_uartBus);
        dmx_wait_sent(_uartBus, DMX_TIMEOUT_TICK);

        return true;
    }

private:
    int _txPin;
    int _uartBus;

    bool _initialized{false};
};

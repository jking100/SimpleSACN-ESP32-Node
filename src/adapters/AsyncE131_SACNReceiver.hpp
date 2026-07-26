#pragma once

#include <WiFi.h>
#include <ESPAsyncE131.h>

#include "model/DMXFrame.hpp"
#include "interfaces/SACNReceiver.hpp"

/*
SACNReceiver implemented using forkineye/ESPAsyncE131 library, his lib is wifi locked so this is too
*/
class AsyncE131_SACNReceiver : public SACNReceiver
{
public:
    AsyncE131_SACNReceiver(uint16_t universe, uint16_t universeCount, const char *wifi_ssid, const char *wifi_password)
        : _universe(universe), _universeCount(universeCount), _wifi_ssid(wifi_ssid), _wifi_password(wifi_password)
    {
        _e131 = new ESPAsyncE131(_universeCount);
    }

    // todo :: rule of 3 or 5 or 0 idk??? prolly
    ~AsyncE131_SACNReceiver()
    {
        delete _e131;
        delete &_frame;
    }

    /*
    this will hard block if it cant connect
    */
    bool begin() override
    {
        // example code from forkineye library
        WiFi.mode(WIFI_STA);
        WiFi.begin(_wifi_ssid, _wifi_password);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.printf("waiting wifi w/ {%s} : {%s}\n", _wifi_ssid, _wifi_password);
        }

        Serial.println("");
        Serial.print(F("Connected with IP: "));
        Serial.println(WiFi.localIP());

        // Listen via Multicast
        if (_e131->begin(E131_MULTICAST, _universe, _universeCount))
        {
            Serial.println(F("Listening for data..."));
            return true;
        }
        else
        {
            Serial.println(F("*** e131.begin failed ***"));
            return false;
        }
    }

    DMXFrame latest() override
    {
        // give last if nothing received since
        if (_e131->isEmpty())
        {
            // Serial.printf("Empty SACN\n");
            return _frame;
        }

        _e131->pull(&packet);

        //  packet is not of the configured universe, give last
        if (ntohs(packet.universe) != _universe)
        {
            Serial.printf("Bad universe{%d} - want {%d}\n", ntohs(packet.universe), _universe);
            return _frame;
        }

        memcpy(_frame.data, packet.property_values, 513);

        // Serial.printf("%d|%d|%d|%d\n", _frame.data[1], _frame.data[2], _frame.data[3], _frame.data[4]);
        return _frame;
    }

private:
    uint16_t _universe{};
    uint16_t _universeCount{};

    const char *_wifi_ssid{};
    const char *_wifi_password{};

    ESPAsyncE131 *_e131{};
    e131_packet_t packet{};

    DMXFrame _frame{};
};

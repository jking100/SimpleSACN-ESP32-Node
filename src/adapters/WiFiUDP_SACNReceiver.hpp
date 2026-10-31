#pragma once

#include <WiFi.h>
#include "SACNPacket.hpp"
#include "SACNParse.hpp"

#include "model/StampedDMXFrame.hpp"
#include "interfaces/SACNReceiver.hpp"

/*
SACNReceiver implemented using WiFiUDP from arduino core, using my handrolled sacn protocol implementation
*/
class WiFiUDP_SACNReceiver : public SACNReceiver
{
public:
    WiFiUDP_SACNReceiver(uint16_t universe, const char *wifi_ssid, const char *wifi_password)
        : _universe(universe), _wifi_ssid(wifi_ssid), _wifi_password(wifi_password)
    {
    }

    /*
    this will hard block if it cant connect
    */
    bool begin() override
    {
        // example code from forkineye library
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(_wifi_ssid, _wifi_password);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            // Serial.printf("waiting wifi w/ {%s} : {%s}\n", _wifi_ssid, _wifi_password);
        }

        Serial.println("");
        Serial.print(F("Connected with IP: "));
        Serial.println(WiFi.localIP());

        // multicast is per spec on 239.255.whatever, except the top 255 addrs
        _multicastIP = IPAddress(239, 255, ((_universe >> 8) & 0xFF), ((_universe >> 0) & 0xFF));

        // port is 5568
        bool stat = _UDPMulticastSource.beginMulticast(_multicastIP, 5568);

        // Listen via Multicast
        if (stat)
        {
            Serial.println(F("Listening for data..."));
            return true;
        }
        else
        {
            Serial.println(F("*** WiFiUDP.beginMulticast failed ***"));
            return false;
        }
    }

    bool poll(StampedDMXFrame &out) override
    {
        int size{};
        bool foundPacket{};

        while ((size = _UDPMulticastSource.parsePacket()) > 0)
        {

            _UDPMulticastSource.read(buf, sizeof(buf));

            if (SACNParser::parseToSACN(buf, size, &packet) != SACNParser::parseStatus::GOOD)
                continue;

            if (SACNParser::packetValidator(packet, &_universe, &_sequenceTracker) != SACNParser::validateStatus::GOOD)
                continue;

            foundPacket = true;

            memcpy(out.frame.data, packet.propertyValues, sizeof(out.frame.data));

            out.networkRxTime = millis();
            out.streamTerminated = SACNParser::isStreamTerminated(packet);
            out.previewData = SACNParser::isPreviewData(packet);
        }

        return foundPacket;
    }

private:
    uint16_t _universe{};

    WiFiUDP _UDPMulticastSource{};
    IPAddress _multicastIP{};

    const char *_wifi_ssid{};
    const char *_wifi_password{};

    sacnPacket packet{};

    SACNParser::SequenceTracker _sequenceTracker{};

    uint8_t buf[638];
};
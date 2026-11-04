#pragma once

#include <WiFi.h>

// joins wifi and a multicast group, hands back raw udp payloads; no sacn knowledge
class WiFiUDP_Poller
{
public:
    WiFiUDP_Poller(const char *ssid, const char *password, IPAddress multicastIP, uint16_t port)
        : _ssid(ssid), _password(password), _multicastIP(multicastIP), _port(port)
    {
    }

    // blocks until wifi connects
    bool begin()
    {
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(_ssid, _password);

        while (WiFi.status() != WL_CONNECTED)
            delay(500);

        Serial.print(F("\nConnected with IP: "));
        Serial.println(WiFi.localIP());

        if (!_udp.beginMulticast(_multicastIP, _port))
        {
            Serial.println(F("*** WiFiUDP.beginMulticast failed ***"));
            return false;
        }

        Serial.println(F("Listening for data..."));
        return true;
    }

    // drains every waiting packet, keeps the newest in out
    // @return bytes written to out, 0 if nothing was waiting
    size_t poll(uint8_t *out, size_t len)
    {
        size_t got{};
        while (_udp.parsePacket() > 0)
            got = _udp.read(out, len);
        return got;
    }

private:
    WiFiUDP _udp{};

    const char *_ssid{};
    const char *_password{};
    IPAddress _multicastIP{};
    uint16_t _port{};
};

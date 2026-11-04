# Feather-dmx-node

ESP32-S3 firmware that takes lighting data in over the network (sACN / E1.31) and blasts it
back out as DMX512 to real fixtures. Basically a simple single universe sACN-to-DMX node.

Right now it works end to end using my own implementation of the 2025 sACN standard for a receiver and the someweisguy/esp_dmx library to send dmx over UART;
This demo code in /src shows the utilization of these two libraries to turn an adafruit ESP32-S3 dev board into a single universe wifi sacn node.
This node has been tested working reliably with QLC+ and sACNView.
Possible applications of a board running this firmware are home/holiday lighting, backyard gatherings, weddings, or anywhere where a simple rgb light strip would leave something to be desired.

<p align="center">
  <img src="miscfiles/running-1.jpeg" width="500" alt="the assembled node - feather + M5 DMX unit">
  <br><em>the node as of this readme commit</em>
</p>

## How it's built

- Adafruit Feather ESP32-S3
- M5 [uart] DMX Unit (handles the nasty electrical stuff with DMX512 aka RS-485)

## Building

PlatformIO with Arduino framework, targeting an Adafruit Feather ESP32-S3.

You will need to create a `src/secrets/secrets.hpp` containing your WiFi info:

```cpp
namespace secrets
{
    const char ssid[] = "your-wifi";
    const char passphrase[] = "your-password";
}
```

Configuration of the universe and uart bus/tx pin can be done in src/config.hpp

Build and flash with PlatformIO

---
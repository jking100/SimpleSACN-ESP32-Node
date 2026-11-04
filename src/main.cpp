// third party
#include <Arduino.h>
#include <FreeRTOS.h>

// wifi details : see secrets/secrets.hpp.example
#include "secrets/secrets.hpp"

// sacn logic (lib/KingSACN)
#include "KingSACN/Node/safeNode.hpp"

// esp32 hardware
#include "adapters/WiFiUDP_Poller.hpp"
#include "adapters/ESP_DMXSender.hpp"
#include "diag.hpp"

constexpr bool PRINTDEBUG{true};

constexpr int uartBus{1};
constexpr int txPin{39};
constexpr uint16_t universe{1};
// E1.31 multicast group for a universe: 239.255.<hi>.<lo>
const IPAddress multicastIP(239, 255, (universe >> 8) & 0xFF, universe & 0xFF);
constexpr uint16_t sacnPort{5568};

Node::safeNode node({universe, sacnPolicy::timeoutPolicy::HOLD});
// Node::safeNode node({universe, sacnPolicy::timeoutPolicy::STOP});

WiFiUDP_Poller udp{secrets::ssid, secrets::passphrase, multicastIP, sacnPort};
ESP_DMXSender dmxOut{txPin, uartBus};

// 0 as argument results in 1hz
constexpr uint16_t hzToMS(uint8_t hz)
{
    return hz ? 1000 / hz : 1000;
}

// drain the socket and hand the newest packet to the node
void taskPollNetwork(void *)
{
    uint8_t buf[638]; // largest E1.31 data packet

    TickType_t lastWake{xTaskGetTickCount()};
    for (;;)
    {
        size_t n = udp.poll(buf, sizeof(buf));
        if (n)
            node.onPacket(buf, n, millis());

        xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(hzToMS(100)));
    }
}

// send the node's current frame out the uart, or nothing if it has none
void taskSendDMX(void *)
{
    DMXFrame frame;

    TickType_t lastWake{xTaskGetTickCount()};
    for (;;)
    {
        if (node.latestFrame(&frame, millis()))
            dmxOut.send(frame);

        xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(hzToMS(44))); // send dmx at 44hz
    }
}

void setup()
{
    Serial.begin(115200);

    // initialize wifi and dmx, solid red light if failure
    if (!udp.begin() || !dmxOut.begin())
    {
        Serial.printf("Error starting wifi or dmx\n");
        gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_NUM_13, 1);
        return;
    }

    TaskHandle_t pollNetwork{};
    TaskHandle_t sendDMX{};

    xTaskCreate(taskPollNetwork, "PollNetwork", configMINIMAL_STACK_SIZE * 10, NULL, 3, &pollNetwork);
    xTaskCreate(taskSendDMX, "SendDMX", configMINIMAL_STACK_SIZE * 10, NULL, 4, &sendDMX);

    // print stack/heap info for the above tasks to serial
    if (PRINTDEBUG)
    {
        TaskHandle_t handles[2]{pollNetwork, sendDMX};
        diag::begin(handles, 2);
    }
}

// all work happens in the tasks above
void loop() { vTaskDelete(NULL); }

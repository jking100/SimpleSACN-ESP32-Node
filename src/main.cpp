// ----------------------------------------
// others
#include <Arduino.h>
#include <FreeRTOS.h>

#include <esp_dmx.h>
// ----------------------------------------
// mine
#include "secrets/secrets.hpp"

// pure
#include "model/StampedDMXFrame.hpp"
#include "SACNPacket.hpp"
#include "interfaces/DMXSender.hpp"
#include "interfaces/SACNReceiver.hpp"
#include "interfaces/Fakes/FakeSACNReceiver.hpp"
#include "interfaces/FrameMailbox.hpp"

// device dependant
#include "adapters/ESP_DMXSender.hpp"
#include "adapters/WiFiUDP_SACNReceiver.hpp" //powered by my handrolled sacn
#include "adapters/FRTOSQueue_FrameMailbox.hpp"
#include "diag.hpp"
// ----------------------------------------

constexpr int uartBus{1};
constexpr int txPin{39};
ESP_DMXSender dmxOut{txPin, uartBus};

constexpr int UNIVERSE{1};
constexpr int UNIVERSE_COUNT{1};
WiFiUDP_SACNReceiver wifi_sacn_recv{UNIVERSE, secrets::ssid, secrets::passphrase};

FRTOSQueue_FrameMailbox frameMailbox{};

// 0 as argument results in 1hz
constexpr uint16_t hzToMS(uint8_t hz)
{
    return hz ? 1000 / hz : 1000;
}

void aTaskPollNetwork(void *pvParameters)
{
    TickType_t xLastWakeTime{xTaskGetTickCount()};
    for (;;)
    {
        StampedDMXFrame frame;
        if (wifi_sacn_recv.poll(frame))
            frameMailbox.publish(frame);

        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(hzToMS(88)));
    }
}

void aTaskPushUART(void *pvParameters)
{
    TickType_t xLastWakeTime{xTaskGetTickCount()};
    for (;;)
    {
        StampedDMXFrame frame;
        if (frameMailbox.latest(frame))
            dmxOut.send(frame);

        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(hzToMS(44)));
    }
}

void setup()
{
    Serial.begin(115200);

    { // wait for serial conn for a bit, red light on
        gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_NUM_13, 1);

        while (!Serial && millis() < 7500) // give 7.5s to connect serial before writing anything
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        gpio_set_level(GPIO_NUM_13, 0);
    }

    Serial.printf("last reset reason: %d\n", esp_reset_reason());
    if (esp_reset_reason() == esp_reset_reason_t::ESP_RST_PANIC)
    {
        Serial.print("I crashed :(");
    }

    {
        wifi_sacn_recv.begin();
        dmxOut.begin(DMXSender::dataLossOption::HOLD);
        frameMailbox.begin();
    }

    TaskHandle_t pollNetwork{};
    TaskHandle_t pushUART{};

    // minstacksize freertos is 828, overhead is 60 bytes (this port of freertos uses bytes not words)
    xTaskCreate(aTaskPollNetwork, "PollNetwork", configMINIMAL_STACK_SIZE * 7, NULL, 3, &pollNetwork);
    xTaskCreate(aTaskPushUART, "PushUART", configMINIMAL_STACK_SIZE * 7, NULL, 4, &pushUART);

    TaskHandle_t handles[2]{pollNetwork, pushUART};
    diag::begin(handles, 2);
}

void loop() { vTaskDelete(NULL); }

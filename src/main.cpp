// ----------------------------------------
// others
#include <Arduino.h>
#include <FreeRTOS.h>

#include <esp_dmx.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncE131.h>
// ----------------------------------------
// mine
#include "secrets/secrets.hpp"

// pure
#include "model/DMXFrame.hpp"
#include "SACNPacket.hpp"
#include "interfaces/DMXSender.hpp"
#include "interfaces/SACNReceiver.hpp"
#include "FakeSACNReceiver.hpp"
#include "MasterControlProgram.hpp"

// device dependant
#include "adapters/ESP_DMXSender.hpp"
#include "adapters/AsyncE131_SACNReceiver.hpp"
// ----------------------------------------

// ----------------------------------------
// GLOBAL OBJECTS
constexpr int PIN{33};
constexpr int NUMPIXELS{1};
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

constexpr int uartBus{1};
constexpr int txPin{39};
ESP_DMXSender dmxOut{txPin, uartBus};

constexpr int UNIVERSE{1};
constexpr int UNIVERSE_COUNT{1};
AsyncE131_SACNReceiver sacnIn{UNIVERSE, UNIVERSE_COUNT, secrets::ssid, secrets::passphrase};

FakeSACNReceiver fake_sacnIn{};

MasterControlProgram MCP{&sacnIn, &dmxOut};
// MasterControlProgram MCP{&fake_sacnIn, &dmxOut};

TaskHandle_t blink13{}; // i can use this handle with other rtos functions to see whats up with the task, and i think also comm between tasks
TaskHandle_t tickMCP{};
TaskHandle_t serialDiag{};

// up to 255, dont call me with 0
uint16_t hzToMS(uint8_t hz)
{
    return hz ? 1000 / hz : 1000;
}

// sanity blink the onboard red led
void aTaskBlink13(void *pvParameters)
{
    for (;;)
    {
        gpio_set_level(GPIO_NUM_13, 1);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_NUM_13, 0);
        vTaskDelay(750 / portTICK_PERIOD_MS);
    }
}

void aTaskSerialDiag(void *pvParameters)
{
    auto printStackInfo = [](TaskHandle_t th)
    {
        Serial.printf("%s stack headroom: %4u bytes\n", pcTaskGetName(th), uxTaskGetStackHighWaterMark(th));
    };

    for (;;)
    {
        printStackInfo(blink13);
        printStackInfo(tickMCP);
        printStackInfo(serialDiag); // could use NULL to check this task instead of refrencing itself
        Serial.printf("Free heap: %u bytes\n", esp_get_free_heap_size());
        Serial.printf("Min free heap ever: %u bytes\n", esp_get_minimum_free_heap_size());
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void aTaskTickMCP(void *pvParameters)
{
    TickType_t xLastWakeTime{xTaskGetTickCount()};
    for (;;)
    {
        MCP.tick();

        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(hzToMS(40)));
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

    MCP.begin(); // start the master control program

    // minstacksize freertos is 828, overhead is 60 bytes (this port of freertos uses bytes not words)
    xTaskCreate(aTaskTickMCP, "TickMCP", configMINIMAL_STACK_SIZE * 10, NULL, 2, &tickMCP);
    xTaskCreate(aTaskBlink13, "Blink13", configMINIMAL_STACK_SIZE * 2, NULL, 1, &blink13);
    xTaskCreate(aTaskSerialDiag, "SerialDiag", configMINIMAL_STACK_SIZE * 4, NULL, 1, &serialDiag);
}

void loop() { vTaskDelete(NULL); }

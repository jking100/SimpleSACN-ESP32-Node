#pragma once

#include <FreeRTOS.h>

namespace diag
{
    TaskHandle_t handlesArray[10];
    size_t used{};

    TaskHandle_t blink13{};
    // sanity blink the onboard red led on pin 13
    void aTaskBlink13(void *pvParameters)
    {
        for (;;)
        {
            gpio_set_level(GPIO_NUM_13, 1);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_13, 0);
            vTaskDelay(1500 / portTICK_PERIOD_MS);
        }
    }

    TaskHandle_t serialRTOSDiag{};
    void aTaskSerialRTOSDiag(void *pvParameters)
    {
        auto printStackInfo = [](TaskHandle_t th)
        {
            Serial.printf("%s stack headroom: %4u bytes\n", pcTaskGetName(th), uxTaskGetStackHighWaterMark(th));
        };

        for (;;)
        {
            for (size_t i{}; i < used; i++)
            {
                printStackInfo(handlesArray[i]);
            }
            printStackInfo(serialRTOSDiag); // could use NULL to check this task instead of refrencing itself
            printStackInfo(blink13);
            Serial.printf("------------------------------\n");
            Serial.printf("         Free heap: %u bytes\n", esp_get_free_heap_size());
            Serial.printf("Min free heap ever: %u bytes\n", esp_get_minimum_free_heap_size());
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

    void begin(TaskHandle_t *handles, size_t len)
    {
        xTaskCreate(aTaskBlink13, "Blink13", configMINIMAL_STACK_SIZE * 2, NULL, 1, &blink13);
        xTaskCreate(aTaskSerialRTOSDiag, "SerialDiag", configMINIMAL_STACK_SIZE * 4, NULL, 2, &serialRTOSDiag);

        for (int i{}; i < len && i < 10; i++)
        {
            handlesArray[i] = *(handles + i);
            used++;
        }
    }

}
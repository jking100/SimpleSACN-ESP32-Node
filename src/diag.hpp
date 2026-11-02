#pragma once

#include <FreeRTOS.h>

// debug helpers: heartbeat led and periodic stack/heap report over serial
namespace diag
{
    constexpr size_t maxWatched{10};
    TaskHandle_t watched[maxWatched];
    size_t watchedCount{};

    TaskHandle_t blink13{};
    // heartbeat on the onboard red led (pin 13)
    void taskBlink13(void *)
    {
        for (;;)
        {
            gpio_set_level(GPIO_NUM_13, 1);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_13, 0);
            vTaskDelay(1500 / portTICK_PERIOD_MS);
        }
    }

    TaskHandle_t serialReport{};
    void taskSerialReport(void *)
    {
        auto printStackInfo = [](TaskHandle_t th)
        {
            Serial.printf("%s stack headroom: %4u bytes\n", pcTaskGetName(th), uxTaskGetStackHighWaterMark(th));
        };

        for (;;)
        {
            for (size_t i{}; i < watchedCount; i++)
                printStackInfo(watched[i]);
            printStackInfo(serialReport);
            printStackInfo(blink13);
            Serial.printf("------------------------------\n");
            Serial.printf("         Free heap: %u bytes\n", esp_get_free_heap_size());
            Serial.printf("Min free heap ever: %u bytes\n", esp_get_minimum_free_heap_size());
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

    // start the diag tasks and report on the given tasks too (first 10)
    void begin(TaskHandle_t *handles, size_t len)
    {
        xTaskCreate(taskBlink13, "Blink13", configMINIMAL_STACK_SIZE * 2, NULL, 1, &blink13);
        xTaskCreate(taskSerialReport, "SerialDiag", configMINIMAL_STACK_SIZE * 6, NULL, 2, &serialReport);

        for (size_t i{}; i < len && i < maxWatched; i++)
            watched[watchedCount++] = handles[i];
    }
}

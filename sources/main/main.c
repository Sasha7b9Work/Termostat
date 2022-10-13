// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>


static void TaskLoop(void *cookie)
{
    vTaskDelay(1000 / portTICK_RATE_MS);

    while (1)
    {
        uint16 value = 0;

        if (adc_read(&value))
        {

        }
    }
}


void app_main()
{
    xTaskCreate(TaskLoop, "TaskLoop", 1024, NULL, 5, NULL);
}

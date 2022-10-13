// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "TemperatureSensor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


static void TaskLoop(void *cookie)
{
    while (1)
    {
        bool result = 0;

        float temperature = TempS_CurrentTemperature(&result);

        if (result)
        {

        }
    }

    vTaskDelay(1000 / portTICK_RATE_MS);
}


void app_main()
{
    xTaskCreate(TaskLoop, "TaskLoop", 1024, NULL, 5, NULL);
}

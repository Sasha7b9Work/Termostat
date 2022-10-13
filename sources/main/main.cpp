// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "TemperatureSensor.h"
#include "Heater.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


static void TaskLoop(void *cookie)
{
    while (1)
    {
        float temperature = 0.0f;

        if (TempS_CurrentTemperature(&temperature))
        {
            Heater_Process(temperature);
        }
    }

    vTaskDelay(1000 / portTICK_RATE_MS);
}


#ifdef __cplusplus
extern "C" {
#endif


void app_main()
{
    xTaskCreate(TaskLoop, "TaskLoop", 1024, NULL, 5, NULL);
}


#ifdef __cplusplus
}
#endif

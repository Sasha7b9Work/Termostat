// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware.h"
#include "TemperatureSensor.h"
#include "Heater.h"
#include <cstring>


#define BUF_SIZE (1024)


static void MainTask(void *cookie)
{
    while (1)
    {
        UART0::Send("Point 2");

        auto temp = TemperatureSensor::self->CurrentTemperature();

        if (temp.IsValid())
        {
            Heater::self->Process(temp.temp);
        }

        UART0::Send("Point 3");

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

#ifdef __cplusplus
extern "C" {
#endif

void app_main()
{
    volatile int i = 0;

    for (i = 0; i < 100000; i++)
    {

    }


    UART0::Init();

    UART0::Send("Point 0");

    TemperatureSensor::Init();

    UART0::Send("Point 00");

    Heater::Init();

    Heater::self->SetTargetTemperature(35.0f);

    UART0::Send("Point 1");

    xTaskCreate(MainTask, "MainTask", 1024, NULL, 5, NULL);
}

#ifdef __cplusplus
}
#endif

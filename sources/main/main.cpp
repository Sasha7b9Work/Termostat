// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "TemperatureSensor.h"
#include "Heater.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>


static void TaskLoop(void *cookie)
{
    static int counter = 0;

    while (1)
    {
        float temperature = 0.0f;

        static TemperatureSensor sensor;

        if (sensor.CurrentTemperature(&temperature))
        {
            Heater_Process(temperature);
        }

        gpio_set_level(GPIO_NUM_15, counter++ % 2);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


#ifdef __cplusplus
extern "C" {
#endif


void app_main()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_NUM_15;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_conf);


    xTaskCreate(TaskLoop, "TaskLoop", 1024, NULL, 5, NULL);
}


#ifdef __cplusplus
}
#endif

// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware.h"
#include <driver/gpio.h>
#include <cstring>


#define BUF_SIZE (1024)


static void MainTask(void *cookie)
{
    while (1)
    {
        static int counter = 0;

        gpio_set_level(GPIO_NUM_2, counter++ % 2);

        UART0::Send("Test message");

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
    io_conf.pin_bit_mask = GPIO_NUM_2;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_conf);

    UART0::Init();

    xTaskCreate(MainTask, "MainTask", 1024, NULL, 5, NULL);
}

#ifdef __cplusplus
}
#endif

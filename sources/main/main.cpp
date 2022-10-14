// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "MainTask.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <driver/uart.h>


#define BUF_SIZE (1024)


static void MainTask(void *cookie)
{
    NS1::MainTask task;

    while (1)
    {
        task.Update();

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

    uart_config_t uart_conf;
    uart_conf.baud_rate = 115200;
    uart_conf.data_bits = UART_DATA_8_BITS;
    uart_conf.parity = UART_PARITY_DISABLE;
    uart_conf.stop_bits = UART_STOP_BITS_1;
    uart_conf.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

    uart_param_config(UART_NUM_0, &uart_conf);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

    xTaskCreate(MainTask, "MainTask", 1024, NULL, 5, NULL);
}

#ifdef __cplusplus
}
#endif

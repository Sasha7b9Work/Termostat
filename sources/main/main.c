// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include <string.h>
#include <driver/gpio.h>
#include <driver/uart.h>


#define BUF_SIZE (1024)


void app_main()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_NUM_15;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_conf);

    volatile unsigned int i = 0;

    int counter = 0;

    uart_config_t uart_conf =
    {
        .baud_rate = 74880,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM_0, &uart_conf);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

    while (1)
    {
        for (i = 0; i < (unsigned int)(-1); i++)
        {
        }

        gpio_set_level(GPIO_NUM_15, counter++ % 2);

        char *message = "Test message";

        uart_write_bytes(UART_NUM_0, message, strlen(message) + 1);
    }
}

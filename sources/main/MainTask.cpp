// 2022/10/14 13:11:51 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "MainTask.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <cstring>


void NS1::MainTask::Update()
{
    static int counter = 0;

    gpio_set_level(GPIO_NUM_2, counter++ % 2);

    const char *message = "Test message";

    uart_write_bytes(UART_NUM_0, message, std::strlen(message) + 1);
}

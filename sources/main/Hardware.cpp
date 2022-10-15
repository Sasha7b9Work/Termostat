// 2022/10/15 15:55:16 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware.h"
#include <driver/uart.h>
#include <cstring>


void UART0::Init()
{
    uart_config_t uart_conf;
    uart_conf.baud_rate = 115200;
    uart_conf.data_bits = UART_DATA_8_BITS;
    uart_conf.parity = UART_PARITY_DISABLE;
    uart_conf.stop_bits = UART_STOP_BITS_1;
    uart_conf.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

    uart_param_config(UART_NUM_0, &uart_conf);
    uart_driver_install(UART_NUM_0, 2048, 0, 0, NULL, 0);
}


void UART0::Send(pchar message)
{
    uart_write_bytes(UART_NUM_0, message, std::strlen(message) + 1);
}

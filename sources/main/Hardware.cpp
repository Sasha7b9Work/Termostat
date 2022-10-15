// 2022/10/15 15:55:16 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware.h"
#include <driver/uart.h>
#include <driver/adc.h>
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


void GPIO::Init(gpio_num_t pin)
{
    if (pin == GPIO_NUM_2)
    {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = GPIO_NUM_2;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
    }
}


void GPIO::Set(gpio_num_t pin, uint value)
{
    gpio_set_level(pin, value);
}


void ADC::Init()
{
    adc_config_t config;

    config.mode = ADC_READ_TOUT_MODE;
    config.clk_div = 8;

    adc_init(&config);
}


bool ADC::Read(uint16 *value)
{
    uint16 adc = 0;

    bool result = adc_read(&adc);

    *value = adc;

    return result;
}

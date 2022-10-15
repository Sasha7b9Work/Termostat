// 2022/10/15 15:55:12 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include <driver/gpio.h>


namespace UART0
{
    void Init();

    void Send(pchar);
}


namespace GPIO
{
    void Init(gpio_num_t pin);

    void Set(gpio_num_t pin, uint);
}


namespace ADC
{
    void Init();

    bool Read(uint16 *);
}

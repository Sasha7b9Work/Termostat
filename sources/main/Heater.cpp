// 2022/10/13 21:12:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Heater.h"
#include "Hardware.h"


Heater *Heater::self = nullptr;


#define PIN_HEATER GPIO_NUM_15


void Heater::Init()
{
    UART0::Send("Point 000");

    static Heater heather;

    UART0::Send("Point 001");

    self = &heather;

    UART0::Send("Point 002");

    GPIO::Init(PIN_HEATER);

    UART0::Send("Point 003");
}


void Heater::Process(float temperature)
{
    if (temperature > target_temp)
    {
        Disable();
    }
    else if (temperature < (target_temp - hysteresis))
    {
        Enable();
    }
}


void Heater::Enable()
{
    if (!Enabled())
    {
        GPIO::Set(PIN_HEATER);
    }
}


void Heater::Disable()
{
    if (Enabled())
    {
        GPIO::Reset(PIN_HEATER);
    }
}


bool Heater::Enabled()
{
    return GPIO::IsHi(PIN_HEATER);
}
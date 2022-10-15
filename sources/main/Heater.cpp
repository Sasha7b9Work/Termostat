// 2022/10/13 21:12:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Heater.h"
#include "Hardware.h"


Heater *Heater::self = nullptr;


#define PIN_HEATER GPIO_NUM_15


void Heater::Create()
{
    static Heater heather;

    self = &heather;

    GPIO::Init(PIN_HEATER);
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
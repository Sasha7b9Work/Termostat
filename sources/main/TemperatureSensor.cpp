// 2022/10/13 17:25:21 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "TemperatureSensor.h"
#include <driver/adc.h>


static float TempS_Calculate(uint16);


float TempS_CurrentTemperature(bool &result)
{
    static float prev_value = 0.0;

    uint16 raw_value = 0;

    if (adc_read(&raw_value))
    {

    }
}


static float TempS_Calculate(uint16 raw_value)
{

}


// 2022/10/13 17:25:21 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "TemperatureSensor.h"
#include <driver/adc.h>


bool TemperatureSensor::CurrentTemperature(float *result)
{
    uint16 raw_value = 0;

    if (adc_read(&raw_value))
    {
        *result = Calculate(raw_value);

        return true;
    }

    return false;
}


float TemperatureSensor::Calculate(uint16 raw_value)
{
    return 0.0f;
}


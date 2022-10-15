// 2022/10/13 17:25:21 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "TemperatureSensor.h"
#include <driver/adc.h>


TemperatureSensor *TemperatureSensor::self = nullptr;


TemperatureSensor::TemperatureSensor()
{

}


void TemperatureSensor::Create()
{
    static TemperatureSensor sensor;

    self = &sensor;
}


DataTSensor TemperatureSensor::CurrentTemperature()
{
    uint16 raw_value = 0;

    DataTSensor result;

    if (adc_read(&raw_value))
    {
        result.SetTemperature(Calculate(raw_value));
    }

    return result;
}


float TemperatureSensor::Calculate(uint16 raw_value)
{
    return 0.0f;
}


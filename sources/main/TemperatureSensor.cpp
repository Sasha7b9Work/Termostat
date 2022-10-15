// 2022/10/13 17:25:21 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "TemperatureSensor.h"
#include "Hardware.h"
#include <driver/adc.h>


TemperatureSensor *TemperatureSensor::self = nullptr;


void TemperatureSensor::Create()
{
    static TemperatureSensor sensor;

    self = &sensor;

    ADC::Init();
}


DataTSensor TemperatureSensor::CurrentTemperature()
{
    uint16 raw_value = 0;

    DataTSensor result;

    if (ADC::Read(&raw_value))
    {
        result.SetTemperature(Calculate(raw_value));
    }

    return result;
}


float TemperatureSensor::Calculate(uint16 raw_value)
{
    return 0.0f;
}


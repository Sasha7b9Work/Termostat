// 2022/10/13 17:25:21 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "TemperatureSensor.h"
#include "Hardware.h"
#include <driver/adc.h>
#include <cmath>


TemperatureSensor *TemperatureSensor::self = nullptr;


void TemperatureSensor::Init()
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
    return CalculateNTC(10000, 2200, 3911, 25, 1);
}


float TemperatureSensor::CalculateNTC(uint nom_res, uint ser_res, uint16 betaK, uint8 temp, uint8 samples)
{
    double average = 0;

    for (int i = 0; i < samples; i++)
    {
        average += AnalogRead();
    }

    average /= samples;

    average = ser_res * ((3.3 * 1023 - 1) / average - 1);

    // Steinhart–Hart equation, based on https://learn.adafruit.com/thermistor/using-a-thermistor

    double steinhart = (std::log(average / nom_res)) / betaK;
    steinhart += 1.0 / (temp + 273.15);
    steinhart = 1.0 / steinhart;
    steinhart -= 273.15;

    return (float)steinhart;
}


float TemperatureSensor::AnalogRead()
{
    return 0.0f;
}

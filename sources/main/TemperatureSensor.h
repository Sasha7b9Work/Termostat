// 2022/10/13 17:25:36 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


class TemperatureSensor
{
public:
    bool CurrentTemperature(float *out);
private:
    float Calculate(uint16);
};
// 2022/10/13 21:12:06 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


class Heater
{
public:
    static void Create();
    static Heater *self;

    void Process(float temperature);

    void SetHysteresis(float hyst) { hysteresis = hyst; };

private:
    float hysteresis = 0.0f;        // Задача - поддерживать температуру в промежутке [source_temp...(source_temp - hysteresis)]
    float source_temp = 0.0f;

    void Enable();
    void Disable();
    bool Enabled();
};

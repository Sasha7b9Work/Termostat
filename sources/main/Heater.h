// 2022/10/13 21:12:06 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


class Heater
{
public:
    static void Init();

    static Heater *self;

    void Process(float temperature);

    void SetHysteresis(float hyst) { hysteresis = hyst; };

    void SetTargetTemperature(float target) { target_temp = target; }

private:
    float hysteresis = 0.0f;        // Задача - поддерживать температуру в промежутке [source_temp...(source_temp - hysteresis)]
    float target_temp = 0.0f;

    void Enable();
    void Disable();
    bool Enabled();
};

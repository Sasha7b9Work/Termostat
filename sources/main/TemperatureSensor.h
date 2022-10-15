// 2022/10/13 17:25:36 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


struct DataTSensor
{
    float temp;

    DataTSensor() : temp(0.0f), valid(false) {}
    void SetTemperature(float _temp)
    {
        temp = _temp;
        valid = true;
    }
    bool IsValid() { return valid; }
private:
    bool valid;
};


class TemperatureSensor
{
public:

    static void Init();

    static TemperatureSensor *self;

    DataTSensor CurrentTemperature();

private:

    float Calculate(uint16);

    float CalculateNTC(uint nom_res, uint ser_res, uint16 betaK, uint8 temp, uint8 samples);

    float AnalogRead();
};
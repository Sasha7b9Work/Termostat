// 2022/10/13 21:12:06 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


class Heater
{
public:
    static void Create();
    static Heater *self;

    void Process(float temperature);
};

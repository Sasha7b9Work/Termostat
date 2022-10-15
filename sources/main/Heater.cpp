// 2022/10/13 21:12:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Heater.h"


Heater *Heater::self = nullptr;


void Heater::Create()
{
    static Heater heather;

    self = &heather;
}


void Heater::Process(float temperature)
{

}
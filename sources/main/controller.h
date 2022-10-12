#pragma once

typedef enum
{//CONFIG_SENSOR_COUNT = 11 CHECK
    DS18B20 = 0,
    NTC3_3,
    NTC5,
    NTC6_8,
    NTC10,
    NTC12,
    NTC14_8,
    NTC15,
    NTC20,
    NTC33,
    NTC47
} SENSORS;


SENSORS controller_get_sensor_type();

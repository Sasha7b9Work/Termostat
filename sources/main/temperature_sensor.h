#pragma once

void temperature_sensor_init();

void toggleAnalogSwitch();

float temperature_sensor_get_temperature();

float calc_temp(uint16_t beta, uint16_t thermistor_r, uint16_t nominal_t, uint16_t resistor_r);

void sendTemperature();

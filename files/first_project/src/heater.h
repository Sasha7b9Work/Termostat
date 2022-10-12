#pragma once

void heater_init();
void heater_write(bool on);

#define heater_enable() heater_write(true)
#define heater_disable() heater_write(false)


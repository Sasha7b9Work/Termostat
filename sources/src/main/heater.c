#include <stdio.h>
#include <driver/gpio.h>

#include "heater.h"
#include "config.h"

_Bool inited = 0;

void heater_init()
{
//    gpio_set_level(CONFIG_HEATER_RELAY_GPIO, !(CONFIG_HEATER_RELAY_ACTIVE));
//    gpio_set_direction(CONFIG_HEATER_RELAY_GPIO, GPIO_OUTPUT);
    inited = 1;
}


void heater_write(_Bool on)
{
    printf("Setting heater %s\n", on ? "on" : "off");

    if (inited)
    {
        gpio_set_level(CONFIG_HEATER_RELAY_GPIO, on ? (CONFIG_HEATER_RELAY_ACTIVE) : !(CONFIG_HEATER_RELAY_ACTIVE));
    }
}

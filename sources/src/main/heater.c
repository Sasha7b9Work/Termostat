#include <stdio.h>
#include <driver/gpio.h>

#include "heater.h"
#include "config.h"

_Bool inited = _False;

void heater_init()
{
    gpio_write(CONFIG_HEATER_RELAY_GPIO, !(CONFIG_HEATER_RELAY_ACTIVE));
    gpio_enable(CONFIG_HEATER_RELAY_GPIO, GPIO_OUTPUT);
    inited = true;
}


void heater_write(bool on)
{
    printf("Setting heater %s\n", on ? "on" : "off");

    if (inited)
    {
        gpio_write(CONFIG_HEATER_RELAY_GPIO, on ? (CONFIG_HEATER_RELAY_ACTIVE) : !(CONFIG_HEATER_RELAY_ACTIVE));
    }
}

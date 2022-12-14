#include <FreeRTOS.h>
#include <task.h>
#include <esp/gpio.h>
#include <ds18b20/ds18b20.h>

#include "config.h"
#include "my_homekit.h"
#include "controller.h"
#include "temperature_sensor.h"

#include <math.h>

#include <esp/hwrand.h>

static float temperature = 0.0;
static int fail = 0;

static TaskHandle_t sensorTask;
static bool inited;

void sendTemperature()
{
    toggleAnalogSwitch();
    printf("PRE temperature %.2f\n", temperature);
    float local_temp = temperature;
    switch (controller_get_sensor_type())
    {
    case DS18B20:
        local_temp = ds18b20_read_single(CONFIG_TEMPERATURE_DIGIT_SENSOR_GPIO); //A0
        break;
    case NTC3_3:
        local_temp =  calc_temp(4000, 3300, 25, RESISTOR_T);
        break;
    case NTC5:
        local_temp =  calc_temp(4000, 5000, 25, 1100);
        break;
    case NTC6_8:
        local_temp =  calc_temp(4200, 6800, 25, RESISTOR_T);
        break;
    case NTC10:
        local_temp =  calc_temp(3911, 10000, 25, RESISTOR_T);
        break;
    case NTC12:
        local_temp =  calc_temp(3621, 12000, 25, RESISTOR_T);
        break;
    case NTC14_8:
        local_temp =  calc_temp(3761, 14800, 20, RESISTOR_T);
        break;
    case NTC15:
        local_temp =  calc_temp(3354, 15000, 25, RESISTOR_T);
        break;
    case NTC20:
        local_temp =  calc_temp(4000, 20000, 25, RESISTOR_T);
        break;
    case NTC33:
        local_temp =  calc_temp(4001, 33000, 25, RESISTOR_T);
        break;
    case NTC47:
        local_temp =  calc_temp(3887, 47000, 25, RESISTOR_T);
        break;
    default:
        printf("ERROR SENSOR VALUE");
    }

    printf("Got temperature %.2f\n", temperature);
    if (temperature >= 1)
    {
        my_homekit_set_current_temperature(temperature);
        controller_notify_current_temperature(temperature);
    }
    else
    {
        my_homekit_set_current_temperature(0);
        controller_notify_current_temperature(0);
    }
}

void temperature_sensor_task(void *_args)
{
    while (1)
    {
        printf("PRE temperature %.2f\n", temperature);
        float local_temp = temperature;

        switch (controller_get_sensor_type())
        {
        case DS18B20:
            local_temp = ds18b20_read_single(CONFIG_TEMPERATURE_DIGIT_SENSOR_GPIO); //A0
            break;
        case NTC3_3:
            local_temp =  calc_temp(4000, 3300, 25, RESISTOR_T);
            break;
        case NTC5:
            local_temp =  calc_temp(4000, 5000, 25, 1100);
            break;
        case NTC6_8:
            local_temp =  calc_temp(4200, 6800, 25, RESISTOR_T);
            break;
        case NTC10:
            local_temp =  calc_temp(3911, 10000, 25, RESISTOR_T);
            break;
        case NTC12:
            local_temp =  calc_temp(3621, 12000, 25, RESISTOR_T);
            break;
        case NTC14_8:
            local_temp =  calc_temp(3761, 14800, 20, RESISTOR_T);
            break;
        case NTC15:
            local_temp =  calc_temp(3354, 15000, 25, RESISTOR_T);
            break;
        case NTC20:
            local_temp =  calc_temp(4000, 20000, 25, RESISTOR_T);
            break;
        case NTC33:
            local_temp =  calc_temp(4001, 33000, 25, RESISTOR_T);
            break;
        case NTC47:
            local_temp =  calc_temp(3887, 47000, 25, RESISTOR_T);
            break;
        default:
            printf("ERROR SENSOR VALUE");
        }
        printf("Got local_temp temperature %.2f\n", local_temp);
        local_temp += controller_get_temp_shift();

        if (local_temp >= 1)
        {
            fail = 0;
            temperature = local_temp;
            controller_notify_target_state(1, false);
        }
        else
        {

            if (controller_get_sensor_type() != DS18B20)
            {
                gpio_write(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, true);
                vTaskDelayMs(50);
                gpio_write(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, false);
                vTaskDelayMs(50);
                gpio_write(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, true);
            }
            else
            {
                gpio_write(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, false);
                vTaskDelayMs(50);
                gpio_write(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, true);
                vTaskDelayMs(50);
                gpio_write(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, false);
            }
            if (++fail > 10)
            {
                fail = 0;
                controller_notify_target_state(0, false);
                my_homekit_set_current_temperature(0);
                controller_notify_current_temperature(0);
                temperature = 0;
            }
            vTaskDelayMs(5000);
            continue;
        }

        printf("Got temperature %.2f\n", temperature);
        if (temperature >= 1)
        {
            my_homekit_set_current_temperature(temperature);
            controller_notify_current_temperature(temperature);
        }
        else
        {
            my_homekit_set_current_temperature(0);
            controller_notify_current_temperature(0);
        }

        vTaskDelayMs(CONFIG_TEMPERATURE_POLL_PERIOD_MS);
    }
}

float calc_temp(uint16_t beta, uint16_t thermistor_r, uint16_t nominal_t, uint16_t resistor_r)
{

    float sum = 0;

    for (uint8_t i = 0; i < 10; i++)
    {
        sum += (float)sdk_system_adc_read();
    }

//    printf("(sum/10) %.2f\n", (sum/10));
    float average = resistor_r * ((3.3*1023 - 1) / (sum/10) - 1);

//    printf("average %.2f\n", average);

    float steinhart = (log(average / thermistor_r)) / beta;

//    printf("steinhart %.2f\n", steinhart);

    steinhart += 1.0 / (nominal_t + 273.15);

//    printf("steinhart %.2f\n", steinhart);

    steinhart = 1.0 / steinhart; // invert

//    printf("steinhart %.2f\n", steinhart);

    steinhart -= 273.15; // convert to celsius

//    printf("steinhart %.2f\n", steinhart);


    return steinhart;
}

void temperature_sensor_init()
{
        inited = true;
        gpio_enable(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, GPIO_OUTPUT);

        gpio_set_pullup(CONFIG_TEMPERATURE_DIGIT_SENSOR_GPIO, true, true);//A0

        toggleAnalogSwitch();

        printf("temperature_sensor_init \n");
        if (pdPASS != xTaskCreate(temperature_sensor_task, "TSen", 512, NULL, 1, &sensorTask))
        {
            printf("Failed to start temperature sensor task\n");
        }
}

void toggleAnalogSwitch()
{
    if (controller_get_sensor_type() != DS18B20)
    {
        gpio_write(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, true);
    }
    else
    {
        gpio_write(CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO, false);
    }
}

float temperature_sensor_get_temperature()
{
    return temperature;
}

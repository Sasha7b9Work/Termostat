#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <espressif/esp_system.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#include "config.h"
#include "controller.h"
#include "my_homekit.h"
#include "temperature_sensor.h"

static bool initialized = false;

static void thermostat_identify(homekit_value_t _value)
{
    printf("Thermostat identify\n");
}

static void on_homekit_event(homekit_event_t event)
{

    switch (event)
    {
    case HOMEKIT_EVENT_CLIENT_CONNECTED:
        printf("HOMEKIT_EVENT_CLIENT_CONNECTED\n");
        controller_notify_have_homekit_pair(false);
        break;
    case HOMEKIT_EVENT_PAIRING_SUCCESS:
        printf("HOMEKIT_EVENT_CLIENT_CONNECTED\n");
        controller_notify_have_homekit_pair(true);
        break;
    case HOMEKIT_EVENT_PAIRING_REMOVED:
    case HOMEKIT_EVENT_PAIRING_ERROR:
        printf("HOMEKIT_EVENT_PAIRING_REMOVED\n");
        sysparam_set_bool("have_homekit_pair", false);
        homekit_storage_reset();
        printf("Restarting\n");
//        sdk_system_restart();
        sdk_system_deep_sleep(500000);
        break;
    }
}


static void on_target_temperature(homekit_characteristic_t *ch, homekit_value_t value, void *context);
static void on_target_state(homekit_characteristic_t *ch, homekit_value_t value, void *context);

static homekit_characteristic_t current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0);

static homekit_characteristic_t target_temperature_thermostat  = HOMEKIT_CHARACTERISTIC_(
            TARGET_TEMPERATURE, 22,
            .min_value = (float[]){ CONFIG_TARGET_TEMPERATURE_THERMOSTAT_MIN },
            .max_value = (float[]){ CONFIG_TARGET_TEMPERATURE_THERMOSTAT_MAX },
            .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_target_temperature),
            );

static homekit_characteristic_t target_temperature_titan  = HOMEKIT_CHARACTERISTIC_(
            TARGET_TEMPERATURE, 22,
            .min_value = (float[]){ CONFIG_TARGET_TEMPERATURE_TITAN_MIN },
            .max_value = (float[]){ CONFIG_TARGET_TEMPERATURE_TITAN_MAX },
            .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_target_temperature),
            );

static homekit_characteristic_t current_state = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATING_COOLING_STATE, 0);

static homekit_characteristic_t units = HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS, 0);

static homekit_characteristic_t target_state = HOMEKIT_CHARACTERISTIC_(
            TARGET_HEATING_COOLING_STATE, 0,
            .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_target_state),
            .max_value = (float[]) {1},
            .valid_values = {
        .count = 2,
        .values = (uint8_t[]) { 0, 1, },
        },
            );

static homekit_characteristic_t accessory_name = HOMEKIT_CHARACTERISTIC_(NAME, "Thermostat");

static homekit_characteristic_t serial_number = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, "101");

static homekit_characteristic_t firmware_revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, "1");


static void on_target_temperature(homekit_characteristic_t *ch, homekit_value_t value, void *context)
{
    controller_notify_target_temperature(value.float_value);
}


static void on_target_state(homekit_characteristic_t *ch, homekit_value_t value, void *context)
{
    if ((uint8_t)controller_get_target_state() != value.uint8_value)
        controller_notify_target_state(value.uint8_value, true);
}


static homekit_accessory_t *accessories_thermostat[] =
{
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_thermostat, .services=(homekit_service_t*[])
    {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[])
        {
            &accessory_name,
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, CONFIG_MANUFACTURER),
            &serial_number,
            HOMEKIT_CHARACTERISTIC(MODEL, "Thermostat"),
            &firmware_revision,
            HOMEKIT_CHARACTERISTIC(IDENTIFY, thermostat_identify),
            NULL
        }),
        HOMEKIT_SERVICE(THERMOSTAT, .primary=true, .characteristics=(homekit_characteristic_t*[])
        {
            HOMEKIT_CHARACTERISTIC(NAME, "Thermostat"),
            &current_temperature,
            &target_temperature_thermostat,
            &current_state,
            &target_state,
            &units,
            NULL
        }),
        NULL
    }),
    NULL
};


static homekit_accessory_t *accessories_titan[] =
{
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_thermostat, .services=(homekit_service_t*[])
    {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[])
        {
            &accessory_name,
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, CONFIG_MANUFACTURER),
            &serial_number,
            HOMEKIT_CHARACTERISTIC(MODEL, "Titan"),
            &firmware_revision,
            HOMEKIT_CHARACTERISTIC(IDENTIFY, thermostat_identify),
            NULL
        }),
        HOMEKIT_SERVICE(THERMOSTAT, .primary=true, .characteristics=(homekit_characteristic_t*[])
        {
            HOMEKIT_CHARACTERISTIC(NAME, "Titan"),
            &current_temperature,
            &target_temperature_titan,
            &current_state,
            &target_state,
            &units,
            NULL
        }),
        NULL
    }),
    NULL
};



static homekit_server_config_t homekit_config =
{
    .config_number = 2,
    .password = "111-11-111",
    .on_event = on_homekit_event,
    .setupId = "A7QJ",
};


char *serial;
static size_t serial_size;


void my_homekit_init(char *name)
{
    if (initialized)
        return;

    accessory_name.value = HOMEKIT_STRING(name);

    serial_size = snprintf(NULL, 0, "%i", sdk_system_get_chip_id());
    serial = malloc(serial_size);
    snprintf(serial, serial_size+1, "%i", sdk_system_get_chip_id());
    serial_number.value = HOMEKIT_STRING(serial);


    homekit_config.setupId = malloc(5);
    snprintf(homekit_config.setupId, 5, "%d", sdk_system_get_chip_id());
    homekit_config.setupId[4] = '\0';


    char *firmware = malloc(4);
    snprintf(firmware, 4, "%03d", GIT_VERSION);
    firmware[3] = '\0';
    firmware_revision.value = HOMEKIT_STRING(firmware);



    current_temperature.value = HOMEKIT_FLOAT(temperature_sensor_get_temperature());

    current_state.value = HOMEKIT_UINT8((uint8_t)controller_get_current_state());

    target_state.value = HOMEKIT_UINT8((uint8_t)controller_get_target_state());


    if (controller_get_mode() == THERMOSTAT)
    {

        homekit_config.accessories = accessories_thermostat;
        target_temperature_thermostat.value = HOMEKIT_FLOAT(controller_get_target_temperature());

    }
    else
    {
        homekit_config.accessories = accessories_titan;
        target_temperature_titan.value = HOMEKIT_FLOAT(controller_get_target_temperature());

    }

    homekit_server_init(&homekit_config);

    char setup_uri[20];
    homekit_get_setup_uri(&homekit_config, setup_uri, sizeof(setup_uri));
    controller_notify_qr_homekit(setup_uri);

    initialized = true;
}


void my_homekit_set_current_temperature(float temperature)
{
    current_temperature.value = HOMEKIT_FLOAT(temperature);
    if (initialized) {
        homekit_characteristic_notify(&current_temperature, current_temperature.value);
    }
}

void my_homekit_set_target_temperature(float temperature)
{
    if (controller_get_mode() == THERMOSTAT)
    {
        target_temperature_thermostat.value = HOMEKIT_FLOAT(temperature);
        if (initialized)
        {
            homekit_characteristic_notify(&target_temperature_thermostat, target_temperature_thermostat.value);
        }
    }
    else
    {
        target_temperature_titan.value = HOMEKIT_FLOAT(temperature);
        if (initialized)
        {
            homekit_characteristic_notify(&target_temperature_titan, target_temperature_titan.value);
        }
    }

}

void my_homekit_set_target_state(uint8_t state)
{
    target_state.value = HOMEKIT_UINT8(state);
    if (initialized)
    {
        homekit_characteristic_notify(&target_state, target_state.value);
    }
}

void my_homekit_set_current_state(uint8_t state)
{
    current_state.value = HOMEKIT_UINT8(state);
    if (initialized)
    {
        homekit_characteristic_notify(&current_state, current_state.value);
    }
}

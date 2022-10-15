#include <Arduino.h>
#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <stdio.h>

#define ACCESSORY_SN  ("101010")
#define ACCESSORY_MANUFACTURER ("DIY")
#define ACCESSORY_MODEL  ("Lytko-101")
#define ACCESSORY_FIRMWARE_REVISION ("1.01")

char SETUP_ID[4]  = "1234";

static char ACCESSORY_NAME[32] = "Lytko";

extern byte spaWork;
extern uint8_t mac[6];
extern byte pair_hk;
extern byte news;
extern float target_temp;

void on_update(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
  update_state();
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, ACCESSORY_NAME);

homekit_characteristic_t serial_number = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, ACCESSORY_SN);

homekit_characteristic_t current_temperature = HOMEKIT_CHARACTERISTIC_(
      CURRENT_TEMPERATURE, 0,
.min_value = (float[]) {
  -100
}
    );

homekit_characteristic_t target_temperature  = HOMEKIT_CHARACTERISTIC_(
      TARGET_TEMPERATURE, 22,
.min_value = (float[]) {
  -100
},
.callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_update),
    );

homekit_characteristic_t units = HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS, 0);

homekit_characteristic_t current_state = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATING_COOLING_STATE, 0);

homekit_characteristic_t target_state  = HOMEKIT_CHARACTERISTIC_(
      TARGET_HEATING_COOLING_STATE, 0,
      .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_update),
.max_value = (float[]) {
  1
},
.valid_values = {
  .count = 2,
  .values = (uint8_t[]) {
    0, 1
  },
},
    );

void update_state() {
  uint8_t state = target_state.value.int_value;
  if (spaWork != state) {
    news |= 1;
    spaWork = state;
  }
  if (target_temperature.value.float_value != target_temp) {
    news |= 2;
  }
}

void accessory_identify(homekit_value_t _value) {

}

homekit_accessory_t *accessories[] =
{
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_thermostat,
  .services = (homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION,
    .characteristics = (homekit_characteristic_t*[]) {
      &name,
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, ACCESSORY_MANUFACTURER),
      &serial_number,
      HOMEKIT_CHARACTERISTIC(MODEL, ACCESSORY_MODEL),
      HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, ACCESSORY_FIRMWARE_REVISION),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
      NULL
    }),
    HOMEKIT_SERVICE(THERMOSTAT, .primary = true,
    .characteristics = (homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Thermostat"),
                             &current_temperature,
                             &target_temperature,
                             &current_state,
                             &target_state,
                             &units,
                             NULL
    }),
    NULL
  }),
  NULL
};

void on_homekit_event(homekit_event_t event) {
  if (event == HOMEKIT_EVENT_PAIRING_ADDED) {
    pair_hk = 255;
  }
  else if (event == HOMEKIT_EVENT_PAIRING_REMOVED) {
    pair_hk = 128;
  }
  else if (event == HOMEKIT_EVENT_CLIENT_CONNECTED) {
    if (!homekit_is_paired()) {
      pair_hk = 127;
    }
  }
  else if (event == HOMEKIT_EVENT_SERVER_INITIALIZED) {
    if (pair_hk == 0) {
      pair_hk = 8;
    }
  }
}

homekit_server_config_t config = {
  .accessories = accessories,
  .password = "111-11-111",
  .on_event = on_homekit_event,
  .setupId = SETUP_ID
};

void accessory_init() {
  name.value = HOMEKIT_STRING_CPP("Lytko");
  sprintf(SETUP_ID, "%02X%02X", mac[4], mac[5]);
}

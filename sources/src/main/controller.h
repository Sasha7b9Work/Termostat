    #pragma once

#include <FreeRTOS.h>
#include <event_groups.h>
#include <task.h>
#include <queue.h>

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


typedef enum
{
    THERMOSTAT = 0,
    TITAN,
} MODES;

typedef enum
{
    CONTROLLER_SCREEN_NO_CONNECT,

    CONTROLLER_SCREEN_LOCK,

    CONTROLLER_SCREEN_LOADING,

    CONTROLLER_SCREEN_TEMPERATURE,
    CONTROLLER_SCREEN_WELCOME,

    CONTROLLER_SCREEN_SETTINGS,

    CONTROLLER_SCREEN_DEBUG,

    CONTROLLER_SCREEN_CHOOSE_SENSOR,
    CONTROLLER_SCREEN_HYSTERESIS,

    CONTROLLER_SCREEN_HOMEKIT,

    CONTROLLER_SCREEN_CLEAR_SETTINGS,

    CONTROLLER_SCREEN_UPDATE,
} controller_screen_t;

typedef enum
{
    CONTROLLER_EVENT_LOCKED,

    CONTROLLER_EVENT_MODE,

    CONTROLLER_EVENT_HAVE_HOMEKIT_PAIR_CHANGED,

    CONTROLLER_EVENT_CURRENT_TEMPERATURE_CHANGED,
    CONTROLLER_EVENT_TARGET_TEMPERATURE_CHANGED,
    CONTROLLER_EVENT_TARGET_STATE_CHANGED,

    CONTROLLER_EVENT_SENSOR_TYPE,
    CONTROLLER_EVENT_HYSTERESIS,
    CONTROLLER_EVENT_TEMP_SHIFT,

    CONTROLLER_EVENT_WIFI_SIGNAL,

    CONTROLLER_EVENT_UPTIME,
    CONTROLLER_EVENT_MEMORY,

    CONTROLLER_EVENT_QR_HOMEKIT,

    CONTROLLER_EVENT_UPDATE_PERCENT,
    CONTROLLER_EVENT_UPDATE_AVAILABLE,

    CONTROLLER_EVENT_WIFI_CONNECT,
} controller_event_t;

typedef struct
{
    controller_event_t event;
    union
    {
        int int_value;
        float float_value;
        char *data;
        bool bool_value;
    };
    bool save;
} controller_notification_t;

struct _controller_state
{
    char* name;
    size_t name_size;

    uint8_t uart_no;

    QueueHandle_t notifications;
    QueueHandle_t cmd;
    EventGroupHandle_t flags;

    char read_buffer[50];
    uint8_t read_buffer_pos;
    uint8_t marker_count;

    char send_buffer[100];

    controller_screen_t active_screen;

    bool have_homekit_pair;

    bool locked;

    int mode;

    bool wifi_connected;

    float current_temperature;
    float target_temperature;
    int current_state;
    int target_state;


    float min_target_temperature;
    float max_target_temperature;


    int sensor_type;
    int hysteresis;
    float temp_shift;

    int wifi_signal;

    int uptime;
    int memory;

    char *qr_homekit;

    bool update_started;
    uint8_t update_percent;
    bool update_available;
};

void controller_start();
void controller_stop();

void controller_notify(controller_notification_t *notification);

void controller_notify_have_homekit_pair(bool value);

void controller_notify_locked(bool value);

void controller_notify_mode(MODES mode);

void controller_notify_current_temperature(float temperature);
void controller_notify_target_temperature(float temperature);
void controller_notify_target_state(int state, bool save);

void controller_notify_sensor_type(SENSORS type);
void controller_notify_hysteresis(int hysteresis);
void controller_notify_temp_shift(float value);

void controller_notify_wifi_signal(int signal);

void controller_notify_uptime(uint32_t uptime);
void controller_notify_memory(uint32_t memory);

void controller_notify_qr_homekit(char *qr_code);

void controller_notify_update_percent(uint8_t update_percent);

void controller_notify_update_available(bool update_available);

void controller_notify_wifi_connect(bool status);

bool controller_get_have_homekit_pair();

char* controller_get_name();

MODES controller_get_mode();

float controller_get_current_temperature();
float controller_get_target_temperature();

float controller_get_min_target_temperature();
float controller_get_max_target_temperature();

int controller_get_current_state();
int controller_get_target_state();

SENSORS controller_get_sensor_type();
int controller_get_hysteresis();
float controller_get_temp_shift();



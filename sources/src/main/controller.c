#include "controller.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>
#include <driver/uart.h>
//#include <sysparam.h>


#include "config.h"
//#include "my_homekit.h"
#include "temperature_sensor.h"
#include "heater.h"
#include "update.h"
#include <esp_sleep.h>

//#include "wifi_config.h"

#define SHUTDOWN_FLAG (1 << 0)

typedef struct _controller_state controller_t;

controller_t *controller;

static void controller_refresh_screen(controller_t *controller);

static void controller_process_command(char *cmd, uint8_t cmd_size);

static void controller_process_notifications(controller_t *controller);

static void controller_display(controller_t *controller, char *format, ...);


static bool is_controller_shutdown(controller_t *controller)
{
    return (bool)(xEventGroupGetBits(controller->flags) & SHUTDOWN_FLAG);
}

static void controller_display_locked(controller_t *controller)
{
    if (controller->locked)
    {
        controller_display(controller, DISPLAY_LOCK_ON);
    }
    else
    {
        controller_display(controller, DISPLAY_LOCK_OFF);
    }
}

static void controller_display_current_temperature(controller_t *controller)
{
    if (controller->active_screen == CONTROLLER_SCREEN_TEMPERATURE || controller->active_screen == CONTROLLER_SCREEN_LOCK)
        controller_display(controller, DISPLAY_DEGREE, (controller->current_temperature));
}

static void controller_display_target_temperature(controller_t *controller)
{
    if (controller->active_screen == CONTROLLER_SCREEN_TEMPERATURE || controller->active_screen == CONTROLLER_SCREEN_LOCK)
        controller_display(controller, DISPLAY_DEGREE_BIG, controller->target_temperature);
}

static void controller_display_target_state(controller_t *controller)
{
    if (controller->active_screen == CONTROLLER_SCREEN_TEMPERATURE || controller->active_screen == CONTROLLER_SCREEN_LOCK)
        controller_display(controller, DISPLAY_H_ACTIVE, controller->target_state);
}

static void controller_display_current_state(controller_t *controller)
{
    if (controller->active_screen == CONTROLLER_SCREEN_TEMPERATURE || controller->active_screen == CONTROLLER_SCREEN_LOCK)
        controller_display(controller, DISPLAY_ARROW, controller->current_state);
}


static void controller_display_wifi_signal(controller_t *controller)
{
    if (controller->active_screen == CONTROLLER_SCREEN_TEMPERATURE)
        controller_display(controller, DISPLAY_SIGNAL, controller->wifi_signal);
}


static void controller_display_memory(controller_t *controller)
{
    if (controller->active_screen == CONTROLLER_SCREEN_DEBUG)
    {
        char p[20];
        itoa (controller->memory, p, 10);
        controller_display(controller, DISPLAY_MEMORY, p);
    }
}

static void controller_display_uptime(controller_t *controller)
{
    if (controller->active_screen == CONTROLLER_SCREEN_DEBUG)
    {
        char p[20];
        itoa (controller->uptime, p, 10);
        controller_display(controller, DISPLAY_UPTIME, p);
    }
}

static void controller_display_hist_shift_label(controller_t *controller)
{
    controller_display(controller, DISPLAY_HIST_SHIFT_LABEL, controller->hysteresis, controller->temp_shift);
}


static void controller_display_sensor_type(controller_t *controller)
{
    controller_display(controller, DISPLAY_SENSOR_TYPE, controller->sensor_type);
    if (controller->sensor_type < 10)
        controller_display(controller, DISPLAY_SENSOR_NAME, controller->sensor_type);
    else
        controller_display(controller, DISPLAY_SENSOR_NAME_PAGE_1, controller->sensor_type);
}

static void controller_display_hysteresis(controller_t *controller)
{
    controller_display(controller, DISPLAY_HYST, controller->hysteresis);
}

static void controller_display_temp_shift(controller_t *controller)
{
    controller_display(controller, DISPLAY_TEMP_SHIFT, controller->temp_shift);
}


static void controller_display_qr_wifi(controller_t *controller)
{
    if (controller->name == NULL)
        return;

    controller_display(controller, DISPLAY_QR_WIFI, controller->name);
}

static void controller_display_update_percent(controller_t *controller)
{
    controller_display(controller, DISPLAY_UPDATE_STATUS, controller->update_percent);
    controller_display(controller, DISPLAY_UPDATE_PERCENT, controller->update_percent);
}

static void controller_display_qr_homekit(controller_t *controller)
{
    if (controller->qr_homekit == NULL)
        return;

    controller_display(controller, DISPLAY_QR_HOMEKIT, controller->qr_homekit);
}




void controller_notify_locked(bool value)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_LOCKED,
                                                          .bool_value = value,
                                                      }});
}


void controller_notify_mode(MODES type)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_MODE,
                                                          .int_value = type,
                                                      }});
}

void controller_notify_have_homekit_pair(bool value)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_HAVE_HOMEKIT_PAIR_CHANGED,
                                                          .bool_value = value,
                                                      }});
}


void controller_notify_current_temperature(float temperature)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_CURRENT_TEMPERATURE_CHANGED,
                                                          .float_value = temperature,
                                                      }});
}

void controller_notify_target_temperature(float temperature)
{
    if (controller->target_temperature != temperature)
    {
        controller_notify((controller_notification_t[]) { {
                                                              .event = CONTROLLER_EVENT_TARGET_TEMPERATURE_CHANGED,
                                                              .float_value = temperature,
                                                          }});
    }
}

void controller_notify_target_state(int state, bool save)
{
    printf("controller_notify_target_state %i %i \n", state, save);
    //    if (controller->target_state != state || local)
    //    {
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_TARGET_STATE_CHANGED,
                                                          .int_value = state,
                                                          .save = save,
                                                      }});
    //    }
}


void controller_notify_sensor_type(SENSORS type)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_SENSOR_TYPE,
                                                          .int_value = type,
                                                      }});
}

void controller_notify_hysteresis(int hysteresis)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_HYSTERESIS,
                                                          .int_value = hysteresis,
                                                      }});
}

void controller_notify_temp_shift(float value)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_TEMP_SHIFT,
                                                          .float_value = value,
                                                      }});
}


void controller_notify_wifi_signal(int signal)
{

    if (controller->wifi_signal != signal)
    {
        controller_notify((controller_notification_t[]) { {
                                                              .event = CONTROLLER_EVENT_WIFI_SIGNAL,
                                                              .int_value = signal,
                                                          }});
    }
}


void controller_notify_uptime(uint32_t uptime)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_UPTIME,
                                                          .int_value = uptime,
                                                      }});
}

void controller_notify_memory(uint32_t memory)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_MEMORY,
                                                          .int_value = memory,
                                                      }});
}

void controller_notify_qr_homekit(char *qr_code)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_QR_HOMEKIT,
                                                          .data = strdup(qr_code),
                                                      }});
}


void controller_notify_update_percent(uint8_t update_percent)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_UPDATE_PERCENT,
                                                          .int_value = update_percent,
                                                      }});
}


void controller_notify_update_available(bool update_available)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_UPDATE_AVAILABLE,
                                                          .bool_value = update_available,
                                                      }});
}

void controller_notify_wifi_connect(bool status)
{
    controller_notify((controller_notification_t[]) { {
                                                          .event = CONTROLLER_EVENT_WIFI_CONNECT,
                                                          .bool_value = status,
                                                      }});
}




char* controller_get_name()
{
    return controller->name;
}


MODES controller_get_mode()
{
    return controller->mode;
}

float controller_get_current_temperature()
{
    return controller->current_temperature;
}

float controller_get_target_temperature()
{
    return controller->target_temperature;
}

int controller_get_current_state()
{
    return controller->current_state;
}

int controller_get_target_state()
{
    return controller->target_state;
}

SENSORS controller_get_sensor_type()
{
    return controller->sensor_type;
}

int controller_get_hysteresis()
{
    return controller->hysteresis;
}

float controller_get_temp_shift()
{
    return controller->temp_shift;
}

bool controller_get_have_homekit_pair()
{
    return controller->have_homekit_pair;
}

static void controller_free(controller_t *controller)
{
    if (controller->name)
        free(controller->name);
    if (controller->qr_homekit)
        free(controller->qr_homekit);

    vEventGroupDelete(controller->flags);
    vQueueDelete(controller->notifications);

    free(controller);
}


static void controller_update_state(controller_t *controller)
{
    controller->target_temperature =
            MIN(controller->max_target_temperature,
                MAX(controller->min_target_temperature,
                    controller->target_temperature));

//    if (controller->target_state == MY_HOMEKIT_TARGET_STATE_OFF)
//    {
//        heater_disable();
//        my_homekit_set_current_state(MY_HOMEKIT_CURRENT_STATE_OFF);
//        controller->current_state = 0;
//    }
//    else if (controller->target_state == MY_HOMEKIT_TARGET_STATE_HEAT)
//    {
//        float hysteresis_adj = controller->hysteresis == 0.0 ? CONFIG_HEATER_HYSTERESIS : controller->hysteresis;
//
//        if (controller->current_temperature >= controller->target_temperature)
//        {
//            heater_disable();
//            controller->current_state = 0;
//            my_homekit_set_current_state(MY_HOMEKIT_CURRENT_STATE_OFF);
//        }
//        else
//            if (controller->current_temperature <= (controller->target_temperature - hysteresis_adj))
//            {
//                heater_enable();
//                controller->current_state = 1;
//                my_homekit_set_current_state(MY_HOMEKIT_CURRENT_STATE_HEATING);
//            }
//    }

    controller_display_current_temperature(controller);
    controller_display_target_temperature(controller);
    controller_display_current_state(controller);
    controller_display_target_state(controller);
}

static void controller_display(controller_t *controller, char *format, ...)
{
    va_list args;
    va_start(args, format);
    size_t size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    char *extra_buffer = NULL;
    char *buffer = controller->send_buffer;

    if (size >= sizeof(controller->send_buffer))
    {
        buffer = extra_buffer = malloc(size + 1);
    }

    va_start(args, format);
    size = vsnprintf(buffer, size+1, format, args);
    va_end(args);
    buffer[size] = 0;

    DEBUG("sending command \"%s\"", buffer);
    taskENTER_CRITICAL();
//    softuart_puts(controller->uart_no, buffer);
//    softuart_put(controller->uart_no, 0xff);
//    softuart_put(controller->uart_no, 0xff);
//    softuart_put(controller->uart_no, 0xff);
    taskEXIT_CRITICAL();

    if (extra_buffer)
    {
        free(extra_buffer);
    }
}

//static void controller_read_command(controller_t *controller)
//{
//    char *cmd = controller->read_buffer;
//    uint8_t cmd_size = controller->read_buffer_pos;
//
//    {
//        char *debug_buffer = malloc(64);
//
//        size_t left_chars = 63;
//        char *p = debug_buffer;
//        for (int i=0; i<controller->read_buffer_pos && left_chars >= 3; i++) {
//            size_t size = snprintf(p, left_chars, " %02X", cmd[i]);
//            p += size;
//            left_chars -= size;
//        }
//        *p = 0;
//        DEBUG("received command \"%s\"", debug_buffer);
//        free(debug_buffer);
//    }
//
//    while (cmd_size && cmd[0] == 0xD1) {
//        cmd++;
//        cmd_size--;
//    }
//
//    if (!cmd_size)
//        return;
//
//    controller_process_command(cmd, cmd_size);
//
//}

static bool startExist;

static void controller_process_serial(controller_t *controller) {
//    while (softuart_available(controller->uart_no)) {
//        uint8_t c = softuart_read(controller->uart_no);
//
//        //        if (c == 0x1a || c == 0xd1 || c == 0xd0 || c == 0xe8 || c == 0xd2 || c == 0xf7  || c == 0x7f || c == 0xdf)
//        //            continue;
//
//        if (c == 0x0 && !startExist)
//        {
//            continue;
//        }
//
//        if (c == 0x66 || c == 0x65)
//        {
//            controller->read_buffer_pos = 0;
//            controller->marker_count = 0;
//            startExist = true;
//        }
//
//      if (c == 0xff)
//      {
//          if (++controller->marker_count >= 3)
//          {
//              if (controller->read_buffer_pos)
//              {
//                  controller_read_command(controller);
//                  startExist = false;
//              }
//              // Do not hog processing
//              taskYIELD();
//
//              controller->read_buffer_pos = 0;
//              controller->marker_count = 0;
//          }
//      }
//      else
//      {
//          if (controller->read_buffer_pos < sizeof(controller->read_buffer))
//          {
//              controller->read_buffer[controller->read_buffer_pos++] = c;
//              controller->marker_count = 0;
//          }
//      }
//  }
}

static void controller_task(void *param)
{
    controller_display(controller, "rest");

    while (!is_controller_shutdown(controller))
    {
        controller_process_serial(controller);
        controller_process_notifications(controller);

        vTaskDelayMs(10);
    }

//    softuart_close(controller->uart_no);

    controller_free(controller);
    controller = NULL;

    vTaskDelete(NULL);
}

void controller_notify(controller_notification_t *notification)
{
    if (!controller)
        return;

    if (xQueueSend(controller->notifications, notification, 10 / portTICK_PERIOD_MS) != pdTRUE)
    {
        ERROR("Failed to send controller notification");
    }
}


void controller_start()
{
    if (controller)
        return;

    startExist = false;

    controller_t *c = calloc(1, sizeof(controller_t));
    c->notifications = xQueueCreate(20, sizeof(controller_notification_t));
    if (!c->notifications)
    {
        ERROR("Failed to allocate controller notifications queue");
        controller_free(c);
        return;
    }
    c->flags = xEventGroupCreate();
    if (!c->flags) {
        ERROR("Failed to allocate controller event group");
        controller_free(c);
        return;
    }

    c->mode=0;
//    sysparam_get_int8("mode", &c->mode);
    c->mode =
            MIN(1,
                MAX(0,
                    c->mode));

//    c->name_size = snprintf(NULL, 0, TEMPLATE_NAME, sdk_system_get_chip_id());
    c->name = malloc(c->name_size);
//    snprintf(c->name, c->name_size+1, TEMPLATE_NAME, sdk_system_get_chip_id());

    if (c->mode == THERMOSTAT)
    {
        c->min_target_temperature = CONFIG_TARGET_TEMPERATURE_THERMOSTAT_MIN;

        c->max_target_temperature = CONFIG_TARGET_TEMPERATURE_THERMOSTAT_MAX;
    }
    else
    {
        c->min_target_temperature = CONFIG_TARGET_TEMPERATURE_TITAN_MIN;

        c->max_target_temperature = CONFIG_TARGET_TEMPERATURE_TITAN_MAX;
    }

    c->uart_no = 0;
    c->locked = false;
//    sysparam_get_bool("locked", &c->locked);


    c->have_homekit_pair = false;
//                                sysparam_set_bool("have_homekit_pair", true);
//    sysparam_get_bool("have_homekit_pair", &c->have_homekit_pair);

    c->current_temperature = temperature_sensor_get_temperature();

    int8_t target_temperature_local = 0;
//    sysparam_get_int8("target_temperature", &target_temperature_local);
    c->target_temperature =
            MIN(c->max_target_temperature,
                MAX(c->min_target_temperature,
                    (float)target_temperature_local));


    c->target_state = false;
//    sysparam_get_bool("target_state", &c->target_state);

    c->sensor_type=0;
//    sysparam_get_int8("sensor_type", &c->sensor_type);
    c->sensor_type =
            MIN(CONFIG_SENSOR_COUNT,
                MAX(0,
                    c->sensor_type));

    c->hysteresis=1;
//    sysparam_get_int8("hysteresis", &c->hysteresis);
    c->hysteresis = MIN(5, MAX(1, c->hysteresis));

    int temp_shift = 0;
//    sysparam_get_int32("temp_shift", &temp_shift);
    c->temp_shift = MIN(5, MAX(-5, temp_shift * 0.1));

    c->wifi_connected = false;

    c->update_started = false;
    c->update_percent = 0;
    c->update_available = false;

    printf("Device name: %s\n", c->name);

//    if (!softuart_open(c->uart_no, 9600, CONFIG_DISPLAY_RX_GPIO, CONFIG_DISPLAY_TX_GPIO))
//    {
//        ERROR("Failed to intialize soft UART");
//        controller_free(c);
//        return;
//    }

    controller = c;
    if (xTaskCreate(controller_task, "Ctrl", 512, NULL, 2, NULL) != pdTRUE)
    {
        ERROR("Failed to start controller task");
        controller_free(c);
        controller = NULL;
        return;
    }

}

void controller_stop()
{
    if (!controller)
        return;

    xEventGroupSetBits(controller->flags, SHUTDOWN_FLAG);
}


static void controller_refresh_screen(controller_t *controller)
{
    switch (controller->active_screen)
    {
    case CONTROLLER_SCREEN_NO_CONNECT:

        controller_display(controller, "rest");
        controller_display(controller, DISPLAY_HK);
        controller_display(controller, DISPLAY_BKCMD);


        break;
    case CONTROLLER_SCREEN_LOCK:
        controller_display_current_temperature(controller);
        controller_display_target_temperature(controller);
        controller_display_target_state(controller);
        controller_display_current_state(controller);
        controller_display_wifi_signal(controller);

        break;
    case CONTROLLER_SCREEN_LOADING:

        controller_display(controller, DISPLAY_HK);
        controller_display(controller, DISPLAY_BKCMD);
        controller_display(controller, DISPLAY_MODE, controller->mode);
        controller_display_locked(controller);
        controller_display_sensor_type(controller);

        controller_notify_wifi_connect(controller->wifi_connected);

        controller_display(controller, DISPLAY_NEW_VERSION, "");
        controller_display(controller, DISPLAY_UPDATE_OFF);

        controller_display_hist_shift_label(controller);


        break;

    case CONTROLLER_SCREEN_WELCOME:
        controller_display_qr_wifi(controller);
        break;

    case CONTROLLER_SCREEN_TEMPERATURE:
        controller_display_current_temperature(controller);
        controller_display_target_temperature(controller);
        controller_display_target_state(controller);
        controller_display_current_state(controller);
        controller_display_wifi_signal(controller);
        sendTemperature();
        //move to display
        if (controller->update_available)
        {
            controller_display(controller, DISPLAY_UPDATE_ON);
        }
        else
        {
            controller_display(controller, DISPLAY_UPDATE_OFF);
        }
        break;

    case CONTROLLER_SCREEN_SETTINGS:
        controller_display_hist_shift_label(controller);
        break;

    case CONTROLLER_SCREEN_DEBUG:
        controller_display_memory(controller);
        controller_display_uptime(controller);
        controller_display(controller, DISPLAY_MODE, controller->mode);
        break;

    case CONTROLLER_SCREEN_CHOOSE_SENSOR:
        controller_display_sensor_type(controller);
        break;

    case CONTROLLER_SCREEN_HYSTERESIS:
        controller_display_hysteresis(controller);
        controller_display_temp_shift(controller);
        break;

    case CONTROLLER_SCREEN_HOMEKIT:
        controller_display_qr_homekit(controller);
        break;

    case CONTROLLER_SCREEN_CLEAR_SETTINGS:
        break;

    case CONTROLLER_SCREEN_UPDATE:
    {
        check_ota();
        char p[4];
        snprintf(p, 4, "%03d", GIT_VERSION);
        controller_display(controller, DISPLAY_VERSION, p);

        if (controller->update_available)
        {
            snprintf(p, 4, "%03d", GIT_VERSION+1);
            controller_display(controller, DISPLAY_NEW_VERSION, p);
        }
    }
        break;
    }
}

static void controller_process_command(char *cmd, uint8_t cmd_size)
{
    if (cmd[0] == 0x66 && cmd_size == 2)
    {
        switch (cmd[1])
        {
        case 0x0:
        case 0xC:
            if (controller->active_screen == CONTROLLER_SCREEN_LOADING)
                return;
            controller->active_screen = CONTROLLER_SCREEN_LOADING;
            break;
        case 0x1:
            controller->active_screen = CONTROLLER_SCREEN_TEMPERATURE;
            break;
        case 0x2:
            controller->active_screen = CONTROLLER_SCREEN_UPDATE;
            break;
        case 0x3:
            controller->active_screen = CONTROLLER_SCREEN_HOMEKIT;
            break;
        case 0x4:
            controller->active_screen = CONTROLLER_SCREEN_NO_CONNECT;
            break;
        case 0x5:
            controller->active_screen = CONTROLLER_SCREEN_SETTINGS;
            break;
        case 0x6:
            controller->active_screen = CONTROLLER_SCREEN_HYSTERESIS;
            break;
        case 0x7:
            controller->active_screen = CONTROLLER_SCREEN_CHOOSE_SENSOR;
            break;
        case 0x8:
            controller->active_screen = CONTROLLER_SCREEN_LOCK;
            break;
        case 0x9:
            controller->active_screen = CONTROLLER_SCREEN_CLEAR_SETTINGS;
            break;
        case 0x18:
            controller->active_screen = CONTROLLER_SCREEN_WELCOME;
            break;
        case 0xF:
            controller->active_screen = CONTROLLER_SCREEN_DEBUG;
            break;
        default:
            return;
        };
        controller_refresh_screen(controller);
    }
    else if (cmd[0] == 0x65)
    {
        if (cmd[1] == 1)
        {
            // click notification
            switch (cmd[2])
            {
            case 0xD:
                controller_notify_target_temperature(controller->target_temperature + 0.5);
                break;
            case 0xE:
                controller_notify_target_temperature(controller->target_temperature - 0.5);
                break;
            case 0x11:
                controller_notify_target_state(1, true);
                break;
            case 0x14:
                controller_notify_target_state(0, true);
                break;
            case 0x16:
                controller_notify_sensor_type((int)cmd[3]);
                break;
            case 0x3:
                controller_notify_mode(THERMOSTAT);
                break;
            case 0x2:
                controller_notify_mode(TITAN);
                break;
            }
        }
        else
            if (cmd[1] == 6)
            {
                switch(cmd[2])
                {
                case 0xF:
                    controller_notify_hysteresis(controller->hysteresis + 0.5);
                    break;
                case 0x11:
                    controller_notify_hysteresis(controller->hysteresis - 0.5);
                    break;
                case 0xD:
                    controller_notify_temp_shift(controller->temp_shift + 0.5);
                    break;
                case 0xC:
                    controller_notify_temp_shift(controller->temp_shift - 0.5);
                    break;
                case 0x3:
                    controller_notify_locked(true);
                    break;
                case 0x4:
                    controller_notify_locked(false);
                    break;
                }
            }
            else
                if (cmd[1] == 0xA) // Reset
                {
                    controller_display(controller, DISPLAY_HK_LOADING);

//                    homekit_storage_reset();

//                    printf("wifi_config_reset = %s \n", wifi_config_reset() ? "true" : "false");

//                    sysparam_set_int8("mode", 0);
//                  sysparam_set_bool("have_homekit_pair", false);
//                  sysparam_set_bool("locked", false);
//                  sysparam_set_int8("target_temperature", 0);
//                  sysparam_set_bool("target_state", false);
//                  sysparam_set_int8("sensor_type", 0);
//                  sysparam_set_int8("hysteresis", 0);
//                  sysparam_set_int32("temp_shift", 0);
//                    sdk_system_restart();
                    esp_deep_sleep(500000);

                }
                else
                    if (cmd[1] == 0x2)
                    {
                        switch(cmd[2])
                        {
                        case 0xB:
                            if (!controller->update_started)
                            {
                                printf("need update is ");
//                                if (sysparam_set_bool("need_update", true) == SYSPARAM_OK)
//                                {
//                                    printf ("ok\n");
//                                    sdk_system_restart();
//                                    sdk_system_deep_sleep(500000);
//                                }
                                printf ("not ok\n");
                                //go to update-mode
                            }
                            break;
                        }
                    }
                    else
                        if (cmd[1] == 0xE && !controller->update_started && controller->wifi_connected)
                        {
                            switch(cmd[2])
                            {
                            case 0x1:
                                controller_display_update_percent(controller);
                                controller->update_started = true;
//                                startUpdateRestore();
                                break;
                            case 0xD:
                                controller_display_update_percent(controller);
                                controller->update_started = true;
//                                startUpdateSpecial();
                                break;
                            }
                        }
    }
}

static void controller_process_notifications(controller_t *controller)
{
    controller_notification_t notification;
    while (xQueueReceive(controller->notifications, &notification, 0) == pdTRUE)
    {
        switch (notification.event)
        {

        case CONTROLLER_EVENT_LOCKED:
            controller->locked = notification.bool_value;

//            sysparam_set_bool("locked", controller->locked);

            controller_display_locked(controller);
            break;

        case CONTROLLER_EVENT_MODE:
//            sysparam_set_int8("mode", notification.int_value);
            controller->mode = notification.int_value;
            if (controller->name != NULL)
                free(controller->name);
//            controller->name_size = snprintf(NULL, 0, TEMPLATE_NAME, esp_get_chip_id());
            controller->name = malloc(controller->name_size);
//            snprintf(controller->name, controller->name_size+1, TEMPLATE_NAME, sdk_system_get_chip_id());

            if (controller->mode == THERMOSTAT)
            {
                controller->min_target_temperature = CONFIG_TARGET_TEMPERATURE_THERMOSTAT_MIN;

                controller->max_target_temperature = CONFIG_TARGET_TEMPERATURE_THERMOSTAT_MAX;
            }
            else
            {
                controller->min_target_temperature = CONFIG_TARGET_TEMPERATURE_TITAN_MIN;

                controller->max_target_temperature = CONFIG_TARGET_TEMPERATURE_TITAN_MAX;
            }
            break;

        case CONTROLLER_EVENT_HAVE_HOMEKIT_PAIR_CHANGED:

            if (!controller->have_homekit_pair)
            {
                controller->have_homekit_pair = notification.bool_value;

//                sysparam_set_bool("have_homekit_pair", notification.bool_value);

                if (controller->have_homekit_pair)
                {
                    controller_display(controller, DISPLAY_TEMP);
//                    startCheckUpdate();
                }
                else
                {
                    controller_display(controller, DISPLAY_HK_LOADING);
                }
            }
//            if (!controller->have_homekit_pair && notification.bool_value)
//            {
//                controller_display(controller, DISPLAY_TEMP);
//                startCheckUpdate();
//            }
//            else if (!controller->have_homekit_pair && !notification.bool_value)
//            {
//                if (controller->active_screen != CONTROLLER_SCREEN_LOADING)
//                {
//                    controller_display(controller, DISPLAY_HK_LOADING);
//                }

//            }

//            controller->have_homekit_pair = notification.bool_value;

//            sysparam_set_bool("have_homekit_pair", notification.bool_value);

            break;

        case CONTROLLER_EVENT_CURRENT_TEMPERATURE_CHANGED:
            controller->current_temperature = notification.float_value;
            controller_update_state(controller);
            break;

        case CONTROLLER_EVENT_TARGET_TEMPERATURE_CHANGED:

            controller->target_temperature =
                    MIN(controller->max_target_temperature,
                        MAX(controller->min_target_temperature,
                            notification.float_value));

//            my_homekit_set_target_temperature(controller->target_temperature);
//            sysparam_set_int8("target_temperature", (int8_t)controller->target_temperature);
            controller_update_state(controller);
            break;

        case CONTROLLER_EVENT_TARGET_STATE_CHANGED:
            printf("\n");
            printf("event target ts %i int %i save %i \n", controller->target_state, notification.int_value, notification.save);

            controller->target_state = MIN(1, MAX(0, notification.int_value));

            printf("event target ts %i \n", controller->target_state);
            if (notification.save)
            {
                printf("event target 1 \n");
//                printf("event target set %i \n", sysparam_set_bool("target_state", controller->target_state));
            }
            else
            {
                printf("event target 2 \n");
                if (notification.int_value > 0)
                {
                    printf("event target ts 2.1 \n");
//                    printf("event target get %i \n", sysparam_get_bool("target_state", &controller->target_state));
                }
            }
//            my_homekit_set_target_state(controller->target_state);
            controller_update_state(controller);
            printf("\n");
            break;

        case CONTROLLER_EVENT_SENSOR_TYPE:
//            sysparam_set_int8("sensor_type", notification.int_value);
            controller->sensor_type = notification.int_value;
            controller_display_sensor_type(controller);
            controller_update_state(controller);
            sendTemperature();
            break;


        case CONTROLLER_EVENT_HYSTERESIS:
            controller->hysteresis = MIN(5, MAX(1, notification.int_value));
//            sysparam_set_int8("hysteresis", controller->hysteresis);
            controller_display_hysteresis(controller);
            controller_display_hist_shift_label(controller);
            controller_update_state(controller);
            break;

        case CONTROLLER_EVENT_TEMP_SHIFT:
            controller->temp_shift = MIN(5, MAX(-5, notification.float_value));
//            sysparam_set_int32("temp_shift", (int)(controller->temp_shift*10));
            controller_display_temp_shift(controller);
            controller_display_hist_shift_label(controller);
            controller_update_state(controller);
            sendTemperature();
            break;

        case CONTROLLER_EVENT_WIFI_SIGNAL:
            controller->wifi_signal = notification.int_value;
            controller_display_wifi_signal(controller);
            break;

        case CONTROLLER_EVENT_UPTIME:
            controller->uptime = notification.int_value;
            controller_display_uptime(controller);
            break;

        case CONTROLLER_EVENT_MEMORY:
            controller->memory = notification.int_value;
            controller_display_memory(controller);
            break;

        case CONTROLLER_EVENT_QR_HOMEKIT:
            if (controller->qr_homekit)
                free(controller->qr_homekit);
            controller->qr_homekit = notification.data;
            controller_display_qr_homekit(controller);
            break;

        case CONTROLLER_EVENT_UPDATE_PERCENT:
            controller->update_percent = notification.int_value;
            if (controller->update_percent == 0)
            {
                controller->update_started = false;
                controller_display_update_percent(controller);

//                bool need_update = false;
//                if ((sysparam_get_bool("need_update", &need_update) == SYSPARAM_OK) && (need_update))
//                {
//                    controller_display(controller, DISPLAY_HK_LOADING);
//                    sdk_system_restart();
//                    sdk_system_deep_sleep(500000);
//              }
//              else
//              {
//                  controller_display(controller, DISPLAY_RESTORE);
//              }
            }
            else
                if (controller->update_percent < 101)
                {
                    controller_display_update_percent(controller);
                }
                else
                {
//                    sysparam_set_bool("need_update", false);
                    controller_display(controller, DISPLAY_HK_LOADING);
                }
            break;

        case CONTROLLER_EVENT_UPDATE_AVAILABLE:
            //move to display
            controller->update_available = notification.bool_value;
            if (notification.bool_value)
            {
                controller_display(controller, DISPLAY_UPDATE_ON);
            }
            else
            {
                controller_display(controller, DISPLAY_UPDATE_OFF);
            }
            break;

        case CONTROLLER_EVENT_WIFI_CONNECT:

            if (controller->wifi_connected != notification.bool_value)
            {//Status changed

                if (!controller->wifi_connected && notification.bool_value) //connected
                {
//                    bool need_update = false;
//                    if ((sysparam_get_bool("need_update", &need_update) == SYSPARAM_OK) && (need_update))
//                    {
//                        controller_display(controller, DISPLAY_UPDATE_PAGE);
//                        controller_display_update_percent(controller);
//                        controller->update_started = true;
//                        startUpdate();
//                    }
//                    else
//                    {
//                        my_homekit_init(controller_get_name());
//
//                        if (controller->have_homekit_pair)
//                        {
//                            controller_display(controller, DISPLAY_TEMP);
//                            startCheckUpdate();
//                        }
//                        else
//                        {
//                            controller_display(controller, DISPLAY_SENSOR_PAGE);
//                        }
//                    }
                }
                else //disconnected
                {
                    //                    controller_display(controller, DISPLAY_WIFI_PAGE);

                }
            }
            else
            {//Status don`t change

//                char *ssid = wifi_config_get_ssid();
//
//                if (ssid != NULL)
//                {
//                    free(ssid);
//                    controller_display(controller, DISPLAY_HK_LOADING);
//                }
//                else
//                {
//                    controller_display(controller, DISPLAY_CONF);
//                }

            }

            controller->wifi_connected = notification.bool_value;

            break;
        }
    }
}


#include "main.h"

#include <stdlib.h>
#include <stdio.h>

#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>

#include <FreeRTOS.h>
#include <esp8266.h>
#include <esp/uart.h>
#include <esplibs/libmain.h>
#include <task.h>
#include <etstimer.h>


#include "config.h"
#include "controller.h"
#include "temperature_sensor.h"
#include "update.h"

static char *ssid;

void app_main()
{
#if CONFIG_THERMOSTAT_DEBUG
    uart_set_baud(0, 115200);
#else
    gpio_enable(CONFIG_WATCHDOG_GPIO, GPIO_OUTPUT);
    gpio_write(CONFIG_WATCHDOG_GPIO, false);
    xTaskCreate(&watchdog_task, "wd", 256, NULL, 1, NULL);
#endif

    gpio_enable(CONFIG_WATCHDOG_ENABLE_GPIO, GPIO_OUTPUT);
    gpio_write(CONFIG_WATCHDOG_ENABLE_GPIO, true);

    printf("Free Heap %i \n", xPortGetFreeHeapSize());

    printf("VERSION %i \n", GIT_VERSION);

    heater_init();

    controller_start();

    controller_notify_wifi_signal(-1);

    temperature_sensor_init();
    //        wifi_config_set("Mi Phone", "homeswapass");

//    wifi_config_set("MySweetHome", "homeswapass");

    //        wifi_config_reset();

    wifi_config_init2(controller_get_name(), WIFI_CONFIG_PASSWORD, on_wifi_event);
}


void on_wifi_event(wifi_config_event_t event)
{
    if (event == WIFI_CONFIG_CONNECTED)
    {
        printf("Connected to WiFi\n");

        ssid = wifi_config_get_ssid();

        if (ssid != NULL)
        {
            xTaskCreate(&wifiscan_task, "WIFISCAN", 256, NULL, 1, NULL);
        }
        controller_notify_wifi_connect(true);
    }
    else if (event == WIFI_CONFIG_DISCONNECTED)
    {
        printf("Disconnected from WiFi\n");

        controller_notify_wifi_connect(false);
    }
    else if (event == WIFI_CONFIG_CONNECTING)
    {
        printf("WIFI_CONFIG_SUCCESS from WiFi\n");

        controller_notify_wifi_connect(false);

//        sdk_system_restart();
        sdk_system_deep_sleep(500000);
    }
}


static void wifi_scan_callback(void *arg, sdk_scan_status_t status)
{
    if (status != SCAN_OK)
    {
        return;
    }

    struct sdk_bss_info *bss = (struct sdk_bss_info *)arg;

    bss = bss->next.stqe_next;

    bool found = false;

    while (bss)
    {
        if (ssid != NULL && strcmp(bss->ssid, ssid) == 0)
        {
            int signal = bss->rssi * -1;
            controller_notify_wifi_signal(signal < 50 ? 4 : signal < 60 ? 3 : signal < 70 ? 2 : 1);
            found = true;
            break;
        }
        bss = bss->next.stqe_next;
    }

    if (!found)
    {
        controller_notify_wifi_signal(-1);
    }

}


static void wifiscan_task(void *pvParameters)
{
    while (true)
    {
        printf("Free Heap %i \n", xPortGetFreeHeapSize());

        controller_notify_memory((int)xPortGetFreeHeapSize());
        controller_notify_uptime(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);

        if ((int) xPortGetFreeHeapSize() < 2500)
        {
//            sdk_system_restart();
            sdk_system_deep_sleep(500000);
        }

        if (!controller_get_have_homekit_pair())
        {
            vTaskDelayMs(5000);
            continue;
        }
        if (sdk_wifi_get_opmode() == STATIONAP_MODE || sdk_wifi_get_opmode() == STATION_MODE)
        {
            sdk_wifi_station_scan(NULL, wifi_scan_callback);
        }
        vTaskDelayMs(5000);
    }

    vTaskDelete(NULL);
}


#if !CONFIG_THERMOSTAT_DEBUG
static void watchdog_task(void *pvParameters)
{
    while (true)
    {

        if (sdk_wifi_get_opmode() == STATIONAP_MODE || sdk_wifi_get_opmode() == STATION_MODE)
        {
            sdk_wifi_station_scan(NULL, wifi_scan_callback);
        }

        gpio_toggle(CONFIG_WATCHDOG_GPIO);

        vTaskDelayMs(1000);
    }

    vTaskDelete(NULL);
}
#endif



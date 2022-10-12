#ifndef CONFIG_THERMOSTAT_DEBUG
#define CONFIG_THERMOSTAT_DEBUG 0
#endif

#define GIT_VERSION 10

#ifdef CONFIG_THERMOSTAT_DEBUG
#define ERROR(message, ...) printf("!!!: " message "\n", ##__VA_ARGS__)
#define DEBUG(message, ...) printf(">>>: " message "\n", ##__VA_ARGS__)
#else
#define ERROR(message, ...)
#define DEBUG(message, ...)
#endif

#define CONFIG_MANUFACTURER "Triquetras"
#define CONFIG_FIRMWARE_REVISION "1"
#define TEMPLATE_NAME "Triquetras-%i"
#define WIFI_CONFIG_PASSWORD "12345678"

#define CONFIG_TEMPERATURE_POLL_PERIOD_MS 10000
#define CONFIG_TEMPERATURE_DIGIT_SENSOR_GPIO 2
#define CONFIG_TEMPERATURE_SENSOR_SWITCH_GPIO 0

#define CONFIG_DISPLAY_TX_GPIO 4
#define CONFIG_DISPLAY_RX_GPIO 5

#define CONFIG_TARGET_TEMPERATURE_THERMOSTAT_MIN 18
#define CONFIG_TARGET_TEMPERATURE_THERMOSTAT_MAX 40

#define CONFIG_TARGET_TEMPERATURE_TITAN_MIN 7
#define CONFIG_TARGET_TEMPERATURE_TITAN_MAX 75

#define CONFIG_HEATER_HYSTERESIS 1.0

#define CONFIG_SENSOR_COUNT 11

#define CONFIG_HEATER_RELAY_GPIO 15
#define CONFIG_HEATER_RELAY_ACTIVE 1

#define CONFIG_WATCHDOG_GPIO 1
#define CONFIG_WATCHDOG_ENABLE_GPIO 16

#define WIFI_CONFIG_CONNECT_TIMEOUT 1000

#define RESISTOR_T 2200 // 2.2kOm

#define vTaskDelayMs(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)

#define MIN(a, b) ((b) < (a) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define DISPLAY_BKCMD "bkcmd=0"

#define DISPLAY_LOGO "logo_click.val=1"

#define DISPLAY_TEMP "temp_click.val=1"

#define DISPLAY_UPDATE_PAGE "upd_click.val=1"

#define DISPLAY_RESTORE "restore_click.val=1"

#define DISPLAY_CONF "config_click.val=1"

#define DISPLAY_HK_PAGE "homekit_click.val=1"

#define DISPLAY_WIFI_PAGE "wifi_click.val=1"

#define DISPLAY_HK_LOADING "loading_click.val=1"

#define DISPLAY_HK "logo_page.homekit.val=1"

#define DISPLAY_MODE "debug_page.mode.val=%d"

#define DISPLAY_LOCK_ON "temperature.lock_mode.val=1"

#define DISPLAY_LOCK_OFF "temperature.lock_mode.val=0"

#define DISPLAY_QR_HOMEKIT "qr_hk.txt=\"%s\""

#define DISPLAY_QR_WIFI "qr_wifi.txt=\"WIFI:S:%s;T:WPA;P:12345678;;\""

#define DISPLAY_TEMP_SHIFT "sensor_corr.txt=\"%.1f\""

#define DISPLAY_HYST "hysteresis.txt=\"%d\""

#define DISPLAY_CONF_TEMP_SHIFT "sensor_corr.txt=\"%.1f\""

#define DISPLAY_CONF_HYST "hysteresis.txt=\"%d\""

#define DISPLAY_SENSOR_TYPE "active_sensor.val=%d"

#define DISPLAY_UPTIME "uptime.txt=\"%s\""

#define DISPLAY_MEMORY "memory.txt=\"%s\""

#define DISPLAY_SIGNAL "signal_fill.val=%d"

#define DISPLAY_ARROW "arrow_temp.val=%d"

#define DISPLAY_H_ACTIVE "H_active.val=%d"

#define DISPLAY_DEGREE_BIG "degree_big.txt=\"%.1f\""

#define DISPLAY_DEGREE "degree.txt=\"%.1f\""

#define DISPLAY_HIST_SHIFT_LABEL "settings.hyster_label.txt=\"%d; %.1f\""

#define DISPLAY_VERSION "esp_version.txt=\"%s\""

#define DISPLAY_NEW_VERSION "esp_next_vers.txt=\"%s\""

#define DISPLAY_UPDATE_ON "firmware_upd.is_update.val=1"

#define DISPLAY_UPDATE_OFF "firmware_upd.is_update.val=0"

#define DISPLAY_UPDATE_STATUS "update.val=%d"

#define DISPLAY_UPDATE_PERCENT "percent.txt=\"%d\""

#define DISPLAY_SENSOR_PAGE "sensor_click.val=1"

#define DISPLAY_SENSOR_NAME "settings.sensor_label.txt=sensor_page.sensor_%x.txt"

#define DISPLAY_SENSOR_NAME_PAGE_1 "settings.sensor_label.txt=sensor_page1.sensor_%x.txt"

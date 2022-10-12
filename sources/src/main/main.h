#pragma once

#include "esp_wifi.h"

void on_wifi_event(WIFI_EVENT event);

static void wifiscan_task(void *pvParameter);

static void watchdog_task(void *pvParameter);

static void wifi_scan_callback(void *arg, sdk_scan_status_t status);

#define MAX(a, b) ((a) < (b) ? (b) : (a))

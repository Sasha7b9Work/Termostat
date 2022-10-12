#pragma once

#include "wifi_config.h"

void on_wifi_event(wifi_config_event_t event);

static void wifiscan_task(void *pvParameter);

static void watchdog_task(void *pvParameter);

static void wifi_scan_callback(void *arg, sdk_scan_status_t status);

#define MAX(a, b) ((a) < (b) ? (b) : (a))

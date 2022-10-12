#include "update.h"
#include "config.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "http_client_ota.h"

#include "controller.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

static bool checkUpdateStarted = false;
static TaskHandle_t CheckTask;

static void percent_ota(uint8_t percent)
{
    controller_notify_update_percent(percent);
}

static ota_info info = {
    .server      = "update.lytko.com",
    .port        = "80",
    .binary_path = NULL,
    .sha256_path = NULL,
    .ota_cb      = percent_ota,
    .check       = false
};

static ota_info check_info = {
    .server      = "update.lytko.com",
    .port        = "80",
    .binary_path = NULL,
    .check       = true
};

static void ota_error_handling(OTA_err err) {
    printf("Error: ");

    switch(err) {
    case OTA_DNS_LOOKUP_FALLIED:
        printf("DNS lookup has fallied");
        break;
    case OTA_SOCKET_ALLOCATION_FALLIED:
        printf("Impossible allocate required socket");
        break;
    case OTA_SOCKET_CONNECTION_FALLIED:
        printf("Server unreachable, impossible connect");
        break;
    case OTA_SHA_DONT_MATCH:
        printf("Sha256 sum does not fit downloaded sha256");
        break;
    case OTA_REQUEST_SEND_FALLIED:
        printf("Impossible send HTTP request");
        break;
    case OTA_DOWLOAD_SIZE_NOT_MATCH:
        printf("Download size don't match with server declared size");
        break;
    case OTA_ONE_SLOT_ONLY:
        printf("rboot has only one slot configured, impossible switch it");
        break;
    case OTA_FAIL_SET_NEW_SLOT:
        printf("rboot cannot switch between rom");
        break;
    case OTA_IMAGE_VERIFY_FALLIED:
        printf("Dowloaded image binary checsum is fallied");
        break;
    case OTA_UPDATE_DONE:
        printf("Ota has completed upgrade process, all ready for system software reset");
        break;
    case OTA_HTTP_OK:
        printf("HTTP server has response 200, Ok");
        break;
    case OTA_HTTP_NOTFOUND:
        printf("HTTP server has response 404, file not found");
        break;
    }
    printf(" (%i) \n", err);
}

static void ota_task(void *PvParameter)
{
    printf("ota_task\n");
    if (CheckTask != NULL)
        vTaskSuspend(CheckTask);

    printf("vTaskSuspend check\n");
    while (1)
    {

        if (!controller_get_have_homekit_pair())
        {
            vTaskDelayMs(60000*10);
            continue;
        }

        OTA_err err = ota_update((ota_info *) PvParameter);

        if (err != OTA_HTTP_OK && err != OTA_UPDATE_DONE)
        {
            ota_error_handling(err);
            controller_notify_update_percent(0);
            printf("ota_task break \n");
            break;
        }

        if(err != OTA_UPDATE_DONE) {
            vTaskDelayMs(1000);
            continue;
        }

        controller_notify_update_percent(255);

        printf("UPDATED! \n\n\n");

        vTaskDelayMs(5000);

//        sdk_system_restart();
        sdk_system_deep_sleep(500000);
    }

    printf("CheckTask check\n");
    if (CheckTask != NULL)
        vTaskResume(CheckTask);
    vTaskDelete(NULL);
}

void check_ota()
{

    if (!controller_get_have_homekit_pair())
    {
        return;
    }
    printf("check_ota\n");
    if (CheckTask != NULL)
        vTaskSuspend(CheckTask);
    OTA_err err = ota_update(&check_info);

    printf("check_ota_task %i \n", err);
    if (err != OTA_HTTP_OK)
    {
        ota_error_handling(err);
        controller_notify_update_available(false);
    }
    else
    {
        controller_notify_update_available(true);
    }
    printf("check_ota end\n");
    if (CheckTask != NULL)
        vTaskResume(CheckTask);
}

static void check_ota_task(void *PvParameter)
{
    while (1)
    {

        if (!controller_get_have_homekit_pair())
        {
            vTaskDelayMs(60000*10);
            continue;
        }

        OTA_err err = ota_update((ota_info *) PvParameter);

        printf("check_ota_task %i \n", err);
        if (err != OTA_HTTP_OK)
        {
            ota_error_handling(err);
            controller_notify_update_available(false);
        }
        else
        {
            controller_notify_update_available(true);
        }

        vTaskDelayMs(60000*60);

    }

    vTaskDelete(NULL);
}


void startCheckUpdate()
{
    if (checkUpdateStarted)
    {
        return;
    }
    checkUpdateStarted = true;
    size_t size;

    size = snprintf(NULL, 0, "/homekit/%03d/HK_Thermostat.bin", GIT_VERSION+1);
    check_info.binary_path = malloc(size);
    snprintf(check_info.binary_path, size+1, "/homekit/%03d/HK_Thermostat.bin", GIT_VERSION+1);

    printf("CheckUpdate starting\n");
    /*
    size = snprintf(NULL, 0, "/homekit/%03d/HK_Thermostat.bin.sha256", GIT_VERSION+1);
    check_info.sha256_path = malloc(size);
    snprintf(check_info.sha256_path, size+1, "/homekit/%03d/HK_Thermostat.bin.sha256", GIT_VERSION+1);
    */

    xTaskCreate(check_ota_task, "check_ota_task", 512, &check_info, 2, &CheckTask);
}

void startUpdate()
{
    size_t size;

    size = snprintf(NULL, 0, "/homekit/%03d/HK_Thermostat.bin", GIT_VERSION+1);
    info.binary_path = malloc(size);
    snprintf(info.binary_path, size+1, "/homekit/%03d/HK_Thermostat.bin", GIT_VERSION+1);

    size = snprintf(NULL, 0, "/homekit/%03d/HK_Thermostat.bin.sha256", GIT_VERSION+1);
    info.sha256_path = malloc(size);
    snprintf(info.sha256_path, size+1, "/homekit/%03d/HK_Thermostat.bin.sha256", GIT_VERSION+1);

    printf("Update starting\n");
    printf("info.binary_path %s\n", info.binary_path);
    printf("info.sha256_path %s\n", info.sha256_path);
    BaseType_t status = xTaskCreate(ota_task, "ota_task", 3072, &info, 2, NULL);
    if (pdPASS != status)
    {
        printf("Error start update %i\n", status);
        controller_notify_update_percent(0);
    }
}



void startUpdateRestore()
{

    info.binary_path = "/homekit/restore/HK_Thermostat.bin";
    info.sha256_path = "/homekit/restore/HK_Thermostat.bin.sha256";

    printf("Update starting\n");
    printf("info.binary_path %s\n", info.binary_path);
    printf("info.sha256_path %s\n", info.sha256_path);
    BaseType_t status = xTaskCreate(ota_task, "ota_task", 3072, &info, 2, NULL);
    if (pdPASS != status)
    {
        printf("Error update %i\n", status);
        controller_notify_update_percent(0);
    }
}

void startUpdateSpecial()
{
    info.binary_path = "/homekit/special/HK_Thermostat.bin";
    info.sha256_path = "/homekit/special/HK_Thermostat.bin.sha256";

    printf("Update starting\n");
    printf("info.binary_path %s\n", info.binary_path);
    printf("info.sha256_path %s\n", info.sha256_path);
    BaseType_t status = xTaskCreate(ota_task, "ota_task", 3072, &info, 2, NULL);
    if (pdPASS != status)
    {
        printf("Error update %i\n", status);
        controller_notify_update_percent(0);
    }
}


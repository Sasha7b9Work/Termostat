PROGRAM = HK_Thermostat

PROGRAM_SRC_DIR = src
LIBS ?= gcc hal m

EXTRA_COMPONENTS = \
    extras/http-parser \
    extras/dhcpserver \
    extras/onewire \
    extras/ds18b20 \
    extras/softuart \
    extras/rboot-ota \
    extras/mbedtls \
    extras/http_client_ota \
    extras/esp-wolfssl \
    extras/esp-cjson \
    extras/esp-homekit \
    extras/esp-wifi-config \

EXTRA_CFLAGS += -I../.. -DHOMEKIT_SHORT_APPLE_UUIDS

HOMEKIT_SPI_FLASH_BASE_ADDR ?= 0x180000

EXTRA_CFLAGS += -DMAX_ROMS=2

EXTRA_CFLAGS += -DCONFIG_THERMOSTAT_DEBUG=1 -DOTA_DEBUG

EXTRA_CFLAGS += -DHOMEKIT_DEBUG="\"$(CONFIG_THERMOSTAT_DEBUG)\""

EXTRA_CFLAGS += -DWIFI_CONFIG_DEBUG="\"$(CONFIG_THERMOSTAT_DEBUG)\""



include $(SDK_PATH)common.mk

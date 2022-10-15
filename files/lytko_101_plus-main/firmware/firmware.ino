#include <H4.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266SSDP.h>
#include <ESP8266httpUpdate.h>
#include <ESPAsyncDNSServer.h>
#include <ESPAsyncUDP.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>  // https://github.com/me-no-dev/ESPAsyncWebServer
#include <AsyncJson.h>
#include <LittleFS.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <PangolinMQTT.h>
#include <arduino_homekit_server.h>
#include <time.h>
#include <sys/time.h>

#include "device_config.h"
#include "dwin.h"
#include "log.h"
#include "wdt.h"
#include "update_dwin.h"

#include "fetch_update_info.h"

#include "HTU21D.h"
HTU21D htu;

static timeval tv;
static timespec tp;
static time_t now;
tm *tm;
String fileName = "0.csv";

bool dwin = true;

byte news;
bool updated = true;
String ver = "04.03.07";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

PangolinMQTT mqttClient;

AsyncDNSServer dns;
const byte DNS_PORT = 53;

AsyncUDP udp;
uint8_t mac[6];

byte wifi_scanned = 0;
byte wifi_con = 0;

bool spaWork = true;
bool spaTemp = false;

bool webSocketConnect = false;
bool lytko = false;

bool req = false;

String dataJson = "{}";     // Здесь все данные
String configSetup = "{}";  // Здесь данные для setup
String int_ds_addr = "{}";
String ScheduleSetup = "{\"schedule.json\":\"\"}";
String wifijson = "{\"wifi_networks\": []}";
String first = "{}";
String configWifi = "{\"ssidPass\":\"\",\"ssid\":\"\"}";  // Здесь wifi
String spasetup = "{}";
String addressList = "{\"ssdpList\":[]}";
String signals = "";
String update_progress = "{}";
String mqtttopics = "{}";
String mqtts = "{}";
String mqtt_data = "{}";

bool firstOn = false;
byte interval = 0;

// Spa
byte d;
byte work;
float hysteresis = 1;
float sensor_correction = 0;
byte presetMin = 15;
byte presetMax = 30;
typedef struct Schedule {
  byte hourUp = 0;
  byte minUp = 0;
} schedule;

schedule scheds[2][4];
float thermo;
bool changed = false;

float last_temp;
float target_temp;
bool rel = 0;

signed char tempSensorHealth = 100;
#define FILTER_SMA_ORDER 3
float Filter_Buffer[FILTER_SMA_ORDER] = {
  0,
};
float current_temp[4] = { 25.0, 25.0, 25.0, 25.0 };
float temp_;
float tcase;
byte temp_index = 0;

typedef struct ext {
  byte is = 0;
  bool ds_htu = 0;  //ds = 0; htu = 1;
  float temp = 0;
  float last = 0;
  long stime = 0;
} sensor;

sensor ext_sens, int_sens;
bool firstMin = false;
byte secs;
byte sec10 = 0;
byte ONE_WIRE_BUS = 2;
byte ds = 1;
uint16_t R0, B;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempSensor, intDS;

bool wifiSTA = true;

String devname, devnamemqtt;
String namemqtt;
String idmqtt;
bool mqtt_needToConnect = false;
bool have_mqtt = false;
bool have_mqtt_external_topic = false;
bool mqtt_connect = false;
unsigned long time_disconnect;

bool mqtt_update = false;

bool is_target_temp_first;
byte num1m = 4;
byte num5m = 0;

uint16_t ssdpSize = 0;

byte numCon = 0;
long apModeTime = 0;

int prog = 0;
bool router = false;

bool hk_started;
byte homekit;
byte pair_hk;

String timeZoneStr = "";
String devnameAlice;
bool needAlice = false;
bool alice_needToConnect = false;
byte alice = 0;
String valToHeat;

uint32_t dwin_did;
uint32_t dwin_restart_time = 0;

struct {
  bool update_esp : 1;
  bool update_dwin : 1;
  bool restart_esp : 1;
  bool restart_dwin : 1;
  int unused : 4;
} lazy_actions;

H4 h4 = H4();

// =============================================================================
// HomeKit

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t name;
extern "C" homekit_characteristic_t serial_number;
extern "C" homekit_characteristic_t current_temperature;
extern "C" homekit_characteristic_t target_temperature;
extern "C" homekit_characteristic_t current_state;
extern "C" homekit_characteristic_t target_state;
extern "C" char SETUP_ID[4];
extern "C" void accessory_init();

void h4setup() {
  Wdt::setup();
  Wdt::enable();
  WiFi.scanNetworks(false, false);
  LittleFS.begin();

  if (i2cScanner() > 0) {
    initHTU21();
  }

  configSetup = readFile("config.json", 1024);
  int_ds_addr = readFile("intDS.json", 1024);
  ScheduleSetup = readFile("schedule.json", 1024);
  signals = readFile("signals.json", 1024);
  spasetup = readFile("spa.json", 1024);
  configWifi = readFile("wifi.save.json", 100);

  calculate_constants();

  devname = jsonRead(configSetup, "name");
  homekit = jsonReadtoInt(configSetup, "homekit");
  pair_hk = jsonReadtoInt(configSetup, "pair_hk");

  if (jsonRead(configSetup, "alice_login") != NULL) {
    needAlice = true;
    alice = 255;
  }

  is_target_temp_first = jsonReadtoInt(configSetup, "is_target_temp_first");

  tempInit();

  delay(2000);
  dwin_init();

  if (homekit != 2) {
    MQTT_init();
  }

  scanWiFi();
  WIFIinit(wifiSTA);

  SSDP_init();
  requestSSDP();

  WEB_init();
  spaInit();
  GetSchedule();
  spaWorks();

  timeZoneStr = jsonRead(configSetup, "ts");
  if (timeZoneStr == NULL) timeZoneStr = "1633453200";
  set_clock();

  update_actual();

  if (homekit) {
    dwin_hk_has_pair(pair_hk == 1);
    homekit_setup();
  }

  if (homekit != 2 && have_mqtt) {
    connectToMqtt();
  }

  if (firstOn) {
    dwin_command(DwinCommand::FirstScreen);
  } else {
    dwin_command(DwinCommand::MainScreen);
  }

  h4.nTimes(4, 2000, cycle5s);

  h4.every(5, dwin_task);
  h4.every(100, myCallback);
  h4.every(10000, cycle);
  h4.every(60000, cycle1m);
  h4.every(112000, requestSSDP);
  h4.every(3600000, update_actual);

  if (pair_hk == 1) {
    h4.every(15000, homekit_announce_MDNS);
  }
}

void cycle() {
  h4.once(1000, cycle5s);
  h4.once(2000, tempWorks);
  h4.once(3000, spaRegulation);
  h4.once(4000, spaWorks);
  h4.once(5000, []() {
    sec10++;
    if (sec10 == 6) {
      sec10 = 0;
      ESP.getFreeHeap();
      sendDataWs(true, 0);
      if (mqtt_needToConnect) h4.once(50, cycle1m);
    } else sendDataWs(false, 0);
  });
  h4.once(6000, check_wifi);
}

void dwin_task() {
  if (dwin) {
    dwin_sync();
    dwin_lifecycle();
  }
}

void myCallback() {
  if (homekit) {
    homekit_loop();
    homekit_sync();
  }

  if (needAlice && homekit != 0) {
    getAlice(alice);
  }

  // ===========================================================================

  // Обновление термостата
  if (lazy_actions.update_esp) {
    esp8266_update();
    lazy_actions.update_esp = false;
  }

  // Обновление экрана
  if (lazy_actions.update_dwin) {
    dwin_update();
    lazy_actions.update_dwin = false;
  }

  // Перезагрузка термостата
  if (lazy_actions.restart_esp) {
    dwin_command(DwinCommand::LoadingScreen);

    PORT_HIGH(0);
    LittleFS.end();
    ESP.restart();

    lazy_actions.restart_esp = false;
  }

  // Перезагрузка экрана
  // Время ожидания 20 секунд подобрано на основе наблюдений
  if (lazy_actions.restart_dwin) {
    if ((millis() - dwin_restart_time) > (20 * 1000)) {
      dwin_command(DwinCommand::MainScreen);
      lazy_actions.restart_dwin = false;
      dwin_restart_time = 0;
    }
  }
}

void homekit_setup() {
  accessory_init();
  int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X", name.value.string_value, mac[3], mac[4], mac[5]);
  char *name_value = (char *)malloc(name_len + 1);
  char *serial_value = (char *)malloc(6 + 1);
  snprintf(name_value, name_len + 1, "%s_%02X%02X%02X", name.value.string_value, mac[3], mac[4], mac[5]);
  snprintf(serial_value, 6 + 1, "%02X%02X%02X", mac[3], mac[4], mac[5]);
  name.value = HOMEKIT_STRING_CPP(name_value);
  target_temperature.min_value[0] = presetMin;
  target_temperature.max_value[0] = presetMax;
  serial_number.value = HOMEKIT_STRING_CPP(serial_value);
  arduino_homekit_setup(&config);
  hk_started = true;
}

void homekit_loop() {
  arduino_homekit_loop();

  if (pair_hk == 127) {
    sendWs("loading", 1);
    dwin_hk_loading_screen();
    pair_hk = 0;
  }
}

void homekit_sync() {
  if (news & 2) {
    if (target_temp != target_temperature.value.float_value) {
      ChangeSchedule(0, target_temperature.value.float_value);
    }
    news &= ~2;
  }

  if (news & 1) {
    sendDataWs(1, 0);
    changeSpa(spaWork);
    news &= ~1;
  }
}

void cycle5s() {
  if (homekit != 0) {
    if (homekit == 1 && pair_hk >= 8 && pair_hk < 11) {
      pair_hk++;
    }

    if (pair_hk == 254) {
      dwin_hk_has_pair(1);
      configSetup = jsonWrite(configSetup, "pair_hk", "1");
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      pair_hk = 1;
    }

    if (pair_hk == 255) {
      pair_hk = 254;
      dwin_command(DwinCommand::MainScreen);
    }

    if (pair_hk == 128) {
      dwin_hk_has_pair(0);
      configSetup = jsonWrite(configSetup, "pair_hk", "0");
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      pair_hk = 0;
    }
  }
}

void cycle1m() {
  if (wifiSTA && mqtt_needToConnect && !mqtt_connect && have_mqtt) {
    connectToMqtt();
  }
  if (wifiSTA && alice_needToConnect) alice_init();  //
  if (ext_sens.is) {
    long etime = millis();
    if (etime - ext_sens.stime > 6000000) {
      if (ext_sens.temp == ext_sens.last) {
        ext_sens.temp = 0;
        ext_sens.is = 0;
      }
    }
    if (etime - ext_sens.stime > 5880000) {
      ext_sens.last = ext_sens.temp;
    }
  }
  if (int_sens.is == 1 && int_sens.ds_htu == 1) {
    htu.requestTemperature();
    h4.once(90, []() {
      htu.readTemperature();
      htu.requestHumidity();
      h4.once(30, []() {
        htu.readHumidity();
      });
    });
  }
}

void dwin_sync() {

  // Уровень WiFi
  int wifi_level = 2 * (WiFi.RSSI() + 100);
  if (wifi_level == 262) {
    dwin_set_wifi_strength(0);
  } else {
    dwin_set_wifi_strength(constrain(wifi_level / 20, 0, 4));
  }

  // Температура
  dwin_set_temps(last_temp, target_temp);

  // Нагрев
  dwin_set_active(spaWork);

  // Работа реле
  dwin_set_heating(spaRelayOnOff());

  // Порядок температур на экране
  dwin_set_is_target_temp_first(is_target_temp_first);

  // Гистерезис
  float hysteresis = jsonRead(configSetup, "hysteresis").toFloat();
  dwin_set_hysteresis(hysteresis);

  // Выбранный датчик температуры
  int sensor_id = jsonRead(configSetup, "sensor_model_id").toInt();
  dwin_set_sensor_index(sensor_id);

  // Корректировка температуры
  float sensor_adj = jsonRead(configSetup, "sensor_corr").toFloat();
  dwin_set_temp_adj(sensor_adj);

  // Минимальная-максимальная целевая температура
  dwin_set_target_min_max(presetMin, presetMax);

  // Версии прошивки
  dwin_update_set_versions(ver, esp8266_next_version());
}

void calculate_constants() {
  uint32_t x = ESP.getChipId();
  x ^= x >> (32 - 3);
  dwin_did = (x * 2654435769) >> (32 - 3);
}


// =============================================================================
// Проверка обновлений

static void check_thermostat_new_version();
static void check_screen_new_version();

void update_actual() {
  check_thermostat_new_version();
  check_screen_new_version();
}

void check_thermostat_new_version() {
  auto thermostat_update_info = fetch_thermostat_update_info();
  esp8266_update_info(thermostat_update_info);

  dwin_has_update(thermostat_update_info.has_update ? 1 : 0);

  if (!thermostat_update_info.has_error && thermostat_update_info.has_update) {
    timeZoneStr = thermostat_update_info.ts;
  }
}

void check_screen_new_version() {
  UpdateInfo screen_update_info = fetch_dwin_update_info();
  dwin_update_info(screen_update_info);
}
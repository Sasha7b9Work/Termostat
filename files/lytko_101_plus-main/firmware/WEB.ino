#include "log.h"

#include "checked.h"
#include "favicon.h"
#include "index.h"
#include "index2.h"
#include "noto_sans.h"
#include "qrcode.h"
#include "script_menu_esp.h"
#include "setup.h"
#include "site.h"


/*
   =============================================================================
   Вспомогательные функции
   =============================================================================
*/

boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }

  return true;
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

String uptimeString() {
  char uptime[32];
  char zero[] = "0";
  char nop[]  = "";
  char* zh;
  char* zm;
  char* zs;

  unsigned long totalSeconds = millis() / 1000;
  unsigned long days = totalSeconds / 86400;
  unsigned long tsHours = totalSeconds - days * 86400;
  unsigned long hours = tsHours / 3600;
  unsigned long tsMinutes = tsHours - hours * 3600;
  unsigned long minutes = tsMinutes / 60;
  unsigned long seconds = tsMinutes - minutes * 60;

  if (hours < 10) {
    zh = zero;
  } else {
    zh = nop;
  }

  if (minutes < 10) {
    zm = zero;
  } else {
    zm = nop;
  }

  if (seconds < 10) {
    zs = zero;
  } else {
    zs = nop;
  }

  sprintf(uptime, "%d %s%d:%s%d", (int)days, zh, (int)hours, zm, (int)minutes);

  return String(uptime);
}



/*
   =============================================================================
   Async Web Server
   =============================================================================
*/

void handleRoot(AsyncWebServerRequest *request) {
  if (captivePortal(request)) {
    return;
  }

  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, sizeof(index_html));
  response->addHeader("Content-Encoding", "gzip");
  response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
  request->send(response);
}

boolean captivePortal(AsyncWebServerRequest *request) {
  if (isIp(request->host())) return false;

  AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
  response->addHeader("Location", "http://" + toStringIp(request->client()->localIP()) + "/setup.html");
  request->send(response);
  return true;
}

void handleNotFound(AsyncWebServerRequest *request) {
  if (captivePortal(request)) {
    return;
  }

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName (i) + ": " + request->arg (i) + "\n";
  }

  AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", message);
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  request->send(response);
}


/*
   =============================================================================
   Web
   =============================================================================
*/

void reportToClient(AsyncEventSourceClient * client) {
  if (client->lastId()) {
    Log::printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
  }

  String ev = "\nMAC: " + WiFi.macAddress();
  ev += "\nip: " + WiFi.localIP().toString();
  ev += "\nheap: " + String(ESP.getFreeHeap());
  ev += "\nFlashSize: " + String(ESP.getFlashChipSize());
  ev += "\nConfig: " + configSetup;
  ev += "\nWiFi: " + configWifi;
  ev += "\nSched: " + ScheduleSetup;
  ev += "\nVersion: " + ver;
  ev += "\nUpTime: " + uptimeString();

  client->send(ev.c_str());
}

void addServerResource(
  const char *uri,
  const char *mime,
  const uint8_t *data,
  const uint32_t data_size,
  const bool use_cache = true
) {
  server.on(uri, HTTP_GET, [ = ](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, mime, data, data_size);
    response->addHeader("Content-Encoding", "gzip");
    if (use_cache)
      response->addHeader("Cache-Control", "max-age=86400");
    request->send(response);
  }
           );
}

void WEB_init(void) {

  // Регистрация обработчика веб сокета
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Корневая страница
  addServerResource("/", "text/html", (const uint8_t *)&index_html, sizeof(index_html), false);
  
  server.on("/new.html",   HTTP_GET, [](AsyncWebServerRequest * request) { request->redirect("/"); });

  // Старый веб
  addServerResource("/old.html", "text/html", (const uint8_t *)&index_html2, sizeof(index_html2), false);
  
  server.on("/index2.html", HTTP_GET, [](AsyncWebServerRequest * request) { request->redirect("/old.html"); });

  // Простейшая страница ручного обновления
  server.on("/manual_update", HTTP_GET, [](AsyncWebServerRequest * request) {
    PORT_LOW(16);  //wdt off
    request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });

  // Обработка обновления
  server.on(
    "/update",
    HTTP_POST,
  [](AsyncWebServerRequest * request) {
    lazy_actions.restart_esp = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", lazy_actions.restart_esp ? "OK" : "FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },
  [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      Log::print(request->contentLength());
      Log::printf("Update Start: %s\n", filename.c_str());
      update_progress = jsonWrite(update_progress, "stage", "1");
      update_progress = jsonWriteArray(update_progress, "stages", "[\"backend\"]");

      Update.runAsync(true);

      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000), U_FLASH, 1, HIGH) {
        //Update.printError(Serial);
      }
    }
    if (!Update.hasError()) {
      if (Update.write(data, len) != len) {
        PORT_HIGH(16);   //wdt on
      }
      else {
        if (prog != 100 * index / (request->contentLength() - 212)) {
          prog = 100 * index / (request->contentLength() - 212);
          Log::print(prog);
          update_progress = jsonWrite(update_progress, "progress", prog);
          sendConfigWs(update_progress, "update_progress", 0);
        }
      }
    }
    if (final) {
      if (Update.end(true)) {
        update_progress = jsonWrite(update_progress, "progress", 100);
        sendConfigWs(update_progress, "update_progress", 0);
        Log::printf("Update Success: %uB\n", index + len);
        PORT_HIGH(16);   //wdt on
        lazy_actions.restart_esp = true;
      } else {
        PORT_HIGH(16);   //wdt on
      }
    }
  }
  );

  // Перезагрузка устройства
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    lazy_actions.restart_esp = true;
    request->send(200, "text/plain", "ok");
  });

  server.on("/reset",   HTTP_GET, [](AsyncWebServerRequest * request) { request->redirect("/reboot"); });
  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) { request->redirect("/reboot"); });

  // Страница setup.html
  addServerResource("/setup.html", "text/html", (const uint8_t *)&setup_html, sizeof(setup_html), false);
  server.on("/Setup.html",   HTTP_GET, [](AsyncWebServerRequest * request) { request->redirect("/setup.html"); });
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest * request) { request->redirect("/setup.html"); });

  // Изображения
  addServerResource("/Content/favicon.png", "image/x-icon", (const uint8_t *)&favicon, sizeof(favicon));
  addServerResource("/Content/Checked.svg", "image/svg+xml", (const uint8_t *)&checked_svg, sizeof(checked_svg));

  // Шрифты
  addServerResource("/Content/NotoSans.woff2", "application/font-sfnt", (const uint8_t *)&noto_sans_font, sizeof(noto_sans_font));

  // Стили
  addServerResource("/Content/Site.css", "text/css", (const uint8_t *)&styles_css, sizeof(styles_css));

  // Скрипты
  addServerResource("/Content/qrcode.min.js", "application/javascript", (const uint8_t *)&qrcode_js, sizeof(qrcode_js));
  addServerResource("/Content/scriptMenuEsp.js", "application/javascript", (const uint8_t *)&script_menu_esp_js, sizeof(script_menu_esp_js));

  // Доступ к LFS
  server.serveStatic("/", LittleFS, "/");

  // События
  events.onConnect(&reportToClient);

  // Регистрация обработчиков событий от сервера
  server.addHandler(&events);
  server.onNotFound(handleNotFound);

  // Запуск сервера
  server.begin();
}

void sendDataWs(bool change, bool sendToServer) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  JsonObject& dat = json.createNestedObject("update");

  if (ext_sens.is) {
    last_temp = ext_sens.temp;
    dat["temp1"] = String(averageTemp(), 1);
  }
  else if (int_sens.is == 1) {
    last_temp = int_sens.temp + sensor_correction;
    dat["temp1"] = String(averageTemp(), 1);
  }
  else {
    last_temp = averageTemp();
  }

  changed = true;
  if (have_mqtt_external_topic) {
    if (ext_sens.is) dat["ext_sensor"] = "online";
    else             dat["ext_sensor"] = "offline";
  }
  else               dat["ext_sensor"] = "disconnect";

  dat["temp"] = String(last_temp, 1);
  if (homekit != 0 && pair_hk == 1  && !isnan(last_temp)) {
    if (last_temp == -127) current_temperature.value.float_value = -1;
    else current_temperature.value.float_value = last_temp;
    homekit_characteristic_notify(&current_temperature, current_temperature.value);
  }

  dat["time"] = timeZoneStr;
  dat["relay"] = String(spaRelayOnOff());

  if (homekit != 0 && pair_hk == 1) {
    target_state.value.int_value = spaWork;
    homekit_characteristic_notify(&target_state, target_state.value);
  }

  if (spaWork) dat["heating"] = "heat";
  else dat["heating"] = "off";

  dat["unit"] = "Celsius";
  dat["name"] = devname;

  if (homekit != 0 && pair_hk == 1) {
    current_state.value.int_value = spaRelayOnOff();
    homekit_characteristic_notify(&current_state, current_state.value);
  }

  target_temp = thermo;
  dat["target_temp"] = String(target_temp, 1);
  if (homekit != 0 && pair_hk == 1  && !isnan(target_temp)) {
    target_temperature.value.float_value = target_temp;
    homekit_characteristic_notify(&target_temperature, target_temperature.value);
  }

  dat["newfw"] = esp8266_has_update() ? "1" : "0";

  String oldDataJson = dataJson;
  bool sendws = false;
  dataJson = "";
  dat.printTo(dataJson);
  if (dataJson != oldDataJson) {
    sendws = true;
  }

  String tempjson;
  dat.printTo(tempjson);

  if (mqtt_connect || alice == 1) {
    if (sendws || sendToServer || change)
      start_MQTT(tempjson);
  }
  else {
    if (have_mqtt)
      mqtt_needToConnect = true;
  }

  if (sendws || change) {
    if (numCon > 0) {
      size_t len = json.measureLength();
      AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
      if (buffer) {
        json.printTo((char *)buffer->get(), len + 1);
        ws.textAll(buffer);
      }
    }
  }

  changed = false;
}

void sendSSDPWs(bool sendToServer) {
  changed = true;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& ss = jsonBuffer.parseObject(addressList);
  if (numCon > 0) {
    size_t len = ss.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
      ss.printTo((char *)buffer->get(), len + 1);
      ws.textAll(buffer);
    }
  }
  changed = false;
}

void sendWiFiWs()
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(wifijson);
  wifijson = "";
  json.printTo(wifijson);
  if (numCon > 0) {
    size_t len = json.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
      json.printTo((char *)buffer->get(), len + 1);
      ws.textAll(buffer);
      //json.printTo(Serial);
    }
  }
}

void sendConfigWs(String conf, String file, bool sendToServer)
{
  changed = true;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  JsonObject& ss = jsonBuffer.parseObject(conf);
  if (ss.size() == 0) json.set(file, "{}");
  else json.set(file, ss);
  if (numCon > 0) {
    size_t len = json.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
      json.printTo((char *)buffer->get(), len + 1);
      //json.printTo(Serial);
      ws.textAll(buffer);
    }
  }
  changed = false;
}


void sendWs(String n, String x)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json.set(n, x);
  if (numCon > 0) {
    size_t len = json.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
      json.printTo((char *)buffer->get(), len + 1);
      ws.textAll(buffer);
    }
  }
}

void sendWs(String n, int x)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json.set(n, x);
  if (numCon > 0) {
    size_t len = json.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
      json.printTo((char *)buffer->get(), len + 1);
      ws.textAll(buffer);
    }
  }
}
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    char* data;
    numCon++;
    if (numCon > 4) {
      ws.enable(false);
    }
    sendSSDPWs(0);
    sendConfigWs(configSetup, "config", 0);
    sendConfigWs(int_ds_addr, "intDSaddr", 0);
    if (!homekit) sendConfigWs(mqtttopics, "mqtt_topics", 0);
    sendWiFiWs();
    sendDataWs(true, false);
    ws.textAll("{\"name\": \"" + devname + "\"}");
    sendConfigWs("{\"wifi_signal\":\"" + String(constrain((WiFi.RSSI() + 100) * 2, 0, 100)) + "\", \"socket_status\":\"1\", \"homekit\":" + String(homekit) + "}", "signals", 0);
    if (pair_hk != 1) sendWs("qr_hk", "X-HM://00909FWEF" + String(SETUP_ID));
    sendConfigWs(mqtts, "mqtt", 0);
    sendConfigWs(mqtt_data, "mqtt_data", 0);
  } else if (type == WS_EVT_DISCONNECT) {
    Log::printf("ws[%u] disconnect: %u\n", client->id());
    numCon--;
    if (numCon <= 4) ws.enable(true);
  } else if (type == WS_EVT_ERROR) {
    Log::printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if (type == WS_EVT_PONG) {
    Log::printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len) {
      msg = "";
      Log::printf("ws[%u] %s: \n", client->id(), (info->opcode == WS_TEXT) ? "text" : "binary");
      for (size_t i = 0; i < info->len; i++) {
        msg += (char) data[i];
      }
    }
    if (msg == "\"tempUp\"") {
      ChangeSchedule(1, 0.5);
    }
    if (msg == "\"tempDown\"") {
      ChangeSchedule(255, 0.5);
    }
    if (msg == "schedule_on") {
      spaWork = true;
      changeSpa(spaWork);
    }
    if (msg == "schedule_off") {
      spaWork = false;
      changeSpa(spaWork);
    }
    if (msg == "\"reset\"") {
      LittleFS.format();
      configSetup = "{}";
      saveFile("config.json", configSetup);
      signals = "{}";
      saveFile("signals.json", signals);
      ScheduleSetup = "{}";
      saveFile("schedule.json", ScheduleSetup);
      spasetup = jsonWrite(spasetup, "spawork", 0);
      saveFile("spa.json", spasetup);
      if (homekit) {
        clear_homekit();
      }
      wifi_reset();
      lazy_actions.restart_esp = true;
    }
    if (msg == "\"mqttDisconnect\"") {
      configSetup = jsonWrite(configSetup, "mqtt_ip", "");
      configSetup = jsonWrite(configSetup, "mqtt_port", "");
      configSetup = jsonWrite(configSetup, "mqtt_login", "");
      configSetup = jsonWrite(configSetup, "mqtt_pass", "");
      configSetup = jsonWrite(configSetup, "mqtt_use", "0");
      saveFile("config.json", configSetup);
      if (mqtt_connect)mqttClient.disconnect();
      sendConfigWs(configSetup, "config", webSocketConnect);
    }

    DynamicJsonBuffer jsonBuffer;
    JsonObject& data = jsonBuffer.parseObject(msg.c_str());
    if (data.containsKey("data")) {
      String tt, dd;
      long unix;
      int y, d, m, hh, mm;
      String dat = data.get<String>("data.json");
      JsonObject& object = jsonBuffer.parseObject(dat);
    }
    else if (data.containsKey("config")) {
      String dat = data.get<String>("config");
      JsonObject& object = jsonBuffer.parseObject(dat);
      object.printTo(Serial);
      if (object.containsKey("homekit")) {
        String dat = object.get<String>("homekit");
        //nextion.init("loading_page", 0);
        //Serial.println("1");Serial.println(configSetup);
        if (dat == "1") {
          homekit = 1;
          clear_homekit();
        }
        else if (dat == "2") {
          homekit = 2;
        }
        else {
          homekit = 0;
        }
        //Serial.println("2");Serial.println(configSetup);
        signals = jsonWrite(signals, "homekit", String(homekit));
        configSetup = jsonWrite(configSetup, "homekit", String(homekit));
        saveFile("signals.json", signals);
        configSetup = jsonWrite(configSetup, "pair_hk", "0");
        lazy_actions.restart_esp = true;
      }
      //Serial.println(configSetup);
      if (object.containsKey("login")) {
        configSetup = jsonWrite(configSetup, "login", object.get<String>("login"));
        configSetup = jsonWrite(configSetup, "password", object.get<String>("password"));
        //configSetup = jsonWrite(configSetup, "send_status", object.get<String>("send_status"));
      }
      if (object.containsKey("mqtt_ip")) {
        configSetup = jsonWrite(configSetup, "mqtt_ip", object.get<String>("mqtt_ip"));
        configSetup = jsonWrite(configSetup, "mqtt_port", object.get<String>("mqtt_port"));
        configSetup = jsonWrite(configSetup, "mqtt_login", object.get<String>("mqtt_login"));
        configSetup = jsonWrite(configSetup, "mqtt_pass", object.get<String>("mqtt_pass"));
        configSetup = jsonWrite(configSetup, "mqtt_use", object.get<String>("mqtt_use"));
        saveFile("config.json", configSetup);
        if (wifiSTA) MQTT_init();
      }
      if (object.containsKey("name")) {
        configSetup = jsonWrite(configSetup, "name", object.get<String>("name"));
      }
      if (object.containsKey("hysteresis")) {
        configSetup = jsonWrite(configSetup, "hysteresis", object.get<String>("hysteresis"));
        spaInit();
        tempInit();
      }
      if (object.containsKey("sensor_corr")) {
        configSetup = jsonWrite(configSetup, "sensor_corr", object.get<String>("sensor_corr"));
        spaInit();
        tempInit();
      }
      if (object.containsKey("sensor_model_id")) {
        configSetup = jsonWrite(configSetup, "sensor_model_id", object.get<String>("sensor_model_id"));
        spaInit();
        tempInit();
      }
      if (object.containsKey("sensor_internal_use")) {
        configSetup = jsonWrite(configSetup, "sensor_internal_use", object.get<String>("sensor_internal_use"));
        spaInit();
        tempInit();
      }
      if (object.containsKey("max_temp")) {
        if (pair_hk == 0) configSetup = jsonWrite(configSetup, "max_temp", object.get<String>("max_temp"));
      }
      if (object.containsKey("min_temp")) {
        if (pair_hk == 0) configSetup = jsonWrite(configSetup, "min_temp", object.get<String>("min_temp"));
      }
      if (object.containsKey("wifi_name")) {
        configWifi = jsonWrite(configWifi, "ssid", object.get<String>("wifi_name"));
        configWifi = jsonWrite(configWifi, "ssidPass", object.get<String>("wifi_pass"));
        configWifi = jsonWrite(configWifi, "ssid", object.get<String>("wifi_name"));
        configSetup = jsonWrite(configSetup, "wifi_name", object.get<String>("wifi_name"));
        configWifi = jsonWrite(configWifi, "ssidPass", object.get<String>("wifi_pass"));
        saveFile("wifi.save.json", configWifi);
      }

      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      sendDataWs(true, webSocketConnect);
    }
    else if (data.containsKey("wifi_connect")) {
      String dat = data.get<String>("wifi_connect");
      JsonObject& object = jsonBuffer.parseObject(dat);
      if (object.containsKey("ssid")) {
        configWifi = jsonWrite(configWifi, "ssid", object.get<String>("ssid"));
        configWifi = jsonWrite(configWifi, "ssidPass", object.get<String>("password"));
        configWifi = jsonWrite(configWifi, "ssid", object.get<String>("ssid"));
        configSetup = jsonWrite(configSetup, "wifi_name", object.get<String>("ssid"));
        configWifi = jsonWrite(configWifi, "ssidPass", object.get<String>("password"));
        saveFile("config.json", configSetup);
      }
      saveFile("wifi.save.json", configWifi);
      lazy_actions.restart_esp = true;
    }
    else if (data.containsKey("wifi_refresh")) {
      String dat = data.get<String>("wifi_refresh");
      if (dat=="1" && !wifiSTA){//wifi_scanned>0) { 
          wifi_scanned = 0;
          scanWiFi();
          //sendWiFiWs();
          WiFi.scanNetworks(true);
      }
    }
    else if (data.containsKey("mqtt_connect")) {
      String dat = data.get<String>("mqtt_connect");
      JsonObject& object = jsonBuffer.parseObject(dat);
      configSetup = jsonWrite(configSetup, "mqtt_server", object.get<String>("mqtt_server"));
      configSetup = jsonWrite(configSetup, "mqtt_port", object.get<String>("mqtt_port"));
      configSetup = jsonWrite(configSetup, "mqtt_login", object.get<String>("mqtt_login"));
      configWifi = jsonWrite(configWifi, "mqtt_password", object.get<String>("mqtt_password"));
      saveFile("config.json", configSetup);
      saveFile("wifi.save.json", configWifi);
      if (mqtt_connect)mqttClient.disconnect();
      if (wifiSTA) MQTT_init();
    }
    else if (data.containsKey("mqtt_disconnect")) {
      String dat = data.get<String>("mqtt_disconnect");
      JsonObject& object = jsonBuffer.parseObject(dat);
      configSetup = jsonWrite(configSetup, "mqtt_server", "");
      configSetup = jsonWrite(configSetup, "mqtt_port", "");
      configSetup = jsonWrite(configSetup, "mqtt_login", "");
      configWifi = jsonWrite(configWifi, "mqtt_password", "");
      configSetup = jsonWrite(configSetup, "mqtt_use", "0");
      saveFile("config.json", configSetup);
      saveFile("wifi.save.json", configWifi);
      have_mqtt = false;
      if (mqtt_connect)mqttClient.disconnect();
    }
    else if (data.containsKey("alice_connect")) {
      String dat = data.get<String>("alice_connect");
      JsonObject& object = jsonBuffer.parseObject(dat);
      configSetup = jsonWrite(configSetup, "alice_login", object.get<String>("alice_login"));
      configWifi = jsonWrite(configWifi, "alice_pass", object.get<String>("alice_password"));
      configSetup = jsonWrite(configSetup, "mqtt_alice", "0");
      saveFile("config.json", configSetup);
      saveFile("wifi.save.json", configWifi);
      needAlice = true;
      alice = 255;
      sendConfigWs(configSetup, "config", webSocketConnect);
    }
    else if (data.containsKey("alice_disconnect")) {
      configSetup = jsonWrite(configSetup, "alice_login", "");
      mqttClient.disconnect();
      needAlice = true;
      alice = 0;
    }
    else if (data.containsKey("network_config")) {
      String dat = data.get<String>("network_config");
      JsonObject& object = jsonBuffer.parseObject(dat);
      configSetup = jsonWrite(configSetup, "name", object.get<String>("name"));
      saveFile("config.json", configSetup);
      lazy_actions.restart_esp = true;
      devname = jsonRead(configSetup, "name");
      ws.textAll("{\"name\": \"" + devname + "\"}");
    }
    else if (data.containsKey("mqtt_external_topic")) {
      configSetup = jsonWrite(configSetup, "mqtt_external_topic", data.get<String>("mqtt_external_topic"));
      String tempStr = data.get<String>("mqtt_external_topic");
      mqttClient.subscribe(tempStr.c_str(), 0);
      have_mqtt_external_topic = true;
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      sendDataWs(true, webSocketConnect);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      JsonObject& datas = root.createNestedObject("0");
      datas["ShotAddr"] = "1";
      mqtts = "";
      root.printTo(mqtts);
      sendConfigWs(mqtts, "mqtt", 0);
    }
    else if (data.containsKey("mqtt_external")) {
      String temp = jsonRead(configSetup, "mqtt_external_topic");
      mqttClient.unsubscribe(temp.c_str());
      configSetup = jsonWrite(configSetup, "mqtt_external_topic", "");
      ext_sens.is = 0;
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      sendDataWs(true, webSocketConnect);
    }
    else if (data.containsKey("hysteresis")) {
      configSetup = jsonWrite(configSetup, "hysteresis", data.get<String>("hysteresis"));
      spaInit();
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      sendDataWs(true, webSocketConnect);
    }
    else if (data.containsKey("sensor_corr")) {
      configSetup = jsonWrite(configSetup, "sensor_corr", data.get<String>("sensor_corr"));
      spaInit();
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      sendDataWs(true, webSocketConnect);
    }
    else if (data.containsKey("sensor_model_id")) {
      sendWs("config_nav", 1);
      configSetup = jsonWrite(configSetup, "sensor_model_id", data.get<String>("sensor_model_id"));
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      sendDataWs(true, webSocketConnect);
      lazy_actions.restart_esp = true;
    }
    else if (data.containsKey("heating")) {
      String dat = data.get<String>("heating");
      if (dat == "1") {
        spaWork = true;
      }
      else {
        spaWork = false;
      }
      changeSpa(spaWork);
      sendDataWs(true, webSocketConnect);
    }
    else if (data.containsKey("is_target_temp_first")) {
      String dat = data.get<String>("is_target_temp_first");
      if (dat == "1") {
        configSetup = jsonWrite(configSetup, "is_target_temp_first", "0");
        is_target_temp_first = 0;
      }
      else {
        configSetup = jsonWrite(configSetup, "is_target_temp_first", "1");
        is_target_temp_first = 1;
      }
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      sendDataWs(true, webSocketConnect);
    }
    else if (data.containsKey("max_temp")) {
      if (pair_hk == 0) {
        configSetup = jsonWrite(configSetup, "max_temp", data.get<String>("max_temp"));
        saveFile("config.json", configSetup);
        presetMax = jsonReadtoInt(configSetup, "max_temp");
        sendConfigWs(configSetup, "config", webSocketConnect);
        sendDataWs(true, webSocketConnect);
      }
    }
    else if (data.containsKey("min_temp")) {
      if (pair_hk == 0) {
        configSetup = jsonWrite(configSetup, "min_temp", data.get<String>("min_temp"));
        saveFile("config.json", configSetup);
        presetMin = jsonReadtoInt(configSetup, "min_temp");
        sendConfigWs(configSetup, "config", webSocketConnect);
        sendDataWs(true, webSocketConnect);
      }
    }
    else if (data.containsKey("clear_homekit")) {
      dwin_hk_loading_screen();
      configSetup = jsonWrite(configSetup, "pair_hk", "0");
      saveFile("config.json", configSetup);
      clear_homekit();
      lazy_actions.restart_esp = true;
      pair_hk = 0;
    }
    else if (data.containsKey("homekit")) {
      String dat = data.get<String>("homekit");
      if (dat == "1") {
        homekit = 1;
        clear_homekit();
      }
      else {
        homekit = 0;
      }
      signals = jsonWrite(signals, "homekit", String(homekit));
      saveFile("signals.json", signals);
      configSetup = jsonWrite(configSetup, "pair_hk", "0");
      saveFile("config.json", configSetup);
      sendConfigWs(configSetup, "config", webSocketConnect);
      lazy_actions.restart_esp = true;
    }
    else if (data.containsKey("mk")) {
      sendDataWs(true, false);
    }
    else if (data.containsKey("files")) {
      if (data["files"]["ino_bin"].as<String>().length() > 0) {
        lazy_actions.update_esp = true;
      }

      if (data["files"]["dwin_link"].as<String>().length() > 0) {
        lazy_actions.update_dwin = true;
      }
    }
  }
}

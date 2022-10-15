#include "log.h"
#include "dwin.h"

void set_clock() {
  now = timeZoneStr.toInt();
  tv = { now, 0 };
  settimeofday(&tv, nullptr);
  now = time(nullptr);
  tm = localtime(&now);
  fileName = String(tm->tm_wday) + ".csv";
}

bool saveFile(String file, String conf) {
  int addr;
  int l = conf.length();
  char string[800];

  conf.toCharArray(string, l + 1);
  if (file == "config.json") addr = 2000;
  else if (file == "intDS.json") addr = 3000;
  else if (file == "schedule.json") addr = 3100;
  else if (file == "signals.json") addr = 3300;
  else if (file == "spa.json") addr = 3400;
  else if (file == "wifi.save.json") addr = 3500;

  EEPROM.begin(4000);
  EEPROM.write(addr, byte(l));
  EEPROM.write(addr + 1, l >> 8);
  for (int i = 0; i <= l + 1; i++)
  {
    EEPROM.write(addr + 2 + i, string[i]);
  }
  EEPROM.commit();
  EEPROM.end();

  File configFile = LittleFS.open("/" + file, "w");
  if (!configFile) {
    return false;
  }
  configFile.write(string, l);
  configFile.close();

  return true;
}

String readFile(String file, size_t len ) {
  String temp;
  int l;
  char t[800];
  int addr;
  if (file == "config.json") addr = 2000;
  else if (file == "intDS.json") addr = 3000;
  else if (file == "schedule.json") addr = 3100;
  else if (file == "signals.json") addr = 3300;
  else if (file == "spa.json") addr = 3400;
  else if (file == "wifi.save.json") addr = 3500;

  EEPROM.begin(4000);
  l = EEPROM.read(addr + 1);
  l = l << 8;
  l = l + EEPROM.read(addr);
  if (l < 800) {
    for (int i = 0; i <= l + 1; i++)
    {
      t[i] = EEPROM.read(addr + 2 + i);
    }
  }
  else {
    EEPROM.end();
    File configFile = LittleFS.open("/" + file, "r");
    if (!configFile) {
      return "{}";
    }
    temp = configFile.readString();
    configFile.close();
    return temp;
  }
  EEPROM.end();
  temp = String(t);
  return temp;
}

void change_file(int dd) {
  if (LittleFS.exists(String(dd) + ".csv")) {
    LittleFS.remove(String(dd) + ".csv");
    fileName = String(dd) + "_1.csv";
  }
  else {
    LittleFS.remove(String(dd) + "_1.csv");
    fileName = String(dd) + ".csv";
  }
}

// ------------- Запись строки в файл
String writeFile(String strings) {
  char buffer [20];
  String dd;
  now = time(nullptr);
  tm = localtime(&now);

  File configFile = LittleFS.open("/" + fileName, "a");
  if (!configFile) {
    return "Failed to open config file";
  }
  strftime (buffer, 20, "%T %m/%d", tm);
  dd = String(buffer);
  if (configFile.size() < 60000) configFile.print(strings + dd);
  configFile.close();
  return "Write sucsses";
}

// ------------- Чтение значения json
String jsonRead(String json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name].as<String>();
}

// ------------- Чтение значения json
int jsonReadtoInt(String json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name];
}

int jsonReadtoBool(String json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name].as<bool>();
}

// ------------- Запись значения json String
//{Schedule: [["00:00", "00:00", "00:00", "00:00"], ["00:00", "00:00", "00:00", "00:00"]}
//{"Temperature":[["0","0","0","0"],["0","0","0","0"]]}
String jsonWriteArray(String json, String name, String volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  JsonArray& vol = jsonBuffer.parseArray(volume);
  root[name] = vol;
  json = "";
  root.printTo(json);
  return json;
}

// ------------- Запись значения json String
String jsonWrite(String json, String name, String volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}

// ------------- Запись значения json int
String jsonWrite(String json, String name, int volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}

uint32_t ChipId() {
  return ESP.getChipId();
}

void clear_homekit() {
  EEPROM.begin(1500);
  EEPROM.write(0, 0x48);
  EEPROM.write(1, 0x41);
  EEPROM.write(2, 0x50);
  EEPROM.write(3, 0x0);
  for (int i = 4; i < 1408; i++)
  {
    EEPROM.write(i, 0xff);
  }
  EEPROM.end();
  homekit_storage_reset();
}

#include "fetch_update_info.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


////////////////////////////////////////////////////////////////////////////////
// Константы

#define CRYPTO_KEY "Lytko-101"


////////////////////////////////////////////////////////////////////////////////
// Декларации для сборки

String jsonRead(String, String);
void sendWs(String, String);

extern String configSetup;
extern String ver;
extern uint32_t dwin_did;


////////////////////////////////////////////////////////////////////////////////
// Вспомогательные методы

static String update_check_json(
  uint32_t device_id,
  const char *device_type,
  const char *model,
  const char *version,
  const char *username = nullptr
) {
  DynamicJsonBuffer jsonBuffer(192);
  JsonObject& json = jsonBuffer.createObject();

  json["chip_id"]    = String(device_id);
  json["dev_type"]   = device_type;
  json["model"]      = model;
  json["fw_version"] = version;
  json["username"]   = username == nullptr ? "" : username;

  String result;
  json.printTo(result);
  return result;
}

static void rc4(
  char *key,
  char *data,
  unsigned char *S,
  char *has
) {
  int i, j, t;
  for (i = 0; i < 256; i++) {
    S[i] = i;
  }
  j = 0;
  for (i = 0; i < 256; i++) {
    j = (j + S[i] + key[i % strlen(key)]) % 256;
    t = S[i]; S[i] = S[j]; S[j] = t;
  }
  i = j = 0;
  for (int k = 0; k < strlen(data); k++) {
    i = (i + 1) % 256;
    j = (j + S[i]) % 256;
    t = S[i]; S[i] = S[j]; S[j] = t;
    has[k] = data[k] ^ S[(S[i] + S[j]) % 256];
  }
  has[strlen(data) + 1] = '\0';
}

static String crypt_and_hex(String &str) {

  // Шифрование
  unsigned char S[256];
  char has[512];

  char str_bytes[str.length() + 1];
  str.toCharArray(str_bytes, sizeof(str_bytes));

  rc4(CRYPTO_KEY, str_bytes, S, has);

  // Преобразование в hex строку
  String crypted_str;

  for (int i = 0; i < strlen(str_bytes); i++) {
    byte nib1 = (has[i] >> 4) & 0x0F;  // Старший полубайт
    byte nib2 = (has[i] >> 0) & 0x0F;  // Младший полубайт

    if (nib1 < 0xA) {
      crypted_str += (char)('0' + nib1);
    } else {
      crypted_str += (char)('A' + nib1 - 0xA);
    }

    if (nib2 < 0xA) {
      crypted_str += (char)('0' + nib2);
    } else {
      crypted_str += (char)('A' + nib2  - 0xA);
    }
  }

  return crypted_str;
}

/*static void setup_update_check_client(HTTPClient &httpClient) {
  WiFiClient client;
  httpClient.useHTTP10(false);
  httpClient.setTimeout(5000);
  httpClient.begin(client, "http://admin.lytko.com:8000/api/state");
  httpClient.addHeader("Authorization", "mwokMcw95MQ3jY8jlVxpZ");
  httpClient.addHeader("Content-Type", "application/json");
}*/

static UpdateInfo fetch_update_info(
  uint32_t device_id,
  const char *device_type,
  const char *model,
  const char *version,
  const char *username = nullptr
) {
  UpdateInfo result;
  result.has_update = false;
  result.has_error  = false;
  result.new_version     = "";
  result.new_version_url = "";

  // Проверка подключения к сети
  if (WiFi.status() != WL_CONNECTED) {
    result.has_error = true;
    return result;
  }

  // Формирование запроса
  String json_str = update_check_json(
                      device_id,
                      device_type,
                      model,
                      version,
                      username
                    );

  String request_str = crypt_and_hex(json_str);

  // Отправка запроса
  WiFiClient client;
  HTTPClient httpClient;
  httpClient.useHTTP10(false);
  httpClient.setTimeout(5000);
  httpClient.begin(client, "http://admin.lytko.com:8000/api/state");
  httpClient.addHeader("Authorization", "mwokMcw95MQ3jY8jlVxpZ");
  httpClient.addHeader("Content-Type", "application/json");
  //setup_update_check_client(http_client);

  int code = httpClient.POST(request_str);
  if (code == HTTP_CODE_OK) {
    String payload = httpClient.getString();

    //sendWs("log", "fetch_update_info | Send: " + json_str);
    //sendWs("log", "fetch_update_info | Receive: " + payload);

    // Парсинг ответа
    DynamicJsonBuffer received_json_buffer;
    JsonObject& received_json = received_json_buffer.parseObject(payload);

    // Время ответа
    if (received_json.containsKey("ts")) {
      result.ts = received_json.get<String>("ts");
    }

    // Проверка наличия обновления
    if (received_json.containsKey("fw_url")) {
      String url = received_json.get<String>("fw_url");

      result.new_version_url = url;
      result.new_version = url.substring(
                             url.lastIndexOf("_") + 1,
                             url.lastIndexOf(".")
                           );

      result.has_error = false;
      result.has_update = true;
    } else {
      result.has_error = false;
      result.has_update = false;
    }
  } else {
    // Что-то пошло не так
    result.has_error = true;
    result.has_update = false;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Публичные методы

UpdateInfo fetch_thermostat_update_info() {
  String username;
  String alice_login = jsonRead(configSetup, "alice_login");
  String mqtt_login  = jsonRead(configSetup, "mqtt_login");
  if (alice_login.length() > 3) {
    username = alice_login;
  } else if (mqtt_login.length() > 3) {
    username = mqtt_login;
  } else {
    username = "";
  }

  // Формирование запроса
  return fetch_update_info(
           ESP.getChipId(),
           "Thermostat_101_plus",
           "Lytko-101-plus",
           ver.c_str(),
           username.c_str()
         );
}

UpdateInfo fetch_dwin_update_info() {
  String actual_dwin_version = jsonRead(configSetup, "dwin_version");

  return fetch_update_info(
           dwin_did,
           "Dwin_4_inch",
           "DMG4",
           actual_dwin_version.c_str()
         );
}

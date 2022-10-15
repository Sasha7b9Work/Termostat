////////////////////////////////////////////////////////////////////////////////
// Структуры

struct {
  bool has;
  String version;
  String url;
} esp_update_data;


////////////////////////////////////////////////////////////////////////////////
// Код

bool esp8266_has_update() {
  return esp_update_data.has;
}

String esp8266_next_version() {
  return esp_update_data.version;
}

void esp8266_update_info(UpdateInfo update_info) {
  esp_update_data.has     = (update_info.new_version.length() > 0) && (update_info.new_version_url.length() > 0);
  esp_update_data.version = (update_info.new_version);
  esp_update_data.url     = (update_info.new_version_url);

  configSetup = jsonWrite(configSetup, "link", esp_update_data.url);
  configSetup = jsonWrite(configSetup, "version_new", esp_update_data.version);
  saveFile("config.json", configSetup);
}

void esp8266_update() {
  dwin_command(DwinCommand::UpdateScreen_Update);

  Wdt::disable();

  if (esp8266_update_internal()) {
    PORT_HIGH(0);
    delay(1000);
    PORT_HIGH(0);

    lazy_actions.restart_esp = true;
  } else {
    Wdt::enable();
    // TODO Отобразить и логировать ошибку
  }
}

static bool esp8266_update_internal() {
#define REPORT_ERROR(s) Log::println(s); sendWs("error", s);

  uint32_t updateSize = 0;
  uint8_t  progress   = 0;
  bool     result     = false;

  WiFiClient client;
  HTTPClient httpClient;
  httpClient.useHTTP10(false);
  httpClient.setTimeout(10000);
  httpClient.begin(client, esp_update_data.url);

  Log::println("Starting update...");

  // Проверка на подключение к серверу
  if (httpClient.GET() != HTTP_CODE_OK) {
    REPORT_ERROR("Unable to fetch, please reboot the device and try again");

    httpClient.end();
    return false;
  }

  // Проверка на корректную инициализацию обновления
  updateSize = httpClient.getSize();
  if (!Update.begin(updateSize, false)) {
    REPORT_ERROR("Incorrect startup conditions, please reboot the device and try again");

    httpClient.end();
    return false;
  }

  // Обновление
  uint8_t buffer[1024] = { 0 };
  WiFiClient *stream = httpClient.getStreamPtr();
  while (httpClient.connected() && (updateSize > 0 || updateSize == -1)) {
    uint32_t available = stream->available();

    if (available) {
      int c = stream->readBytes(buffer, ((available > sizeof(buffer)) ? sizeof(buffer) : available));
      Update.write(buffer, c);
      if (updateSize > 0) {
        updateSize -= c;
      }
    }

    uint8_t actual_progress = (uint8_t)(Update.progress() * 100 / httpClient.getSize());
    if (progress != actual_progress) {
      progress = int(Update.progress() * 100 / httpClient.getSize());

      sendWs("update_status", String(progress));

      dwin_set_update_percent(progress);
      dwin_set_update_stage(map(progress, 0, 100, 0, 8));
    }
  }

  result = Update.end();

  // Проверка на корректное завершение обновления
  if (!result) {
    REPORT_ERROR("Invalid completion conditions, please reboot the device and try again");
  } else {
    Log::println("Update complete!");
  }

  // Завершаем работу http клиента
  httpClient.end();

  return result;

#undef REPORT_ERROR
}

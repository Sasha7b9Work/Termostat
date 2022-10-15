#include "update_dwin.h"

#define FILE_PATH(FN) ("/" + FN)

UpdateResult dwin_update_file(uint8_t i, String version_url);
UpdateResult dwin_update_download_file(String file_name, WiFiClient *stream, uint32_t file_size);
void dwin_update_write_file(uint8_t i, String file_name, uint8_t file_index);


////////////////////////////////////////////////////////////////////////////////
// Данные

static const struct {
  uint8_t index;
  String  name;
} dwin_files[] = {
  { 12, "12_HZK.BIN"      },
  { 13, "13TouchFile.bin" },
  { 14, "14ShowFile.bin"  },
  { 22, "22_Config.bin"   },
  { 32, "32.icl" },
  { 48, "48.icl" },
  { 50, "50.icl" },
  { 51, "51.icl" },
  { 52, "52.icl" },
  { 53, "53.icl" },
  { 54, "54.icl" },
  { 55, "55.icl" },
  { 56, "56.icl" },
  { 57, "57.icl" },
  { 58, "58.icl" },
  { 59, "59.icl" },
  { 60, "60.icl" },
  { 61, "61.icl" },
  { 62, "62.icl" }
};

static struct {
  bool has;
  String version;
} dwin_update_data;


////////////////////////////////////////////////////////////////////////////////
// Код

bool dwin_has_update() {
  return dwin_update_data.has;
}

String dwin_next_version() {
  return dwin_update_data.version;
}

void dwin_update_info(UpdateInfo update_info) {
  dwin_update_data.has     = (update_info.new_version.length() > 0) && (update_info.new_version_url.length() > 0);
  dwin_update_data.version = (update_info.new_version);

  configSetup = jsonWrite(configSetup, "dwin_link", dwin_update_data.has ? "stub" : "");
  configSetup = jsonWrite(configSetup, "dwin_version_new", dwin_update_data.version);
  saveFile("config.json", configSetup);
}

UpdateResult dwin_update() {
  if (!dwin_update_data.has) {
    return UpdateResult::NoUpdate;
  }

  // Переходим к экрану обновления
  dwin_command(DwinCommand::UpdateScreen_DwinUpdate);
  dwin_wait_ok();

  // Показываем экран обновления во вебе
  sendWs("update_status", String(0));

  String base_url = "http://downloads.lytko.com/DWIN_SET";
  String version_url = base_url + '/' + dwin_update_data.version;

  uint8_t files_count = sizeof(dwin_files) / sizeof(dwin_files[0]);
  for (uint8_t i = 0; i < files_count; i++) {
    UpdateResult file_update_result = dwin_update_file(i, version_url);
    if (file_update_result != UpdateResult::NoError) {
      return file_update_result;
    }
  }

  // Уведомляем о завершении обновления
  sendWs("update_status", String(100));
  if (mqtt_update) {
    mqtt_send_msg(devnamemqtt + "/dwin_update_progress", String(100), true);
  }

  // Сбрасываем update info, сохраняем новую версию в конфиг
  configSetup = jsonWrite(configSetup, "dwin_version", dwin_update_data.version);
  saveFile("config.json", configSetup);

  UpdateInfo empty_update_info = { false, false, "", "", "" };
  dwin_update_info(empty_update_info);

  sendConfigWs(configSetup, "config", webSocketConnect);

  // Сбрасываем экран, запускаем отложенную инициализацию
  dwin_reset();

  lazy_actions.restart_dwin = true;
  dwin_restart_time = millis();

  return UpdateResult::NoError;
}

UpdateResult dwin_update_file(uint8_t i, String version_url) {
  auto file_index = dwin_files[i].index;
  auto file_name = dwin_files[i].name;
  auto file_url  = version_url + '/' + file_name;
  
  // Инициализация http клиента
  WiFiClient client;
  HTTPClient http_client;
  http_client.useHTTP10(false);
  http_client.setTimeout(2000);
  http_client.begin(client, file_url);

  sendWs("log", String("File url ") + file_url);

  // Проверка подключения к серверу
  int code = http_client.GET();
  if (code != HTTP_CODE_OK) {
    http_client.end();
    return UpdateResult::ServerError;
  }

  // Проверка размера файла
  int file_size = http_client.getSize();
  if (file_size <= 0) {
    http_client.end();
    return UpdateResult::ServerError;
  }

  // Загрузка файла во файловую систему
  UpdateResult download_file_result = dwin_update_download_file(
                                        file_name,
                                        http_client.getStreamPtr(),
                                        file_size
                                      );
  if (download_file_result != UpdateResult::NoError) {
    return download_file_result;
  }

  // Отправка файла на экран
  dwin_update_write_file(i, file_name, file_index);

  // Подчищаем за собой
  LittleFS.remove(FILE_PATH(file_name));

  // Завершаем
  http_client.end();

  return UpdateResult::NoError;
}

UpdateResult dwin_update_download_file(String file_name, WiFiClient *file_stream, uint32_t file_size) {
  String file_path = FILE_PATH(file_name);
  File file = LittleFS.open(file_path, "w+");

  sendWs("log", String("Download file ") + file_path);

  uint32_t timeout = 15 * 1000;
  uint32_t last_time = millis();
  uint32_t bytes_total = file_size;
  uint32_t bytes_left  = file_size;
  while (bytes_left != 0) {

    if (file_stream->available() > 0) {
      file.write(file_stream->read());
      bytes_left--;
      last_time = millis();
      continue;
    }

    if (millis() - last_time >= timeout) {
      sendWs("log", String("Timeout"));
      return UpdateResult::ServerTimeout;
    }
  }

  file.close();

  return UpdateResult::NoError;
}

void dwin_update_write_file(uint8_t i, String file_name, uint8_t file_index) {
  String file_path = FILE_PATH(file_name);
  File file = LittleFS.open(file_path, "r");

  uint32_t file_size = file.size();

  uint32_t segment_size = 32 * 1024;
  uint8_t  chunk_size = 240;

  uint32_t segments_count = ceil((float)file_size / segment_size);
  uint32_t final_segment_size = file_size - ((segments_count - 1) * (32 * 1024));

  for (int16_t s = (segments_count - 1); s >= 0; s--) {

    uint32_t actual_segment_size = (s == segments_count - 1) ? final_segment_size : segment_size;
    uint32_t actual_segment_offset = s * segment_size;

    uint32_t segment_bytes_left = actual_segment_size;

    uint16_t chunks_count = ceil((float) actual_segment_size / chunk_size);
    for (uint16_t c = 0; c < chunks_count; c++) {
      uint32_t actual_chunk_offset = actual_segment_size - segment_bytes_left;
      uint8_t  actual_chunk_size = segment_bytes_left >= chunk_size ? chunk_size : segment_bytes_left;

      uint8_t chunk[actual_chunk_size];
      file.seek(actual_segment_offset + actual_chunk_offset, SeekSet);
      file.read((uint8_t *) &chunk, actual_chunk_size);

      dwin_write_to_ram(actual_chunk_offset, (uint8_t *) &chunk, actual_chunk_size);

      segment_bytes_left -= actual_chunk_size;
    }

    sendWs("log", String("Write to ROM, segment ") + ((file_index * 8) + s));

    dwin_write_ram_to_flash((file_index * 8) + s);

    //sendWs("log", "Success");

    uint8_t files_count = sizeof(dwin_files) / sizeof(dwin_files[0]);
    float progress = (
                         (100.0 / files_count) * (i + 1.0) +
                         ((100.0 / files_count) / segments_count) * (segments_count - s)
                       );

    sendWs("update_status", String(progress));
    if (mqtt_update) {
      mqtt_send_msg(devnamemqtt + "/dwin_update_progress", String(progress), true);
    }
  }

  file.close();
}

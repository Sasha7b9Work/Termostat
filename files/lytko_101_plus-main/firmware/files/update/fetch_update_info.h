#pragma once

#include <Arduino.h>

struct UpdateInfo {
  bool has_update, has_error;
  String ts;
  String new_version;
  String new_version_url;
};

UpdateInfo fetch_thermostat_update_info();
UpdateInfo fetch_dwin_update_info();

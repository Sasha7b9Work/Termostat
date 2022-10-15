#pragma once

#include "fetch_update_info.h"

enum class UpdateResult {
  NoUpdate,
  ServerError,
  ServerTimeout,
  NoError
};

bool dwin_has_update();
String dwin_next_version();
void dwin_update_info(UpdateInfo update_info);
UpdateResult dwin_update();

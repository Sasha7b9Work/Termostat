#ifndef DWIN_H
#define DWIN_H

enum class DwinCommand {
  Idle,

  FirstScreen,

  LoadingScreen,

  MainScreen,
  MainScreen_Heat,
  MainScreen_TempUp,
  MainScreen_TempDown,

  SettingsScreen,
  SettingsScreen_Back,

  UpdateScreen,
  UpdateScreen_Update,
  UpdateScreen_DwinUpdate,

  HystScreen,
  HystScreen_HystUp,
  HystScreen_HystDown,
  HystScreen_AdjUp,
  HystScreen_AdjDown,

  TempScreen,
  TempScreen_Swap,

  WiFiScreen,
  WiFiScreen_Disconnect,

  HomeKitScreen,
  HomeKitScreen_BreakPair,

  SensorScreen,
  SensorScreen_Reselect,

  ChildProtectionScreen,
  ChildProtectionScreen_Enable,
  ChildProtectionScreen_Disable,

  ResetScreen,
  ResetScreen_ResetConfirmed,

  RebootScreen,
  RebootScreen_RebootConfirmed
};

void dwin_set_target_min_max(float min, float max);
void dwin_set_hysteresis(float hysteresis);
void dwin_set_temp_adj(float temp_adj);
void dwin_set_wifi_strength(uint8_t wifi_strength);
void dwin_set_temps(float last_temp, float target_temp);
void dwin_set_sensor_index(uint8_t sensor_index);
void dwin_set_active(bool is_active);
void dwin_set_heating(bool is_heating);
void dwin_set_is_target_temp_first(bool is_target_temp_first);
void dwin_set_update_percent(uint8_t percent);
void dwin_set_update_stage(uint8_t stage);
void dwin_hk_loading_screen();
void dwin_has_update(bool has_update);
void dwin_update_set_versions(String current_version, String new_version);
void dwin_hk_has_pair(bool has_pair);

void dwin_write_to_ram(uint32_t offset, uint8_t *bytes, uint32_t len);
void dwin_write_ram_to_flash(uint32_t segment);
void dwin_wait_ok();

void dwin_reset();
void dwin_command(DwinCommand command);
void dwin_init();
void dwin_lifecycle();

#endif

#include "log.h"
#include "dwin.h"

#include <HardwareSerial.h>

#define DEFAULT_CHILD_PROTECTION_COUNTDOWN 60

class FixedSerial : public HardwareSerial {
  public:
    FixedSerial(int uart_nr) : HardwareSerial(uart_nr) {}

    int fixedAvailable(void) {
      return static_cast<int>(uart_rx_available(_uart));
    }
};

FixedSerial ScreenSerial(UART0);


/* -----------------------------------------------------------------------------
   События и запросы
   -----------------------------------------------------------------------------
**/

uint8_t OkResponse[]  = { 0x5A, 0xA5, 0x03, 0x82, 0x4F, 0x4B };

/* События от экрана */
uint8_t NormalEvent[] = { 0x5A, 0xA5, 0x06, 0x83, 0x00, 0x00, 0x01 };

uint8_t OnHeat[]     = { 0x02, 0x30 };
uint8_t OnTempUp[]   = { 0x02, 0x32 };
uint8_t OnTempDown[] = { 0x02, 0x33 };
uint8_t OnSettings[] = { 0x02, 0x36 };

uint8_t OnUpdate[]   = { 0x07, 0x30 };

uint8_t OnHystUp[]   = { 0x11, 0x30 };
uint8_t OnHystDown[] = { 0x11, 0x31 };
uint8_t OnAdjUp[]   = { 0x11, 0x33 };
uint8_t OnAdjDown[] = { 0x11, 0x32 };

uint8_t OnTempSwap[] = { 0x13, 0x35 };

uint8_t OnWiFiDisconnect[] = { 0x14, 0x30 };

uint8_t OnHomeKitContinueBreakPair[] = { 0x17, 0x30 };

uint8_t OnSensorSelect01[] = { 0x19, 0x30 };
uint8_t OnSensorSelect02[] = { 0x19, 0x31 };
uint8_t OnSensorSelect03[] = { 0x19, 0x32 };
uint8_t OnSensorSelect04[] = { 0x19, 0x33 };
uint8_t OnSensorSelect05[] = { 0x19, 0x34 };
uint8_t OnSensorSelect06[] = { 0x19, 0x35 };
uint8_t OnSensorSelect07[] = { 0x19, 0x36 };
uint8_t OnSensorSelect08[] = { 0x19, 0x37 };
uint8_t OnSensorSelect09[] = { 0x19, 0x38 };
uint8_t OnSensorSelect10[] = { 0x19, 0x39 };
uint8_t OnSensorSelect11[] = { 0x19, 0x40 };

uint8_t OnChildProtectionEnable[]  = { 0x37, 0x00 };
uint8_t OnChildProtectionDisable[] = { 0x38, 0x00 };

uint8_t OnResetConfirmed[] = { 0x22, 0x30 };

uint8_t OnRebootConfirmed[] = { 0x41, 0x41 };

uint8_t OnMenuUpdate[]          = { 0x31, 0x00 };
uint8_t OnMenuHyst[]            = { 0x31, 0x01 };
uint8_t OnMenuTemp[]            = { 0x31, 0x02 };
uint8_t OnMenuWiFi[]            = { 0x31, 0x03 };
uint8_t OnMenuHomeKit[]         = { 0x31, 0x04 };
uint8_t OnMenuSensor[]          = { 0x31, 0x05 };
uint8_t OnMenuChildProtection[] = { 0x31, 0x08 };
uint8_t OnMenuReset[]           = { 0x31, 0x06 };
uint8_t OnMenuReboot[]          = { 0x31, 0x09 };

uint8_t OnSettingsBack[] = { 0x32, 0x00 };

uint8_t OnHome[] = { 0x12, 0x00 };

/* Навигация */
uint8_t ToScreenBlank[] = { 0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00 };
uint8_t ToFirstSetup = 0x28;
uint8_t ToMainScreen = 0x02;
uint8_t ToSettingsScreen = 0x04;
uint8_t ToUpdateAvailableScreen   = 0x07;
uint8_t ToUpdateInstallingScreen  = 0x08;
uint8_t ToDwinUpdateInstallingScreen = 0x32;
uint8_t ToUpdateUnavailableScreen = 0x09;
uint8_t ToHystScreen = 0x0B;
uint8_t ToTempScreen = 0x0D;
uint8_t ToTempSwapCompleteScreen = 0x31;
uint8_t ToWiFiScreen = 0x0E;
uint8_t ToHomeKitNoPairScreen = 0x0F;
uint8_t ToHomeKitPairScreen   = 0x10;
uint8_t ToHomeKitPairBreakingScreen      = 0x18;
uint8_t ToHomeKitPairBreakCompleteScreen = 0x12;
uint8_t ToSensorScreen  = 0x13;
uint8_t ToResetScreen   = 0x15;
uint8_t ToResetSuccessScreen = 0x17;
uint8_t ToChildProtectionEnabledScreen  = 0x38;
uint8_t ToChildProtectionDisabledScreen = 0x37;
uint8_t ToLoadingScreen = 0x18;
uint8_t ToRebootScreen = 0x40;

/* Отображение информации на экране */
uint8_t ShowFirstSetupScreenQrCodeBlank[] = { 0x5A, 0xA5, 0xFF, 0x82, 0x62, 0x00 };

uint8_t ShowCommandMainBlank[] = { 0x5A, 0xA5, 0x05, 0x82, 0x02 };
uint8_t ShowWifiStrength       = 0x25;
uint8_t ShowTargetTempInteger  = 0x20;
uint8_t ShowTargetTempFrac     = 0x21;
uint8_t ShowCurrentTempInteger = 0x22;
uint8_t ShowCurrentTempFrac    = 0x23;
uint8_t ShowHeatStatus         = 0x24;
uint8_t ShowCurrentTempFirst   = 0x26;
uint8_t ShowCountdown          = 0x1D;
uint8_t ShowTopLeftIcon        = 0x1C;

uint8_t ShowCommandUpdateAvailableBlank[] = { 0x5A, 0xA5, 0x05, 0x82, 0x07 };
uint8_t ShowUpdateCurrentVersionBase = 0x20;
uint8_t ShowUpdateNextVersionBase = 0x26;

uint8_t ShowCommandUpdateInstallingBlank[] = { 0x5A, 0xA5, 0x05, 0x82, 0x08 };
uint8_t ShowUpdateInstallingVersionBase = 0x20;
uint8_t ShowUpdateStage = 0x26;
uint8_t ShowUpdatePercent = 0x27;

uint8_t ShowCommandUpdateUnavailableBlank[] = { 0x5A, 0xA5, 0x05, 0x82, 0x09 };

uint8_t ShowCommandHystBlank[] = { 0x5A, 0xA5, 0x05, 0x82, 0x11 };
uint8_t ShowHystInteger    = 0x20;
uint8_t ShowHystFrac       = 0x24;
uint8_t ShowAdjTempSign    = 0x21;
uint8_t ShowAdjTempInteger = 0x22;
uint8_t ShowAdjTempFrac    = 0x23;

uint8_t ShowCommandTempBlank[] = { 0x5A, 0xA5, 0x05, 0x82, 0x13 };
uint8_t ShowMinTemp = 0x20;
uint8_t ShowMaxTemp = 0x21;

uint8_t ShowWifiScreenQrCodeBlank[] = { 0x5A, 0xA5, 0xFF, 0x82, 0x61, 0x00 };

uint8_t ShowHomeKitScreenQrCodeBlank[] = { 0x5A, 0xA5, 0xFF, 0x82, 0x60, 0x00 };

uint8_t ShowSensorStateBlank[] = { 0x5A, 0xA5, 0x05, 0x82, 0x19 };
uint8_t ShowSensorStateBase = 0x19;

/* Прочее */
uint8_t Reset[] = { 0x5A, 0xA5, 0x07, 0x82, 0x00, 0x04, 0x55, 0xAA, 0x5A, 0xA5 };


/* -----------------------------------------------------------------------------
   Общение со внешним миром
   -----------------------------------------------------------------------------
**/

bool isMatchHeader = false;
uint8_t buf[256] = { 0 };


/* -----------------------------------------------------------------------------
   Текущее состояние
   -----------------------------------------------------------------------------
**/

enum class DwinScreen {
  Loading,
  First,
  Main,
  Settings,
  Temp, TempSwapComplete,
  Hyst,
  Sensor,
  HK,
  WiFi,
  Update,
  DwinUpdate,
  ChildProtection,
  Reset,
  Reboot
};

static struct {
  DwinScreen screen;

  float max_target_temp;
  float min_target_temp;

  float hysteresis;
  float temp_adj;

  uint8_t wifi_strength;
  float last_temp;
  float target_temp;
  bool is_active;
  bool is_heating;
  bool is_target_temp_first;

  uint8_t prev_sensor_index;
  uint8_t sensor_index;

  bool pair_hk;

  bool has_update;

  String raw_current_version;
  byte current_version[6];

  String raw_next_version;
  byte next_version[6];

  bool        is_child_protection_enabled;
  bool        is_child_protection_active;
  uint8_t     child_protection_countdown;
  H4_TASK_PTR child_protection_unlock_task;
} state;

void dwin_set_target_min_max(float min, float max) {
  if (state.screen != DwinScreen::Temp) return;

  if (fabs(min - state.min_target_temp) > 0.0001) {
    state.min_target_temp = min;
    showMinTargetTemp();
  }

  if (fabs(max - state.max_target_temp) > 0.0001) {
    state.max_target_temp = max;
    showMaxTargetTemp();
  }
}

void dwin_set_hysteresis(float hysteresis) {
  if (state.screen != DwinScreen::Hyst) return;

  if (fabs(hysteresis - state.hysteresis) > 0.0001) {
    state.hysteresis = hysteresis;
    showHysteresisValue();
  }
}

void dwin_set_temp_adj(float temp_adj) {
  if (state.screen != DwinScreen::Hyst) return;

  if (fabs(temp_adj - state.temp_adj) > 0.0001) {
    state.temp_adj = temp_adj;
    showAdjTemp();
  }
}

void dwin_set_wifi_strength(uint8_t wifi_strength) {
  if (state.screen != DwinScreen::Main) return;

  if (wifi_strength != state.wifi_strength) {
    state.wifi_strength = wifi_strength;
    showWifiStrength();
  }
}

void dwin_set_temps(float last_temp, float target_temp) {
  if (state.screen != DwinScreen::Main) return;

  if (fabs(last_temp - state.last_temp) > 0.0001) {
    state.last_temp = last_temp;
    MainScreen();
  }

  if (fabs(target_temp - state.target_temp) > 0.0001) {
    state.target_temp = target_temp;
    MainScreen();
  }
}

void dwin_set_sensor_index(uint8_t sensor_index) {
  if (state.screen != DwinScreen::Sensor) return;

  if (sensor_index != state.sensor_index) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = sensor_index;
    SensorScreen_Reselect(/* change_config */ false);
  }
}

void dwin_set_active(bool is_active) {
  if (state.screen != DwinScreen::Main) return;

  if (is_active != state.is_active) {
    state.is_active = is_active;
    showHeatStatus();
  }
}

void dwin_set_heating(bool is_heating) {
  if (state.screen != DwinScreen::Main) return;

  if (is_heating != state.is_heating) {
    state.is_heating = is_heating;
    showHeatStatus();
  }
}

void dwin_set_is_target_temp_first(bool is_target_temp_first) {
  if (state.screen != DwinScreen::Main) return;

  if (is_target_temp_first != state.is_target_temp_first) {
    state.is_target_temp_first = is_target_temp_first;
    MainScreen();
  }
}

void dwin_hk_has_pair(bool has_pair) {
  state.pair_hk = has_pair;
}

void dwin_has_update(bool has_update) {
  state.has_update = has_update;
}

void dwin_update_set_versions(String current_version, String new_version) {
  if (state.screen != DwinScreen::Update) return;

  if (current_version != state.raw_current_version) {
    state.raw_current_version = current_version;

    state.current_version[0] = state.raw_current_version.charAt(0) - '0';
    state.current_version[1] = state.raw_current_version.charAt(1) - '0';
    state.current_version[2] = state.raw_current_version.charAt(3) - '0';
    state.current_version[3] = state.raw_current_version.charAt(4) - '0';
    state.current_version[4] = state.raw_current_version.charAt(6) - '0';
    state.current_version[5] = state.raw_current_version.charAt(7) - '0';

    showUpdateScreenVersions();
  }

  if (new_version != state.raw_next_version) {
    state.raw_next_version = new_version;

    state.next_version[0] = state.raw_next_version.charAt(0) - '0';
    state.next_version[1] = state.raw_next_version.charAt(1) - '0';
    state.next_version[2] = state.raw_next_version.charAt(3) - '0';
    state.next_version[3] = state.raw_next_version.charAt(4) - '0';
    state.next_version[4] = state.raw_next_version.charAt(6) - '0';
    state.next_version[5] = state.raw_next_version.charAt(7) - '0';

    showUpdateScreenVersions();
  }
}

void dwin_reset() {
  memset(&state, 0, sizeof(state));

  memcpy(&buf, &Reset, sizeof(Reset));
  ScreenSerial.write(buf, sizeof(Reset));
}

void save_changes(String txt, float val) {
  configSetup = jsonWrite(configSetup, txt, String(val));
  spaInit();
  saveFile("config.json", configSetup);
  sendConfigWs(configSetup, "config", webSocketConnect);
  sendDataWs(true, webSocketConnect);
}

void save_changes(String txt, int val) {
  configSetup = jsonWrite(configSetup, txt, String(val));
  saveFile("config.json", configSetup);
  sendConfigWs(configSetup, "config", webSocketConnect);
  sendDataWs(true, webSocketConnect);
}

void save_changes_val(String txt, int val) {
  configSetup = jsonWrite(configSetup, txt, String(val));
  configSetup = jsonWrite(configSetup, txt, String(val));
  saveFile("config.json", configSetup);
}


/* -----------------------------------------------------------------------------
   Общение с экраном
   -----------------------------------------------------------------------------
**/

/* Общее */

void navigateToScreen(uint8_t screen) {
  memcpy(&buf, &ToScreenBlank, sizeof(ToScreenBlank));
  buf[sizeof(ToScreenBlank)] = screen;
  ScreenSerial.write(buf, sizeof(ToScreenBlank) + 1);
}

void showCommand(
  uint8_t *blank,
  uint8_t  blank_len,
  uint8_t  command,
  uint8_t  value
) {
  memcpy(&buf, blank, blank_len);

  buf[blank_len + 0] = command;
  buf[blank_len + 1] = 0x00;
  buf[blank_len + 2] = value;

  ScreenSerial.write(buf, blank_len + 3);
}

void showQrCode(
  uint8_t *blank,
  uint8_t  blank_len,
  String   str
) {
  memcpy(
    &buf,
    blank,
    blank_len
  );

  buf[2] = 3 + str.length();

  ScreenSerial.write(buf, blank_len);
  ScreenSerial.print(str);
}

/* Первоначальная конфигурация */

void showFirstSetupScreenQrCode() {
  showQrCode(
    (uint8_t*) &ShowFirstSetupScreenQrCodeBlank,
    sizeof(ShowFirstSetupScreenQrCodeBlank),
    String("WIFI:S:Lytko-" + decToHex(ChipId(), 4) + ";T:WPA;P:12345678;;")
  );
}


/* Главный экран */

void showLockIcon() {
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowTopLeftIcon,
    0x02
  );
}

void showSettingsIcon() {
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowTopLeftIcon,
    0x00
  );
}

void showChildProtectionCountdown() {
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowCountdown,
    state.child_protection_countdown
  );
}

void hideChildProtectionCountdown() {
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowCountdown,
    0
  );
}

void showWifiStrength() {
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowWifiStrength,
    state.wifi_strength
  );
}

void showBigNumbers(float value) {
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowTargetTempInteger,
    (uint8_t)(value)
  );

  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowTargetTempFrac,
    (uint8_t)((int)(value * 10) % 10)
  );
}

void showLittleNumbers(float value) {
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowCurrentTempInteger,
    (uint8_t)(value)
  );

  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowCurrentTempFrac,
    (uint8_t)((int)(value * 10) % 10)
  );
}

void showCurrentTemperatureFirst(bool is) {
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowCurrentTempFirst,
    is ? 1 : 0
  );
}

void showHeatStatus() {
  uint8_t v;
  if (state.is_active && state.is_heating) {
    v = 8;
  }
  else if (state.is_active) {
    v = 1;
  }
  else {
    v = 0;
  }
  
  showCommand(
    ShowCommandMainBlank,
    sizeof(ShowCommandMainBlank),
    ShowHeatStatus,
    v
  );
}

void showMainScreenActualState() {
  showWifiStrength();

  if (state.is_child_protection_active) {
    showLockIcon();
    showChildProtectionCountdown();
  }
  else {
    showSettingsIcon();
    hideChildProtectionCountdown();
  }

  if (state.is_target_temp_first) {
    showCurrentTemperatureFirst(false);
    showBigNumbers(state.target_temp);
    showLittleNumbers(state.last_temp);
  }
  else {
    showCurrentTemperatureFirst(true);
    showBigNumbers(state.last_temp);
    showLittleNumbers(state.target_temp);
  }

  showHeatStatus();
}

/* Обновления */

void showUpdateAvailableCurrentVersion() {
  for (int i = 0; i < 6; i++) {
    showCommand(
      ShowCommandUpdateAvailableBlank,
      sizeof(ShowCommandUpdateAvailableBlank),
      ShowUpdateCurrentVersionBase + i,
      state.current_version[i]
    );
  }
}

void showUpdateAvailableNextVersion() {
  for (int i = 0; i < 6; i++) {
    auto addr = ShowUpdateNextVersionBase + i;

    if (addr == 0x2A) addr = 0x30;
    if (addr == 0x2B) addr = 0x31;

    showCommand(
      ShowCommandUpdateAvailableBlank,
      sizeof(ShowCommandUpdateAvailableBlank),
      addr,
      state.next_version[i]
    );
  }
}

void showUpdateInstallingNextVersion() {
  for (int i = 0; i < 6; i++) {
    showCommand(
      ShowCommandUpdateInstallingBlank,
      sizeof(ShowCommandUpdateInstallingBlank),
      ShowUpdateInstallingVersionBase + i,
      state.next_version[i]
    );
  }
}

// Index от 1 до 8
void showUpdateInstallingStage(uint8_t index) {
  showCommand(
    ShowCommandUpdateInstallingBlank,
    sizeof(ShowCommandUpdateInstallingBlank),
    ShowUpdateStage,
    index
  );
}

void showUpdateScreenVersions() {
  if (state.has_update) {
    showUpdateAvailableCurrentVersion();
    showUpdateAvailableNextVersion();
  } else {
    showUpdateUnavailableCurrentVersion();
  }
}

void dwin_set_update_percent(uint8_t percent) {
  showUpdateInstallingProgress(percent);
}

void dwin_set_update_stage(uint8_t stage) {
  showUpdateInstallingStage(stage);
}

// Percent от 0 до 100
void showUpdateInstallingProgress(uint8_t percent) {
  showCommand(
    ShowCommandUpdateInstallingBlank,
    sizeof(ShowCommandUpdateInstallingBlank),
    ShowUpdatePercent,
    percent
  );
}

void showUpdateUnavailableCurrentVersion() {
  for (int i = 0; i < 6; i++) {
    showCommand(
      ShowCommandUpdateUnavailableBlank,
      sizeof(ShowCommandUpdateUnavailableBlank),
      ShowUpdateCurrentVersionBase + i,
      state.current_version[i]
    );
  }
}

/* Гистерезис */

void showHysteresisValue() {
  showCommand(
    ShowCommandHystBlank,
    sizeof(ShowCommandHystBlank),
    ShowHystInteger,
    (uint8_t)(fabs(state.hysteresis))
  );

  showCommand(
    ShowCommandHystBlank,
    sizeof(ShowCommandHystBlank),
    ShowHystFrac,
    (uint8_t)((int)(fabs(state.hysteresis) * 10) % 10)
  );
}

void showAdjTemp() {
  showCommand(
    ShowCommandHystBlank,
    sizeof(ShowCommandHystBlank),
    ShowAdjTempSign,
    state.temp_adj > 0.0 ? 0x0B : 0x0A
  );

  showCommand(
    ShowCommandHystBlank,
    sizeof(ShowCommandHystBlank),
    ShowAdjTempInteger,
    (uint8_t)(fabs(state.temp_adj))
  );

  showCommand(
    ShowCommandHystBlank,
    sizeof(ShowCommandHystBlank),
    ShowAdjTempFrac,
    (uint8_t)((int)(fabs(state.temp_adj) * 10) % 10)
  );
}

void showHystScreenActualState() {
  showHysteresisValue();
  showAdjTemp();
}

/* Температура */

void showMinTargetTemp() {
  showCommand(
    ShowCommandTempBlank,
    sizeof(ShowCommandTempBlank),
    ShowMinTemp,
    (uint8_t)(state.min_target_temp)
  );
}

void showMaxTargetTemp() {
  showCommand(
    ShowCommandTempBlank,
    sizeof(ShowCommandTempBlank),
    ShowMaxTemp,
    (uint8_t)(state.max_target_temp)
  );
}

void showTempScreenActualState() {
  showMinTargetTemp();
  showMaxTargetTemp();
}

/* WiFi */

void showWiFiScreenQrCode() {
  showQrCode(
    (uint8_t*) &ShowWifiScreenQrCodeBlank,
    sizeof(ShowWifiScreenQrCodeBlank),
    String("http://" + WiFi.localIP().toString())
  );
}

/* HomeKit */

void showHomeKitCreatePairQrCode() {
  showQrCode(
    (uint8_t*) &ShowHomeKitScreenQrCodeBlank,
    sizeof(ShowHomeKitScreenQrCodeBlank),
    String("X-HM://00909FWEF" + String(SETUP_ID))
  );
}

/* Sensor */

void setSensorState(uint8_t index, int8_t s) {
  showCommand(
    ShowSensorStateBlank,
    sizeof(ShowSensorStateBlank),
    ShowSensorStateBase + index,
    s
  );
}


/* -----------------------------------------------------------------------------
   Команды
   -----------------------------------------------------------------------------
**/

DwinCommand cmd = DwinCommand::Idle;

#define SCREEN(s) state.screen = s;

void FirstScreen() {
  SCREEN(DwinScreen::First);
  navigateToScreen(ToFirstSetup);
  showFirstSetupScreenQrCode();
}

void LoadingScreen() {
  SCREEN(DwinScreen::Loading);
  navigateToScreen(ToLoadingScreen);
}

void MainScreen() {
  SCREEN(DwinScreen::Main);
  navigateToScreen(ToMainScreen);
  showMainScreenActualState();
}

void SettingsScreen() {
  SCREEN(DwinScreen::Settings);
  navigateToScreen(ToSettingsScreen);
}

void UpdateScreen() {
  SCREEN(DwinScreen::Update);

  if (state.has_update) {
    navigateToScreen(ToUpdateAvailableScreen);
  } else {
    navigateToScreen(ToUpdateUnavailableScreen);
  }

  showUpdateScreenVersions();
}

void UpdateScreen_Update() {
  navigateToScreen(ToUpdateInstallingScreen);
  showUpdateInstallingNextVersion();
  lazy_actions.update_esp = true;
}

void UpdateScreen_DwinUpdate() {
  SCREEN(DwinScreen::DwinUpdate);
  navigateToScreen(ToDwinUpdateInstallingScreen);
}

void HystScreen() {
  SCREEN(DwinScreen::Hyst);
  navigateToScreen(ToHystScreen);
  showHystScreenActualState();
}

void TempScreen() {
  SCREEN(DwinScreen::Temp);
  navigateToScreen(ToTempScreen);
  showTempScreenActualState();
}

void TempScreen_Swap() {
  sendWs("log", "SWAP");
  
  SCREEN(DwinScreen::TempSwapComplete);
  navigateToScreen(ToTempSwapCompleteScreen);

  is_target_temp_first = is_target_temp_first == 1 ? 0 : 1;
  configSetup = jsonWrite(configSetup, "is_target_temp_first", String(is_target_temp_first));
  saveFile("config.json", configSetup);

  sendConfigWs(configSetup, "config", webSocketConnect);
  sendDataWs(true, webSocketConnect);

  h4.once(3000, MainScreen);
}

void WiFiScreen() {
  SCREEN(DwinScreen::WiFi);
  navigateToScreen(ToWiFiScreen);
  showWiFiScreenQrCode();
}

void WiFiScreen_Disconnect() {
  FirstScreen();
  wifi_reset();
  lazy_actions.restart_esp = true;
}

void HomeKitScreen() {
  if (homekit) {
    SCREEN(DwinScreen::HK);

    if (state.pair_hk) {
      navigateToScreen(ToHomeKitPairScreen);
    } else {
      navigateToScreen(ToHomeKitNoPairScreen);
      showHomeKitCreatePairQrCode();
    }
  }
}

void dwin_hk_loading_screen() {
  navigateToScreen(ToHomeKitPairBreakingScreen);
}

void HomeKitScreen_BreakPair() {
  navigateToScreen(ToHomeKitPairBreakingScreen);

  configSetup = jsonWrite(configSetup, "pair_hk", "0");
  saveFile("config.json", configSetup);
  clear_homekit();
  lazy_actions.restart_esp = true;
  pair_hk = 0;

  navigateToScreen(ToHomeKitPairBreakCompleteScreen);
}

void SensorScreen() {
  SCREEN(DwinScreen::Sensor);

  for (int i = 0; i < 11; i++) setSensorState(i, 0);
  SensorScreen_Reselect(/* change_config */ false);

  navigateToScreen(ToSensorScreen);

  uint8_t lytko_hack[] = { 0x5A, 0xA5, 0x05, 0x82, 0x19, 0x23, 0x00, 0x02 };
  ScreenSerial.write((uint8_t *) &lytko_hack, sizeof(lytko_hack));
  
  uint8_t lytko_hack2[] = { 0x5A, 0xA5, 0x05, 0x82, 0x19, 0x23, 0x00, 0x04 };
  ScreenSerial.write((uint8_t *) &lytko_hack2, sizeof(lytko_hack2));
}

void SensorScreen_Reselect(bool change_config) {
  setSensorState(state.prev_sensor_index, 0);
  setSensorState(state.sensor_index, 1);

  if (change_config) {
    configSetup = jsonWrite(configSetup, "sensor_model_id", String(state.sensor_index));
    saveFile("config.json", configSetup);
    spaInit();
    tempInit();
    sendConfigWs(configSetup, "config", webSocketConnect);
    sendDataWs(true, webSocketConnect);
  }
}

void ChildProtectionScreen() {
  SCREEN(DwinScreen::ChildProtection);

  if (state.is_child_protection_enabled) {
    navigateToScreen(ToChildProtectionEnabledScreen);
  }
  else {
    navigateToScreen(ToChildProtectionDisabledScreen);
  }
}

void ChildProtectionScreen_Enable() {
  state.is_child_protection_enabled = true;
  configSetup = jsonWrite(configSetup, "child_protection_enabled", String(state.is_child_protection_enabled));
  saveFile("config.json", configSetup);
    
  state.is_child_protection_active  = false;
  state.child_protection_countdown  = DEFAULT_CHILD_PROTECTION_COUNTDOWN;

  ChildProtectionScreen();

  h4.once(1000, ChildProtectionTimer);
}

void ChildProtectionScreen_Disable() {
  state.is_child_protection_enabled = false;
  configSetup = jsonWrite(configSetup, "child_protection_enabled", String(state.is_child_protection_enabled));
  saveFile("config.json", configSetup);
  
  state.is_child_protection_active  = false;
  state.child_protection_countdown  = 0;

  ChildProtectionScreen();
}

void ChildProtectionTimer() {
  if (!state.is_child_protection_enabled) return;
   
  if (--state.child_protection_countdown == 0) {
    state.is_child_protection_active = true;
    MainScreen();
  }
  else {
    h4.once(1000, ChildProtectionTimer);
  }
}

void FreeChildProtection() {
  state.child_protection_countdown = 3;
  showChildProtectionCountdown();
  
  state.child_protection_unlock_task = h4.once(1000, FreeChildProtectionTimer);
}

void FreeChildProtectionTimer() {
  state.child_protection_countdown--;
  if (state.child_protection_countdown != 0) {
    showChildProtectionCountdown();
    state.child_protection_unlock_task = h4.once(1000, FreeChildProtectionTimer);
  }
  else {
    showSettingsIcon();
    hideChildProtectionCountdown();

    state.is_child_protection_enabled = true;
    state.is_child_protection_active  = false;
    state.child_protection_countdown  = DEFAULT_CHILD_PROTECTION_COUNTDOWN;

    h4.once(1000, ChildProtectionTimer);
  }
}

void CancelFreeChildProtectionTimer() {
  state.child_protection_countdown = 3;
  h4.cancel(state.child_protection_unlock_task);
}

void ResetScreen() {
  SCREEN(DwinScreen::Reset);
  navigateToScreen(ToResetScreen);
}

void ResetScreen_ResetConfirmed() {
  navigateToScreen(ToResetSuccessScreen);

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

void RebootScreen() {
  SCREEN(DwinScreen::Reboot);
  navigateToScreen(ToRebootScreen);
}

void RebootScreen_RebootConfirmed() {
  dwin_command(DwinCommand::LoadingScreen);
  lazy_actions.restart_esp = true;
}

void dwin_write_to_ram(uint32_t offset, uint8_t *bytes, uint32_t len) {
  uint32_t address = 0x8000 + (offset / 2);
  uint8_t buffer[] = {
    0x5A, 0xA5,
    0x01 + 0x02 + len,
    0x82,
    (uint8_t) ((address & 0xFF00) >> 8), (uint8_t) ((address & 0x00FF) >> 0)
  };

  ScreenSerial.write(buffer, sizeof(buffer));
  ScreenSerial.write(bytes, len);
  dwin_wait_ok();
}

void dwin_write_ram_to_flash(uint32_t segment) {

  // Write segment from ram to flash
  uint8_t buffer[] = {
    0x5A, 0xA5,  // Magic
    0x0F,        // Size
    0x82,        // Mode: Write
    0x00, 0xAA,  // Command
    0x5A, 0x02,  // Fixed
    (byte) ((segment & 0xFF00) >> 8), (byte) (segment & 0xFF),
    0x80, 0x00,  // Addr
    0x17, 0x70,  // Delay for the next writing operation
    0x00, 0x00, 0x00, 0x00  // Not defined, zeros
  };

  ScreenSerial.write(buffer, sizeof(buffer));
  dwin_wait_ok();
  
  //sendWs("log", "OK");

  // Check status
  uint8_t check_status[] = {
    0x5A, 0xA5, 0x04, 0x83, 0x00, 0xAA, 0x01
  };

  while (true) {
    ScreenSerial.write(check_status, sizeof(check_status));
  
    if (dwin_check_write_status()) {
      //sendWs("log", "SUC");
      return;
    }
    else {
      //sendWs("log", "FAIL");
      delay(10);
    }
  }
}

bool dwin_check_write_status() {
  while (true) {
    int available = ScreenSerial.fixedAvailable();
    if (available < 9) {
      delay(2);
      continue;
    }

    uint8_t bt = 0;
    if ((bt = ScreenSerial.read()) != 0x5A) { continue; }
    if ((bt = ScreenSerial.read()) != 0xA5) { continue; }
    if ((bt = ScreenSerial.read()) != 0x06) { continue; }
    if ((bt = ScreenSerial.read()) != 0x83) { continue; }
    if ((bt = ScreenSerial.read()) != 0x00) { continue; }
    if ((bt = ScreenSerial.read()) != 0xAA) { continue; }
    if ((bt = ScreenSerial.read()) != 0x01) { continue; }

    uint8_t f = ScreenSerial.read();
    uint8_t s = ScreenSerial.read();
    if (f == 0x5A && s == 0x02) {
      return false;
    }
    else if (f == 0x00 && s == 0x02) {
      return true;
    }
    else {
      ScreenSerial.println(String("Read strange ") + f + " " + s);
      continue;
    }
  }
}

void dwin_wait_ok() {
  while (true) {
    int available = ScreenSerial.fixedAvailable();
    if (available < 6) {
      delay(2);
      continue;
    }

    if (ScreenSerial.read() != 0x5A) continue;
    if (ScreenSerial.read() != 0xA5) continue;
    if (ScreenSerial.read() != 0x03) continue;
    if (ScreenSerial.read() != 0x82) continue;
    if (ScreenSerial.read() != 0x4F) continue;
    if (ScreenSerial.read() != 0x4B) continue;

    break;
  }
}

/* -----------------------------------------------------------------------------
   Жизненный цикл
   -----------------------------------------------------------------------------
**/

static void dwin_data() {
#define MATCH(x, c) (memcmp(x, &buf, c) == 0)
#define CMATCH(x)   (memcmp(x, &buf, 2) == 0)

  int available = ScreenSerial.fixedAvailable();

  if (!isMatchHeader) {
    if (available < 6) {
      return;
    }

    for (int i = 0; i < 6; i++) {
      buf[i] = ScreenSerial.read();
    }

    if (MATCH(&OkResponse, 6)) {
      return;
    } else if (MATCH(&NormalEvent, 6)) {
      isMatchHeader = true;
      return;
    }
  }

  if (available < 3) return;

  ScreenSerial.read();           // Читаем 0x01, но он не используется
  buf[0] = ScreenSerial.read();  // Поверх читаем два значащих байта
  buf[1] = ScreenSerial.read();

  if (CMATCH(&OnHeat)) {
    cmd = DwinCommand::MainScreen_Heat;
  } else if (CMATCH(&OnTempUp)) {
    cmd = DwinCommand::MainScreen_TempUp;
  } else if (CMATCH(&OnTempDown)) {
    cmd = DwinCommand::MainScreen_TempDown;
  } else if (CMATCH(&OnSettings)) {
    cmd = DwinCommand::SettingsScreen;
  } else if (CMATCH(&OnHome)) {
    cmd = DwinCommand::MainScreen;
  } else if (CMATCH(&OnSettingsBack)) {
    cmd = DwinCommand::SettingsScreen_Back;
  } else if (CMATCH(&OnUpdate)) {
    cmd = DwinCommand::UpdateScreen_Update;
  } else if (CMATCH(&OnHystUp)) {
    cmd = DwinCommand::HystScreen_HystUp;
  } else if (CMATCH(&OnHystDown)) {
    cmd = DwinCommand::HystScreen_HystDown;
  } else if (CMATCH(&OnAdjUp)) {
    cmd = DwinCommand::HystScreen_AdjUp;
  } else if (CMATCH(&OnAdjDown)) {
    cmd = DwinCommand::HystScreen_AdjDown;
  } else if (CMATCH(&OnTempSwap)) {
    cmd = DwinCommand::TempScreen_Swap;
  } else if (CMATCH(&OnWiFiDisconnect)) {
    cmd = DwinCommand::WiFiScreen_Disconnect;
  } else if (CMATCH(&OnHomeKitContinueBreakPair)) {
    cmd = DwinCommand::HomeKitScreen_BreakPair;
  } else if (CMATCH(&OnSensorSelect01)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 0;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect02)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 1;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect03)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 2;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect04)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 3;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect05)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 4;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect06)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 5;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect07)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 6;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect08)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 7;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect09)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 8;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect10)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 9;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnSensorSelect11)) {
    state.prev_sensor_index = state.sensor_index;
    state.sensor_index = 10;
    cmd = DwinCommand::SensorScreen_Reselect;
  } else if (CMATCH(&OnChildProtectionEnable)) {
    cmd = DwinCommand::ChildProtectionScreen_Enable;
  } else if (CMATCH(&OnChildProtectionDisable)) {
    cmd = DwinCommand::ChildProtectionScreen_Disable;
  } else if (CMATCH(&OnResetConfirmed)) {
    cmd = DwinCommand::ResetScreen_ResetConfirmed;
  } else if (CMATCH(&OnRebootConfirmed)) {
    cmd = DwinCommand::RebootScreen_RebootConfirmed;
  } else if (CMATCH(&OnMenuUpdate)) {
    cmd = DwinCommand::UpdateScreen;
  } else if (CMATCH(&OnMenuHyst)) {
    cmd = DwinCommand::HystScreen;
  } else if (CMATCH(&OnMenuTemp)) {
    cmd = DwinCommand::TempScreen;
  } else if (CMATCH(&OnMenuWiFi)) {
    cmd = DwinCommand::WiFiScreen;
  } else if (CMATCH(&OnMenuHomeKit)) {
    cmd = DwinCommand::HomeKitScreen;
  } else if (CMATCH(&OnMenuSensor)) {
    cmd = DwinCommand::SensorScreen;
  } else if (CMATCH(&OnMenuChildProtection)) {
    cmd = DwinCommand::ChildProtectionScreen;
  } else if (CMATCH(&OnMenuReset)) {
    cmd = DwinCommand::ResetScreen;
  } else if (CMATCH(&OnMenuReboot)) {
    cmd = DwinCommand::RebootScreen;
  }

  isMatchHeader = false;

#undef CMATCH
#undef MATCH
}

void dwin_command(DwinCommand command) {
  cmd = command;
  dwin_lifecycle();
}

void dwin_init() {
  ScreenSerial.begin(115200);

  memset(&state, 0, sizeof(state));

  state.is_child_protection_enabled = jsonReadtoBool(configSetup, "child_protection_enabled");
  if (state.is_child_protection_enabled) {
    state.is_child_protection_active  = false;
    state.child_protection_countdown  = DEFAULT_CHILD_PROTECTION_COUNTDOWN;
    
    h4.once(1000, ChildProtectionTimer);
  }
  
  dwin_command(DwinCommand::LoadingScreen);
}

void dwin_lifecycle() {
  dwin_data();

  if (cmd == DwinCommand::Idle) {
    return;
  } else {
    if (state.is_child_protection_enabled && !state.is_child_protection_active) {
      state.child_protection_countdown  = DEFAULT_CHILD_PROTECTION_COUNTDOWN;
    }
    else if (state.is_child_protection_enabled && state.is_child_protection_active) {
      h4.cancel(state.child_protection_unlock_task);
      hideChildProtectionCountdown();
    }
  }

  switch (cmd) {
    case DwinCommand::FirstScreen:
      FirstScreen();
      break;

    case DwinCommand::LoadingScreen:
      LoadingScreen();
      break;

    case DwinCommand::MainScreen:
      MainScreen();
      break;
    case DwinCommand::MainScreen_Heat:
      if (!state.is_child_protection_active) {
        spaWork = !state.is_active;
        changeSpa(spaWork);
      }
      break;
    case DwinCommand::MainScreen_TempUp:
      if (!state.is_child_protection_active) {
        ChangeSchedule(1, 0.5);
      }
      break;
    case DwinCommand::MainScreen_TempDown:
      if (!state.is_child_protection_active) {
        ChangeSchedule(255, 0.5);
      }
      break;

    case DwinCommand::SettingsScreen:
      if (!state.is_child_protection_active) {
        SettingsScreen();
      } else {
        FreeChildProtection();
      }
      break;

    case DwinCommand::SettingsScreen_Back:
      MainScreen();
      break;

    case DwinCommand::UpdateScreen:
      UpdateScreen();
      break;
    case DwinCommand::UpdateScreen_Update:
      UpdateScreen_Update();
      break;
    case DwinCommand::UpdateScreen_DwinUpdate:
      UpdateScreen_DwinUpdate();
      break;

    case DwinCommand::HystScreen:
      HystScreen();
      break;
    case DwinCommand::HystScreen_HystUp:
      hysteresis += 0.5;
      if (hysteresis > 5) hysteresis = 5;
      save_changes("hysteresis", hysteresis);
      break;
    case DwinCommand::HystScreen_HystDown:
      hysteresis -= 0.5;
      if (hysteresis < 0) hysteresis = 0;
      save_changes("hysteresis", hysteresis);
      break;
    case DwinCommand::HystScreen_AdjUp:
      sensor_correction += 0.5;
      if (sensor_correction > 5) sensor_correction = 5;
      save_changes("sensor_corr", sensor_correction);
      break;
    case DwinCommand::HystScreen_AdjDown:
      sensor_correction -= 0.5;
      if (sensor_correction < -5) sensor_correction = -5;
      save_changes("sensor_corr", sensor_correction);
      break;

    case DwinCommand::TempScreen:
      TempScreen();
      break;
    case DwinCommand::TempScreen_Swap:
      TempScreen_Swap();
      break;

    case DwinCommand::WiFiScreen:
      if (!state.is_child_protection_active) {
        WiFiScreen();
      }
      break;
    case DwinCommand::WiFiScreen_Disconnect:
      WiFiScreen_Disconnect();
      break;

    case DwinCommand::HomeKitScreen:
      HomeKitScreen();
      break;
    case DwinCommand::HomeKitScreen_BreakPair:
      HomeKitScreen_BreakPair();
      break;

    case DwinCommand::SensorScreen:
      SensorScreen();
      break;
    case DwinCommand::SensorScreen_Reselect:
      SensorScreen_Reselect(/* change_config */ true);
      break;

    case DwinCommand::ChildProtectionScreen:
      ChildProtectionScreen();
      break;
    case DwinCommand::ChildProtectionScreen_Enable:
      ChildProtectionScreen_Enable();
      break;
    case DwinCommand::ChildProtectionScreen_Disable:
      ChildProtectionScreen_Disable();
      break;

    case DwinCommand::ResetScreen:
      ResetScreen();
      break;
    case DwinCommand::ResetScreen_ResetConfirmed:
      ResetScreen_ResetConfirmed();
      break;

    case DwinCommand::RebootScreen:
      RebootScreen();
      break;
    case DwinCommand::RebootScreen_RebootConfirmed:
      RebootScreen_RebootConfirmed();
      break;   
  }

  cmd = DwinCommand::Idle;

  sendDataWs(true, webSocketConnect);
}

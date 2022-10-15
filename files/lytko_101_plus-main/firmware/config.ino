void first_initialize() {
  bool change = false;
  String temp = jsonRead(configSetup, "set");
  if (temp == NULL || temp == "0") {
    firstOn = true;
    change = true;

    temp = jsonRead(configSetup, "dwin_version");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "dwin_version", "01.01.02");
      change = true;
    }

    temp = jsonRead(configSetup, "dwin_id");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "dwin_id", String(dwin_did));
      change = true;
    }

    temp = jsonRead(configSetup, "pair_hk");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "pair_hk", "0");
      change = true;
    }
    
    temp = jsonRead(configSetup, "child_protection_enabled");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "child_protection_enabled", String(false));
      change = true;
    }

    temp = jsonRead(configSetup, "mqtt_server");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "mqtt_server", "");
      configSetup = jsonWrite(configSetup, "mqtt_port", "");
      configSetup = jsonWrite(configSetup, "mqtt_login", "");
      configSetup = jsonWrite(configSetup, "mqtt_use", "0");
      change = true;
    }

    temp = jsonRead(configSetup, "sensor_internal_use");
    if (temp == NULL || temp == "255") {
     if(!int_sens.ds_htu){ 
      PORT_OUT(0);
      PORT_HIGH(0);
      delay(1000);
      intdallasInit();
      //Serial.println(sensors.getDeviceCount());
      if(sensors.getAddress(intDS, 0)!=0) {
        configSetup = jsonWrite(configSetup, "sensor_internal_use", "0");
        //sensors.getAddress(intDS, 0);
        int_ds_addr = jsonWrite(int_ds_addr, "0", "0");
        for(byte i = 0; i<8; i++)
         int_ds_addr = jsonWrite(int_ds_addr, String(i+1), intDS[i]);
        saveFile("intDS.json", int_ds_addr);
        Serial.println(int_ds_addr);
      }
      else {
        configSetup = jsonWrite(configSetup, "sensor_internal_use", "255");
        int_ds_addr = "{}";
        saveFile("intDS.json", int_ds_addr);
      }
     }
     else {
      configSetup = jsonWrite(configSetup, "sensor_internal_use", "0");
     }
     tempInit();
     change = true;
    }

    temp = jsonRead(configSetup, "is_target_temp_first");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "is_target_temp_first", "1");
      is_target_temp_first = 1;
      change = true;
    }

    temp = jsonRead(configSetup, "sensor_model_id");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "sensor_model_id", "0");
      change = true;
    }

    temp = jsonRead(configSetup, "wifi_name");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "wifi_name", "");
      change = true;
    }

    temp = jsonRead(configSetup, "sensor_corr");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "sensor_corr", "0.00");
      change = true;
    }

    temp = jsonRead(configSetup, "hysteresis");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "hysteresis", "1.00");
      change = true;
      tempInit();
    }

    temp = jsonRead(configSetup, "max_temp");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "max_temp", "40");
      change = true;
    }

    temp = jsonRead(configSetup, "min_temp");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "min_temp", "15");
      change = true;
    }

    temp = jsonRead(configSetup, "lock_mode");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "lock_mode", 0);
      change = true;
    }

    temp = jsonRead(configSetup, "homekit");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "homekit", 0);
      change = true;
    }
    
    temp = jsonRead(configSetup, "model");
    if (temp == NULL) {
      configSetup = jsonWrite(configSetup, "model", "101+");
      change = true;
    }

    if (change) {
      saveFile("config.json", configSetup);
    }

    temp = jsonRead(ScheduleSetup, "schedule");
    if (temp == NULL) {
      ScheduleSetup = jsonWrite(ScheduleSetup, "schedule", 23);
      saveFile("schedule.json", ScheduleSetup);
    }

    temp = jsonRead(signals, "reset_count");
    if (temp == NULL) {
      signals = jsonWrite(signals, "reset_count", 0);
      saveFile("signals.json", signals);
    }

    temp = jsonRead(signals, "homekit");
    if (temp == NULL) {
      signals = jsonWrite(signals, "homekit", 0);
      saveFile("signals.json", signals);
    }
  }
}


void MQTT_init(){
  //Serial.println("MQTT");
  
  const char* mqtt_server_ip;
    String temp = jsonRead(configSetup, "mqtt_server")+":"+jsonRead(configSetup, "mqtt_port");
    mqtt_server_ip = temp.c_str();
    if (temp.length()>5){
      namemqtt = "";
      idmqtt = String(ChipId());
      namemqtt = jsonRead(configSetup, "mqtt_login");
      if(jsonRead(configSetup, "mqtt_server")=="lytko.com" || jsonRead(configSetup, "mqtt_server")=="mqtt.lytko.com"){
        lytko = true;
        if(namemqtt!="") devnamemqtt=namemqtt+"/";
        else devnamemqtt = "";
      }
      else devnamemqtt = "";
      devnamemqtt+="climate/lytko/";
      devnamemqtt+= idmqtt;   
  
      if(homekit!=2) mqttClient.onMqttConnect(onMqttConnect);
      mqttClient.onMqttDisconnect(onMqttDisconnect);
      mqttClient.onMqttMessage(onMqttMessage);
      mqttClient.onMqttError(onMqttError);

        const char* loginstr = strdup(jsonRead(configSetup, "mqtt_login").c_str());
        const char* passstr = strdup(jsonRead(configWifi, "mqtt_password").c_str());
      
      mqttClient.setServer(mqtt_server_ip,loginstr,passstr);
      mqttClient.setWill((devnamemqtt+"/available").c_str(),0,false,"offline");
      mqttClient.setKeepAlive(20);
 
      //if(wifiSTA) connectToMqtt();
      mqtt_needToConnect = true;
      have_mqtt = true;
      mqtt_connect = false;
      sec10 = 4;
    }
}
void connectToMqtt() {
  //Serial.println(F("Connecting to MQTT..."));
  //writeFile("Connecting to MQTT...");
  //Serial.println(ESP.getFreeHeap());
  mqttClient.connect(String(ChipId()).c_str(), true);
}

void getAlice(byte link){
     needAlice = false;
      //Serial.println("Alice");
      mqttClient.onMqttConnect(onAliceConnect);
      mqttClient.onMqttDisconnect(onAliceDisconnect);
      mqttClient.onMqttMessage(onMqttMessage);
      mqttClient.onMqttError(onMqttError);

        const char* loginstr = strdup(jsonRead(configSetup, "alice_login").c_str());
        const char* passstr = strdup(jsonRead(configWifi, "alice_pass").c_str());
      lytko = true;
      devnamemqtt = jsonRead(configSetup, "alice_login")+"/climate/lytko/"+String(ChipId());
      namemqtt = jsonRead(configSetup, "alice_login");
      mqttClient.setServer("mqtt.lytko.com:1883", loginstr,passstr);
      mqttClient.setWill((devnamemqtt+"/available").c_str(),0,false,"offline");
      mqttClient.setKeepAlive(20);
     if(link!=0) alice_init();
     else alice_needToConnect = false;
}

void alice_init(){
      mqttClient.connect(String(ChipId()).c_str(), true);
}

void onAliceError(uint8_t e,uint32_t info){
  //Serial.printf("VALUE OF TCP_DISCONNECTED=%d\n",e);
}

void onAliceConnect(bool sessionPresent) {
  alice = 1;
  alice_needToConnect = false;
  //Serial.println(F("Connected to AliceMQTT."));
  //Serial.print(F("Session present: "));
  //Serial.println(sessionPresent); 
  mqttClient.subscribe({(devnamemqtt+"/temperature/set").c_str(),(devnamemqtt+"/stepUp/set").c_str(),(devnamemqtt+"/stepDown/set").c_str(),(devnamemqtt+"/power/set").c_str(),(devnamemqtt+"/config/set").c_str(),(devnamemqtt+"/update/set").c_str()}, 0);

  mqttClient.subscribe("/time", 0);

  String temp = jsonRead(configSetup, "mqtt_external_topic");
  if(temp!= NULL){
    mqttClient.subscribe(temp.c_str(), 0);
    have_mqtt_external_topic = true;
    ext_sens.temp = 0;
    ext_sens.last = 0;
    ext_sens.is = 0;
  }
  configSetup = jsonWrite(configSetup, "mqtt_alice", 1);
  configSetup = jsonWrite(configSetup, "mqtt_alice", 1);
  sendConfigWs(configSetup, "config",0);
  
  make_discovery_topic(namemqtt+"/homeassistant/climate/lytko/"+String(ChipId())+"/config"); 
  
  sendDataWs(true, true);
  mqtt_send_msg(devnamemqtt+"/config", configSetup.c_str(), true);
  mqtt_send_msg(devnamemqtt+"/available","online",true);
}

void onAliceDisconnect(int8_t reason) {
  //if (WiFi.isConnected() && alice) {
    alice_needToConnect = true;
  //}
  mqtt_connect = false;
  alice = 0;
    //Serial.printf("Disconnected from AliceMQTT.%d  ", reason);
    //sendConfigWs("{\"wifi_signal\":\""+String(constrain((WiFi.RSSI()+100)*2, 0, 100))+"\", \"mqtt_status\":\""+String(mqtt_connect)+"\", \"socket_status\":\"1\", \"alice_status\":\""+String(alice)+"\", \"homekit\":"+String(homekit)+"}", "signals", 0);
    configSetup = jsonWrite(configSetup, "mqtt_alice", String(alice));
    saveFile("config.json", configSetup);
    sendConfigWs(configSetup, "config",0);
    //Serial.println(ESP.getFreeHeap());
}

void onMqttConnect(bool sessionPresent) {
  //time_disconnect = millis();
  mqtt_connect = true;
  //numDisc = 0;
  mqtt_needToConnect = false;
  //Serial.println(F("Connected to MQTT."));
  //writeFile("Connected to MQTT.");
  //Serial.print(F("Session present: "));
  //Serial.println(sessionPresent); 
  //mqttClient.setWill((devnamemqtt+"/available").c_str(),0,false,"offline");                                    
  mqttClient.subscribe({(devnamemqtt+"/temperature/set").c_str(),(devnamemqtt+"/stepUp/set").c_str(),(devnamemqtt+"/stepDown/set").c_str(),(devnamemqtt+"/power/set").c_str(),(devnamemqtt+"/config/set").c_str(),(devnamemqtt+"/update/set").c_str()}, 0);
  //mqttClient.subscribe({("alice/"+devnamemqtt+"/temperature/set").c_str(),("alice/"+devnamemqtt+"/stepUp/set").c_str(),("alice/"+devnamemqtt+"/stepDown/set").c_str(),("alice/"+devnamemqtt+"/power/set").c_str(),("alice/"+devnamemqtt+"/config/set").c_str()}, 0);
  //Serial.println(devnamemqtt);
  //Serial.println(F("Subscribed."));
  mqttClient.subscribe("/time", 0);
  //mqttClient.subscribe((devnamemqtt+"/ping").c_str(), 0);
  String temp = jsonRead(configSetup, "mqtt_external_topic");
  if(temp!= NULL){
    mqttClient.subscribe(temp.c_str(), 0);
    have_mqtt_external_topic = true;
    ext_sens.temp = 0;
    ext_sens.last = 0;
    ext_sens.is = 0;
    //time_ssdp = millis()-112000;
  }
  mqtttopics = jsonWrite(mqtttopics, "state", devnamemqtt+"/state"); 
  mqtttopics = jsonWrite(mqtttopics, "state", devnamemqtt+"/state");
  mqtttopics = jsonWrite(mqtttopics, "target_temp", devnamemqtt+"/temperature/set");
  mqtttopics = jsonWrite(mqtttopics, "step_up", devnamemqtt+"/stepUp/set");
  mqtttopics = jsonWrite(mqtttopics, "step_down", devnamemqtt+"/stepDown/set");
  mqtttopics = jsonWrite(mqtttopics, "heating", devnamemqtt+"/power/set");
  //mqtttopics = jsonWrite(mqtttopics, "name", namemqtt+"/"+idmqtt+"/set/name");

  if(!lytko) {
    make_discovery_topic("homeassistant/"+devnamemqtt+"/config"); 
    make_discovery_update_topic("homeassistant/binary_sensor/lytko/"+idmqtt+"/config");
  }
  else {
    make_discovery_topic(namemqtt+"/homeassistant/climate/lytko/"+idmqtt+"/config"); 
  }
  //Serial.println(F("Discovery sended."));
  configSetup = jsonWrite(configSetup, "mqtt_use", "1");
  sendConfigWs(configSetup, "config",0);
  
  sendDataWs(true, true);
  mqtt_send_msg(devnamemqtt+"/config", configSetup.c_str(), true);
  mqtt_send_msg(devnamemqtt+"/available","online",true);
    sendConfigWs(mqtttopics, "mqtt_topics",0);
    //sendConfigWs("{\"wifi_signal\":\""+String(constrain((WiFi.RSSI()+100)*2, 0, 100))+"\", \"mqtt_status\":\""+String(mqtt_connect)+"\", \"socket_status\":\"1\", \"alice_status\":\""+String(alice)+"\", \"homekit\":"+String(homekit)+"}", "signals", 0);
}

void onMqttDisconnect(uint8_t reason) {
  //numDisc++;
  mqtt_connect = false;
    //Serial.printf("Disconnected from MQTT.%d  ", reason);
    //Serial.println(ESP.getFreeHeap());
  configSetup = jsonWrite(configSetup, "mqtt_use", "0");
  sendConfigWs(configSetup, "config",0);
    mqtt_needToConnect = true;
    h4.once(30000,connectToMqtt);
    //if(millis()-time_disconnect<60000) ESP.restart();;
}
void onMqttMessage(const char* topic, const uint8_t* payload, size_t len,uint8_t qos,bool retain,bool dup) {
 if(len!=0){ 
  String str(topic);
  String topicstr = str.substring(str.indexOf("/"));
  //Serial.println(topicstr);
  //Serial.println((char*)payload);
  //Serial.printf("T=%u Message %s qos%d dup=%d retain=%d len=%d\n",millis(),topic,props.qos,props.dup,props.retain,len);

  if(topicstr.indexOf("/stepUp/set")!=-1){
     String num;
     mqttClient.xPayload(payload,len,num);
     ChangeSchedule(1, num.toFloat());
  }
  else if(topicstr.indexOf("/stepDown/set")!=-1){
     String num;
     mqttClient.xPayload(payload,len,num);
     ChangeSchedule(255, num.toFloat());
  }
  else if(topicstr.indexOf("/power/set")!=-1){
    valToHeat = "";
     mqttClient.xPayload(payload,len,valToHeat);
     if(valToHeat =="ON" || valToHeat =="on") {
      if(spaWork==false){
        changeSpa(true);
        sendDataWs(true,webSocketConnect);
      }
     }
     else if(valToHeat =="OFF" || valToHeat =="off") {
      if(spaWork==true){
        changeSpa(false);
        sendDataWs(true,webSocketConnect);
      }
     }
  }
  else if(topicstr.indexOf("/connect")!=-1){
    DynamicJsonBuffer jsonBuffer;
      JsonObject& data = jsonBuffer.parseObject(payload);
    String dat = data.get<String>("wifi_connect");
              JsonObject& object = jsonBuffer.parseObject(payload);
              if(object.containsKey("ssid")){
                configWifi = jsonWrite(configWifi, "ssid", object.get<String>("ssid"));
                configWifi = jsonWrite(configWifi, "ssidPass", object.get<String>("password"));
                configWifi = jsonWrite(configWifi, "ssid", object.get<String>("ssid"));
                configWifi = jsonWrite(configWifi, "ssidPass", object.get<String>("password"));
              }
              saveFile("wifi.save.json", configWifi);
              //Serial.println("saveOK");
  }
  else if(topicstr.indexOf("/config/set")!=-1){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(payload);
              if(object.containsKey("min_temp")){
                configSetup = jsonWrite(configSetup, "min_temp", object.get<String>("min_temp"));
              }
              if(object.containsKey("max_temp")){
                configSetup = jsonWrite(configSetup, "max_temp", object.get<String>("max_temp"));
              }
              if(object.containsKey("is_target_temp_first")){
                String dat = object.get<String>("is_target_temp_first");
                if (dat=="0") { configSetup = jsonWrite(configSetup, "is_target_temp_first", "0"); is_target_temp_first = 0;}
                else { configSetup = jsonWrite(configSetup, "is_target_temp_first", "1"); is_target_temp_first = 1;}
              }
              if(object.containsKey("sensor_corr")){
                configSetup = jsonWrite(configSetup, "sensor_corr", object.get<String>("sensor_corr"));
              }
              if(object.containsKey("hysteresis")){
                configSetup = jsonWrite(configSetup, "hysteresis", object.get<String>("hysteresis"));
              }
              if(object.containsKey("sensor_internal_use")){
                configSetup = jsonWrite(configSetup, "sensor_internal_use", object.get<String>("sensor_internal_use"));
                tempInit();
              }
              if(object.containsKey("sensor_model_id")){
                configSetup = jsonWrite(configSetup, "sensor_model_id", object.get<String>("sensor_model_id"));
                tempInit();
              }
              if(object.containsKey("name")){
                configSetup = jsonWrite(configSetup, "name", object.get<String>("name"));
              }
              spaInit();
              saveFile("config.json", configSetup);
              sendConfigWs(configSetup, "config",webSocketConnect);
              mqtt_send_msg(devnamemqtt+"/config", configSetup.c_str(), true);
              //Serial.println("saveOK");
  }
  else if(topicstr.indexOf("/temperature/set")!=-1){
    String num;
    mqttClient.xPayload(payload,len,num);
    ChangeSchedule(0, num.toFloat());
  }
  else if(topicstr.indexOf("/update/set")!=-1){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(payload);
    if(object.containsKey("update_start")){
      if(object.get<int>("update_start")) {
        lazy_actions.update_esp = true;
        mqtt_update = true;
      }
    }
  }
  else if(topicstr.indexOf("/time")!=-1){
    timeZoneStr = "";
    mqttClient.xPayload(payload,len,timeZoneStr);
  }
  else if(have_mqtt_external_topic && str.indexOf(jsonRead(configSetup, "mqtt_external_topic"))!=-1){
    ext_sens.is = 1;
    ext_sens.last = ext_sens.temp;
    String num;
    mqttClient.xPayload(payload,len,num);
    num.replace(",", ".");
    ext_sens.temp = num.toFloat();
    ext_sens.stime = millis();
    //time_ssdp = millis()-112000;
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonObject& datas = root.createNestedObject("0"); 
      datas["ShotAddr"] = "1";     
      JsonArray& mqtt_type = datas.createNestedArray("type");
      mqtt_type.add("sensor_temp");
      JsonArray& mqtt_unit = datas.createNestedArray("unit");
      mqtt_unit.add("Celsius");
      JsonArray& mqtt_datas = datas.createNestedArray("data");
      mqtt_datas.add(String(ext_sens.temp,1));
    mqtt_data="";
    root.printTo(mqtt_data);
    sendConfigWs(mqtt_data, "mqtt_data",0);
  }
 }
}

void onMqttError(uint8_t e,uint32_t info){
  //Serial.printf("VALUE OF TCP_DISCONNECTED=%d\n",TCP_DISCONNECTED);
}


void start_MQTT(String json){
    // ************* ЦИКЛ ОТПРАВКИ СООБЩЕНИЙ ******************************
    if ((mqtt_connect || alice) && wifiSTA) {  
       mqtt_send_msg(devnamemqtt+"/state", json, true);//temp.c_str());
    }
}

void mqtt_send_msg(String topic, String msg, bool rt){
  mqttClient.xPublish(topic.c_str(),msg, 0, rt);
  //Serial.println("Send");
}

void make_discovery_topic(String topic){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["name"] = devname;
  json["uniq_id"] = String(ChipId());
  //json["icon"] = "mdi:thermostat";
  json["act_t"] = devnamemqtt+"/state";
  json["act_tpl"] = "{{ \"heating\" if value_json.relay == \"1\" else \"idle\" }}";
  json["pow_cmd_t"] = devnamemqtt+"/power/set";
  json["temp_cmd_t"] = devnamemqtt+"/temperature/set";
  json["mode_stat_t"] = devnamemqtt+"/state";
  json["mode_stat_tpl"] = "{{ \"heat\" if value_json.heating == \"heat\" else \"off\" }}";
  json["temp_stat_t"] = devnamemqtt+"/state";
  json["temp_stat_tpl"] = "{{ value_json.target_temp }}";
  json["curr_temp_t"] = devnamemqtt+"/state";
  json["curr_temp_tpl"] = "{{ value_json.temp }}";
  json["min_temp"] = jsonReadtoInt(configSetup, "min_temp");
  json["max_temp"] = jsonReadtoInt(configSetup, "max_temp");
  json["temp_step"] = "0.5";
  json["avty_t"] = devnamemqtt+"/available";
  json["pl_avail"] = "online";
  json["pl_not_avail"] = "offline";
  JsonObject& dev = json.createNestedObject("dev");
  //JsonObject& cns = dev.createNestedObject("cns");
  //char mmm[6];
  //sprintf(mmm, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  //cns["mac"] = String(mmm);
  dev["ids"] = String(ChipId());
  dev["name"] = devname;
  dev["mf"] = "lytko.com";
  dev["mdl"] = "Lytko-101";
  dev["sw"] = ver;
  JsonArray& modes = json.createNestedArray("modes");
  modes.add("off");
  modes.add("heat");
  String output;
  json.printTo(output);
  mqtt_send_msg(topic, output, true);
  //json.printTo(Serial);
}

void make_discovery_update_topic(String topic){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["name"] = "thermostat_updater";//devname;
  json["uniq_id"] = String(ChipId());
  //json["icon"] = "mdi:checkbox";
  json["state_topic"] = devnamemqtt+"/state";
  json["value_template"] = "{{ value_json.newfw }}";
  json["payload_off"] =  "0";
  json["payload_on"] =  "1";
  //json["avty_t"] = "climate/"+devnamemqtt+"/available";
  //json["pl_avail"] = "online";
  //json["pl_not_avail"] = "offline";
   JsonObject& dev = json.createNestedObject("dev");
  dev["ids"] = String(ChipId());
  dev["name"] = devname;
  dev["mf"] = "lytko.com";
  dev["mdl"] = "Lytko-101";
  dev["sw"] = ver;
  String output;
  json.printTo(output);
  mqtt_send_msg(topic, output, true);
  //json.printTo(Serial);
}
//HomeAssistant
/*
  binary_sensor:

        - platform: mqtt
          name: "lytko_mqtt_firmware_binary_sensor"
          state_topic: "climate/lytko/2692784/state"
          value_template: "{{ value_json.newfw }}"
          payload_off: "0"
 */

/*
 homeassistant/climate/lytko/${id}/config
{
  "name": "lytko-${id}",                                       // device name
  "uniq_id": "${id}",                                          // Id, use: 2666848
  "ic": "mdi:thermostat",                                      // Icon
  "act_t": "climate/lytko/${id}/state",                        // action_topic, value from list: [off, heating, cooling, drying, idle, fan], use: {{ 'heating' if value_json['update']['relay'] == '1' else 'idle' }}
  "act_tpl": "{{ json_value.relay }}",                         // action_template
  "pow_cmd_t": "climate/lytko/${id}/power/set"                 // power_command_topic values: heat, off
  "temp_cmd_t": "climate/lytko/${id}/temperature/set"          // temperature_command_topic, value: INT, use: mqtt/2666848/set/targetTemp
  "temp_hi_cmd_t": "climate/lytko/${id}/temperatureUp/set"     // temperature_high_command_topic, value: 1, use: mqtt/2666848/set/tempUp
  "temp_lo_cmd_t": "climate/lytko/${id}/temperatureDown/set"   // temperature_low_command_topic, value: 1, use: mqtt/2666848/set/tempDown
  "dev": {                                                     // device info
       "cns": {
          "mac": "00:11:22:33:44:55",
        },
      'ids':                 'identifiers', // For example a serial number
      'name':                'Lytko', // The name of the device
      'mf':                  'manufacturer', // The manufacturer of the device
      'mdl':                 'model', // The model of the device
      'sw':                  'sw_version', // The firmware version of the device
  },
  "mode_stat_t": "climate/lytko/${id}/state",                   // mode_state_topic, value: { mode: off, relay: off, setTemperature: 27.0, currentTemperature: 27.0 }, use: "{{ 'heat' if value_json['update']['heating'] == '1' else 'off' }}"
  "mode_stat_tpl": "{{ value_json.mode }}",                     // mode_state_template 
  "avty_t": "climate/lytko/${id}/available",                    // availability_topic
  "pl_avail": "online",
  "pl_not_avail": "offline",
  "temp_stat_t":"climate/lytko/${id}/state",                    // temperature_state_template, use: "{{ value_json['update']['target_temp'] }}"
  "temp_stat_tpl":"{{ value_json.setTemperature }}",
  "curr_temp_t":"climate/lytko/${id}/state",                    // current_temperature_topic, use: "{{ value_json['update']['temp'] }}"
  "curr_temp_tpl":"{{ value_json.currentTemperature }}",
  "min_temp":"7.0",
  "max_temp":"75.0",
  "temp_step":"0.5",
  "modes":["off", "heat"]
}
 */

#include <ESP8266WiFiGratuitous.h>

void WIFIinit(byte first) {
  WiFi.setOutputPower(17.5);
  WiFi.persistent(true);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  
  
  configWifi = readFile("wifi.save.json", 200);
  // Попытка подключения к точке доступа
  //Serial.println("WiFi Init");
  //enableWiFiAtBootTime();
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  if (!WiFi.getAutoConnect()) { WiFi.setAutoConnect(true); }
  String temp = "LYTKO-101-";
  temp+=String(ChipId());
  WiFi.hostname(temp);
  byte tries = 5;
  String _ssid     = jsonRead(configWifi, "ssid").c_str();
  String _password = jsonRead(configWifi, "ssidPass").c_str();
  //Serial.println(_ssid);Serial.println(_password);
  experimental::ESP8266WiFiGratuitous::stationKeepAliveSetIntervalMs(5000);
  
//  if (WiFi.status() != WL_CONNECTED)
//  {
    if(_ssid!=""){
     //WiFi.disconnect(); 
     ESP.eraseConfig();   
     WiFi.begin(_ssid.c_str(), _password.c_str());
    }
    else {ESP.eraseConfig(); WiFi.begin(); }
    tries=20;
    while (--tries && WiFi.status() != WL_CONNECTED)
    {
      Serial.print(F("."));      
      delay(1000);
    }
//  }
  
  if (WiFi.status() != WL_CONNECTED)
  {
    if(first!=0){
    // Если не удалось подключиться запускаем в режиме AP
     wifiSTA = StartAPMode();
     firstOn = true;
    }
  }
  else {
    // Иначе удалось подключиться отправляем сообщение
    // о подключении и выводим адрес IP
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);//(WIFI_AP_STA);  
    WiFi.setOutputPower(17.5);
    WiFi.macAddress(mac);

    requestSSDP();

    configSetup = jsonWrite(configSetup, "set", "1"); 
    configSetup = jsonWrite(configSetup, "set", "1");
    configSetup = jsonWrite(configSetup, "version", ver);
    firstOn = false;
    if(first==2) {
      if(hk_started)h4.once(1000,homekit_update_config_number);//homekit_setup);
      else h4.once(1000,homekit_setup);
    }
    wifiSTA = true;
    wifi_con = 0;
  }
}
void scanWiFi(){
  int n = WiFi.scanComplete();
  Serial.println(n);
  if(n == -2){
    WiFi.scanNetworks(false);
  } else if(n>0){
    wifi_scanned = n;
    printScanResult(wifi_scanned);
    //WiFi.scanDelete();
  }
  else WiFi.scanNetworks(false);
}
void printScanResult(byte networksFound)
{
    String _ssid = jsonRead(configWifi, "ssid").c_str();
    String wifijson = "";
    //_ssid = "M_8221";
    //byte power=0;
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    JsonArray& wifi_networks = json.createNestedArray("wifi_networks");
  
    for (int i = 0; i < networksFound; i++) {
     if(WiFi.SSID(i)!=""){ 
      JsonObject& scan = wifi_networks.createNestedObject();
      //scan[WiFi.SSID(i)] = (WiFi.RSSI(i)+100)*2;
      scan["ssid"] = (WiFi.SSID(i));
      if(WiFi.SSID(i)==_ssid){
        router=true;
        /*if((WiFi.RSSI(i)+100)*2>power){
          _bssid = WiFi.BSSID(i); 
          power = (WiFi.RSSI(i)+100)*2; 
          chan = WiFi.channel(i);
        }*/
      }
      //scan["bssid"] = (WiFi.BSSIDstr(i));   
      scan["chan"] = WiFi.channel(i);
      scan["signal"] = String((WiFi.RSSI(i)+100)*2);
      scan["encryption"] = String(WiFi.encryptionType(i));
     }
    }
    wifijson="";
    json.printTo(wifijson);
    //Serial.println(wifijson);
  if(numCon>0){
    size_t len = json.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
        json.printTo((char *)buffer->get(), len + 1);
        ws.textAll(buffer);
    }
  }
}

void wifi_reset(){
  configSetup = jsonWrite(configSetup, "set", "0");
  saveFile("config.json", configSetup);
  
  configWifi = jsonWrite(configWifi, "ssid", "");
  configWifi = jsonWrite(configWifi, "ssidPass", "");
  configWifi = jsonWrite(configWifi, "ssid", "");
  configWifi = jsonWrite(configWifi, "ssidPass", "");
  saveFile("wifi.save.json", configWifi);
  ESP.eraseConfig();
}

void check_wifi(){
  if(wifiSTA){
    //if(WiFi.status() != WL_CONNECTED){
    if(WiFi.RSSI() > 0){
      WIFIinit(wifiSTA);
    }
  }
  else {
    if(router){
      wifi_con++;
      if(wifi_con>10) {
        //wifi_reset();
      }
       WIFIinit(2);
    }
    else {
      String _ssid = jsonRead(configWifi, "ssid").c_str();
      if(_ssid!=NULL && _ssid!="")
      scanWiFi();
      WiFi.scanNetworks(false);
    }
  }
}

bool StartAPMode() {
  first_initialize();
  // Отключаем WIFI
  WiFi.disconnect();
  // Меняем режим на режим точки доступа
  WiFi.mode(WIFI_AP);
  String softAPname = "Lytko-";
  softAPname+=decToHex(ChipId(), 4);
  WiFi.softAP(softAPname.c_str(),"12345678");
  dns.setErrorReplyCode(AsyncDNSReplyCode::NoError);
  dns.start(DNS_PORT, "*", WiFi.softAPIP());//*/
  router = false;
  configSetup = jsonWrite(configSetup, "set", "0");
  return false;
}

void SSDP_init(void) {
//if(!homekit){
  if (devname == "") {
    devname = "LYTKO-"+String(ChipId());
    configSetup = jsonWrite(configSetup, "name", devname);
    saveFile ("config.json", configSetup);
  }
  //first = jsonWrite(first, "id", String(ChipId()));
  //first = jsonWrite(first, "name", devname);
  if(udp.listenMulticast(IPAddress(239, 255, 255, 250), 1901)) {
          udp.onPacket([](AsyncUDPPacket packet) {
          char pB[40];
          String mac; 
          //Serial.print(&packet.data()[60]);
          if(packet.data()[60]=='A'){  
          //SERVER: Arduino/1.0 UPNP/1.1 ___temperature/ESP.getChipId()
            for(int i=0; i<40; i++)
              pB[i] = packet.data()[i+81];
            //Serial.println(pB);
            if(memcmp(pB,"___",3)==0){
              if(pB[3]=='t'){            //___temperature
                int i=0;
                while(pB[i+15]!='\n'){
                  mac += (char)pB[i+15];
                  i++;
                }
                uint64_t id;
                if(mac.length()>8) id = mac.toInt();
                else id = StrTou48(mac);
                if(pB[4]=='e') ssdpLists(id, packet.remoteIP().toString(), F("esp8266_thermostat"),0);
                else ssdpLists(id, packet.remoteIP().toString(), F("esp8266_thermostat_plus"),0);
                //time_ssdp = -120000;
                h4.once(10,[](){sendSSDPWs(webSocketConnect);});
                req = true;
              }
              else if(pB[3]=='a'){            //___air
                int i=0;
                while(pB[i+7]!='\n'){
                  mac += (char)pB[i+7];
                  i++;
                }
                uint64_t id;
                if(mac.length()>8) id = mac.toInt();
                else id = StrTou48(mac);
                ssdpLists(id, packet.remoteIP().toString(), F("esp8266_air"),0);
                //time_ssdp = -120000;
                h4.once(10,[](){sendSSDPWs(webSocketConnect);});
                req = true;
              }
              else if(pB[3]=='p'){            //___panelxx
                int i=0;
                while(pB[i+11]!='\n'){
                  mac += (char)pB[i+11];
                  i++;
                }
                uint64_t id = StrTou48(mac);
                ssdpLists(id, packet.remoteIP().toString(), F("esp32_panel_4inch"),(pB[8]-0x30)*10+(pB[9]-0x30));
                //time_ssdp = -120000;
                req = true;
              }
              else if(pB[3]=='L'){            //___Light
                int i=0;
                while(pB[i+9]!='\n'){
                  mac += (char)pB[i+9];
                  i++;
                }
                uint64_t id;
                if(mac.length()>8) id = mac.toInt();
                else id = StrTou48(mac);
                ssdpLists(id, packet.remoteIP().toString(), F("esp32_panel_4inch"),20);
                //time_ssdp = -120000;
                h4.once(10,[](){sendSSDPWs(webSocketConnect);});
                req = true;
              }
            }
          }
        });  
  }

  server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest * request) {
    String ssdpSend = "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">";
    String  ssdpHeder = xmlNode(F("major"), "1");
    ssdpHeder += xmlNode(F("minor"), "0");
    ssdpHeder = xmlNode(F("specVersion"), ssdpHeder);
    ssdpHeder += xmlNode(F("URLBase"), "http://"+WiFi.localIP().toString());
    String  ssdpDescription = xmlNode(F("deviceType"), "upnp:rootdevice");
    ssdpDescription += xmlNode(F("friendlyName"), jsonRead(configSetup, "name"));
    ssdpDescription += xmlNode(F("presentationURL"), F("/"));
    ssdpDescription += xmlNode(F("serialNumber"), String(ChipId()));
    ssdpDescription += xmlNode(F("Name"), F("SSDP"));
    ssdpDescription += xmlNode(F("modelName"),"___temperature_"+ jsonRead(configSetup, "name"));
    //ssdpDescription += xmlNode(F("modelName"),"___temperature/"+ String(ESP.getChipId()));
    ssdpDescription += xmlNode(F("modelNumber"), ver);
    ssdpDescription += xmlNode(F("modelURL"), F("http://lytko.com/"));
    ssdpDescription += xmlNode(F("manufacturer"), F("LYTKO"));
    ssdpDescription += xmlNode(F("manufacturerURL"), F("http://lytko.com"));
    ssdpDescription += xmlNode(F("UDN"), "uuid:38323636-4558-4dda-9188-cda0e6"+decToHex(ChipId(), 6));
    ssdpDescription = xmlNode(F("device"), ssdpDescription);
    ssdpHeder += ssdpDescription;
    ssdpSend += ssdpHeder;
    ssdpSend += "</root>";
    request->send(200, "text/xml", ssdpSend);
  });
  //Если версия  2.0.0 закомментируйте следующую строчку
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.setSchemaURL("description.xml");
  SSDP.setModelName("___temperature");

  SSDP.setModelNumber(String(ChipId())+".");
  SSDP.begin();
//}
}


String xmlNode(String tags, String data) {
  String temp = "<" + tags + ">" + data + "</" + tags + ">";
  return temp;
}

String decToHex(uint32_t decValue, byte desiredStringLength) {
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;
  return hexString;
}

// ------------- SSDP запрос
void requestSSDP () {
//  if(!homekit){
    addressList = "{\"ssdp\":[]}";
    if (WiFi.status() == WL_CONNECTED){ 
      if(ext_sens.is) ssdpLists(ChipId(),  WiFi.localIP().toString(), F("esp8266_thermostat"), 1);
      else ssdpLists(ChipId(),  WiFi.localIP().toString(), F("esp8266_thermostat"), 0);
    }
    else ssdpLists(ChipId(),  WiFi.softAPIP().toString(), F("esp8266_thermostat"),0);
    //ssdpLists(WiFi.macAddress(),  WiFi.localIP().toString(), jsonRead(configSetup, "name"), "light", len);
    IPAddress ssdpAdress(239, 255, 255, 250);
    unsigned int ssdpPort = 1900;
    unsigned char  ReplyBuffer[]= "M-SEARCH * HTTP/1.1\r\nHost:239.255.255.250:1900\r\nST:upnp:rootdevice\r\nMan:\"ssdp:discover\"\r\nMX:3\r\n\r\n";
    udp.writeTo(ReplyBuffer,sizeof(ReplyBuffer),ssdpAdress,1900);
    h4.once(8000,[](){ if(ssdpSize!=addressList.length()){ssdpSize=addressList.length(); sendSSDPWs(webSocketConnect); }});
//  }
}

void ssdpLists(uint64_t chipIDremote, String remoteIP, String typ, byte children) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& list = jsonBuffer.parseObject(addressList);
  JsonArray& arrays = list["ssdp"].asArray();
  JsonObject& record = arrays.createNestedObject();
  record["id"]  = u48toStr(chipIDremote);
  //record["id"]  = String(chipIDremote);
  record["ip"]  = remoteIP;
  record["type"]  = typ; 
  if(typ=="esp32_panel_4inch"){
   switch((byte)children/10){
    case 0: record["type_1ch"]  = "none"; break;
    case 1: record["type_1ch"]  = "thermostat"; break;
    case 2: record["type_1ch"]  = "light"; break;
   }
   switch(children%10){
    case 0: record["type_2ch"]  = "none"; break;
    case 1: record["type_2ch"]  = "thermostat"; break;
    case 2: record["type_2ch"]  = "light"; break;
   }
  }
  addressList = "";
  list.printTo(addressList);
  //if(addressList.length() != len) sendSSDPWs(webSocketConnect);
}

String u48toStr(uint64_t input)
{
  String result = "";//
  uint8_t base = 16; //hex 10 dec
  do {
    char c = input % base; input /= base;
    if (c < 10) c += '0';
    else c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

uint64_t StrTou48(String str)
{ 
  int l = str.length();
  uint64_t result = 0; 
  for (int i = 0; i<l-1; i++)
    result = (result<<4) + (str[i] <= '9' ? str[i] - '0' : str[i] - 'A' + 10); 
  return result; 
} 
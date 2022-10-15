unsigned long intervalSpa = 0;

void spaInit() {
  String temp = jsonRead(configSetup, "sensor_corr");
  sensor_correction = temp.toFloat();
  spaWork = jsonReadtoInt(spasetup, "spawork");
  presetMax = jsonReadtoInt(configSetup, "max_temp");
  presetMin = jsonReadtoInt(configSetup, "min_temp");
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  hysteresis = jsonRead(configSetup, "hysteresis").toFloat();
}

void spaRelay(bool st)  {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, st);


  rel = st;
}

int spaRelayOnOff()  {
  return rel;
}

bool spaOnOff() {
  if (spaWork) {
    return spaTemp;
  } else {
    return false;
  }
}

void spaWorks() {
  if (spaOnOff()) {
    spaRelay(true);
  } else {
    spaRelay(false);
  }
}

void changeSpa(bool work) {
  spaWork = work;
  spasetup = jsonWrite(spasetup, "spawork", work);
  saveFile("spa.json", spasetup);
  spaWorks();
  sendDataWs(1, 0);
}

void GetSchedule() {
  String temp = jsonRead(ScheduleSetup, "schedule");
  thermo = temp.toFloat();
  cycle5s();
  sendDataWs(1, webSocketConnect);
}

void ChangeSchedule(byte up, float v) {
  if (!changed) {
    changed = true;

    //расписание выключено, все значения одинаковые
    if (up == 1) {
      if ((thermo + v) <= presetMax) thermo += v;
      else if (thermo != presetMax) thermo = presetMax;
      else return;
    }
    else if (up == 255) {
      if ((thermo - v) >= presetMin) thermo -= v;
      else if (thermo != presetMin) thermo = presetMin;
      else return;
    }
    else if (up == 0) {
      if (v >= presetMin && v <= presetMax) {
        thermo = v;
      }
      else return;
    }

    ScheduleSetup = jsonWrite(ScheduleSetup, "schedule", String(thermo, 1));
    saveFile("schedule.json", ScheduleSetup);

    target_temp = thermo;
    sendDataWs(1, 0);
  }
}

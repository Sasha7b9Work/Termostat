byte i2cScanner()
{
  Serial.println ();
  Serial.println ("Scanning ...");
  byte count = 0;

  //Wire.begin(14, 12);
  Wire.begin(12, 14);
  Wire.setClock(100000UL);
  for (byte i = 60; i < 70; i++)
  {
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission () == 0)  // Receive 0 = success (ACK response) 
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);     // PCF8574 7 bit address
      Serial.println (")");
      count++;
    }
  }
  Serial.print ("Found ");      
  Serial.print (count, DEC);        // numbers of devices
  Serial.println (" device(s).");
  return count;
}

void initHTU21(){
    Serial.print("htu");
    htu.begin();
    delay(30);
    Serial.print(htu.readCfgReg());
    if (htu.readCfgReg()>0){
      htuok = true;
      Serial.println(" find");
      int_sens.ds_htu = 1;
 /*     zigbee[numChild].u64ExtAddr = 0xffffffffffffffff;
      zigbee[numChild].u16NwkAddr = 0xffff;
      zigbee[numChild].u8Lqi = 255;
      zigbee[numChild].i = numChild;
      zigbee[numChild].val[0] = 100;
      zigbee[numChild].val[1] = htu.getTemperatureWait();
      zigbee[numChild].val[2] = htu.getHumidityWait();
      Serial.print("Temp:");   Serial.println(zigbee[numChild].val[1]);   
      Serial.print("Hum:");   Serial.println(zigbee[numChild].val[2]); 
      insertnewdev(u64toStr(0xffffffffffffffff), "Lytko", u16toStr(0xffff));
      numChild++; */
    }
}

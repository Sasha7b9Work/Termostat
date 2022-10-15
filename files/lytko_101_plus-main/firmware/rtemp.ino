
void tempInit() {
  PORT_OUT(15);
  PORT_OUT(0);
  int temp;
  int_sens.is = jsonReadtoInt(configSetup, "sensor_internal_use");
 
  temp = jsonReadtoInt(configSetup, "sensor_model_id");

  if(int_sens.is==1 && int_sens.ds_htu==0){
    //ds=2; 
    for(byte i = 0; i<8; i++)
      intDS[i] = jsonReadtoInt(int_ds_addr, String(i+1)); 
    intdallasInit();
  }
  if (temp!=0){
    //B=ln(r1/r2)/(1/t1-1/t2)
    ds=0; 
    PORT_HIGH(0);
    switch(temp){
      case 1: R0 = 3300; B = 4000; break;
      case 2: R0 = 5000; B = 4000; break;
      case 3: R0 = 6800; B = 3400; break;
      case 4: R0 = 10000; B = 3911; break;
      case 5: R0 = 12000; B = 3621; break;
      case 6: R0 = 14800; B = 3761; break;
      case 7: R0 = 15000; B = 3354; break;
      case 8: R0 = 20000; B = 4000; break;
      case 9: R0 = 33000; B = 4001; break;
      case 10: R0 = 47000; B = 3887; break;
      default: break;
    }
  }
  else { 
    ds=1; 
    PORT_LOW(0); 
    dallasInit();
  }
  
  //Serial.println(int_sens.is);
}

void dallasInit(){

  //oneWire.begin(ONE_WIRE_BUS);
  sensors.begin();
  if(sensors.getDeviceCount()==2) {
    sensors.getAddress(tempSensor, 0);
    if(tempSensor == intDS) sensors.getAddress(tempSensor, 1);
  }
  else sensors.getAddress(tempSensor, 0);

  sensors.setResolution(tempSensor, 12);
    sensors.requestTemperatures();
    if(ds==1) {
      temp_ = sensors.getTempC(tempSensor);
    }
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
}

void intdallasInit(){
  //oneWire.begin(ONE_WIRE_BUS);
  sensors.begin();
  //sensors.getAddress(intDS, 0);

  sensors.setResolution(intDS, 12);
  sensors.requestTemperatures();
  int_sens.temp = sensors.getTempC(intDS)-6;
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
}

float averageTemp() {
  float average;
  
    if(tempSensorHealth==0 && int_sens.is==0) {average = -127; spaRelay(false); dallasInit();}
    else 
    average = temp_ + sensor_correction;
    
  //if(homekit && average<0) average = 0.0;
  return average;
}

void tempWorks() {
  float last_t = temp_;
  if(int_sens.is==1) {
    if(int_sens.ds_htu==0)int_sens.temp = sensors.getTempC(intDS)-6;
    else {
      int_sens.temp = htu.getTemperature() - 7;
      htu.requestTemperature();
      h4.once(90,[](){htu.readTemperature();
                     htu.requestHumidity();
                     h4.once(30,[](){htu.readHumidity();});
      });
    }
  }
  if(int_sens.is==2) int_sens.temp = htu.getTemperature();
  if(ds==1) {
    temp_ = sensors.getTempC(tempSensor);
    if(int_sens.is==1 && int_sens.ds_htu==0 && int_sens.temp==(temp_-6)) temp_ = 0;
  }
  else if(R0>9000) temp_ = NTCgettemperature(R0, 2200, B, 25, 1);
  else temp_ = NTCgettemperature(R0, 1100, B, 25, 1);
  //Serial.println(current_temp[temp_index]);
  if (temp_ == -127) {temp_ = last_t; /*tempSensorHealth-= 30;*/}
  //if (temp_ < - 40) {temp_ = last_t; tempSensorHealth-= 30;}
  //else if (temp_ == 85) {temp_ = last_t; tempSensorHealth-= 30;} 
  //     else {tempSensorHealth = 100; } 
  //if (tempSensorHealth<0) tempSensorHealth = 0;
  last_t = 0.2 * temp_ + 0.8 * last_t;
  temp_ = last_t;
  if(ds==1 || (int_sens.is==1 && int_sens.ds_htu==0)) sensors.requestTemperatures();
} 
  
float NTCgettemperature(uint32_t nominalResistance, uint32_t seriesResistance, uint16_t betaCoefficient, uint8_t nominalTemperature, uint8_t samples){
    float average = 0;
    float temp = 0;
    for (uint8_t i = 0; i < samples; i++) {
        average += analogRead(A0);
    }
    average /= samples;
    
    // convert the value to resistance
    average = seriesResistance * ((3.3*1023 - 1) / average - 1);

    // Steinhart–Hart equation, based on https://learn.adafruit.com/thermistor/using-a-thermistor
    float steinhart = (log(average / nominalResistance)) / betaCoefficient;
    steinhart += 1.0 / (nominalTemperature + 273.15);
    steinhart = 1.0 / steinhart; // invert
    steinhart -= 273.15; // convert to celsius
    //temp = Filter_SMA(steinhart);
    return steinhart;  
}
// Spa temperature regulation

void spaRegulation() {
    float t;
    if(ext_sens.is) t = ext_sens.temp;
    else if(int_sens.is==1) t = int_sens.temp + sensor_correction;
    else t = averageTemp();
    
    float f = thermo - hysteresis;
    if (t>=thermo) spaTemp = false;
    else if (!spaTemp && t<f) spaTemp = true;
    if((ext_sens.is || int_sens.is==1) && temp_>28) spaTemp = false;
    //regulator.input = t;
    //Serial.print("3");
}

float getTempSpa() {
  return averageTemp();
}
/*
float middle_of_3(float a, float b, float c)
{
 float middle;
 if ((a <= b) && (a <= c)) middle = (b <= c) ? b : c;
 else if ((b <= a) && (b <= c)) middle = (a <= c) ? a : c;
      else middle = (a <= b) ? a : b;
 return middle;
}
*/
/*float Filter_SMA(float For_Filtered)
{
  Filter_Buffer[FILTER_SMA_ORDER - 1] = For_Filtered;
  float Output = 0;
  for(uint8_t i = 0; i < FILTER_SMA_ORDER; i++)
  {
    Output += Filter_Buffer[i];
  }
  Output /= FILTER_SMA_ORDER;
  for(uint8_t i = 0; i < FILTER_SMA_ORDER; i++)
        Filter_Buffer[i] = Filter_Buffer[i+1];
  return Output;
}*/
/*
void ftoa(float f, char *str, uint8_t precision) {
  uint8_t i, j, divisor = 1;
  int8_t log_f;
  int32_t int_digits = (int)f;             //store the integer digits
  float decimals;
  char s1[12];

  memset(str, 0, sizeof(s1));  
  memset(s1, 0, 10);

  if (f < 0) {                             //if a negative number 
    str[0] = '-';                          //start the char array with '-'
    f = abs(f);                            //store its positive absolute value
  }
  log_f = ceil(log10(f));                  //get number of digits before the decimal
  if (log_f > 0) {                         //log value > 0 indicates a number > 1
    if (log_f == precision) {              //if number of digits = significant figures
      f += 0.5;                            //add 0.5 to round up decimals >= 0.5
      itoa(f, s1, 10);                     //itoa converts the number to a char array
      strcat(str, s1);                     //add to the number string
    }
    else if ((log_f - precision) > 0) {    //if more integer digits than significant digits
      i = log_f - precision;               //count digits to discard
      divisor = 10;
      for (j = 0; j < i; j++) divisor *= 10;    //divisor isolates our desired integer digits 
      f /= divisor;                             //divide
      f += 0.5;                            //round when converting to int
      int_digits = (int)f;
      int_digits *= divisor;               //and multiply back to the adjusted value
      itoa(int_digits, s1, 10);
      strcat(str, s1);
    }
    else {                                 //if more precision specified than integer digits,
      itoa(int_digits, s1, 10);            //convert
      strcat(str, s1);                     //and append
    }
  }

  else {                                   //decimal fractions between 0 and 1: leading 0
    s1[0] = '0';
    strcat(str, s1);
  }

  if (log_f < precision) {                 //if precision exceeds number of integer digits,
    decimals = f - (int)f;                 //get decimal value as float
    strcat(str, ".");                      //append decimal point to char array

    i = precision - log_f;                 //number of decimals to read
    for (j = 0; j < i; j++) {              //for each,
      decimals *= 10;                      //multiply decimals by 10
      if (j == (i-1)) decimals += 0.5;     //and if it's the last, add 0.5 to round it
      itoa((int)decimals, s1, 10);         //convert as integer to character array
      strcat(str, s1);                     //append to string
      decimals -= (int)decimals;           //and remove, moving to the next
    }
  }
}
*/

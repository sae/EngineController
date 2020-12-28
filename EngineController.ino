//#include <MQTT.h>  //to next version
//#include <MQTTClient.h>

/**
 * diesel or any engine controller
 * use board LOLIN(WeMos V1 R2)
 * Flash size 4M(SPIFFS 2MB)
 * 
 * temperature sensor - dallas 1wire DS18B20
 * fuel level - ultrasonic distance sensor like a US-025A
 * optocoupler - ~any 
 * 
 * /t /v /l /r : single value
 * /all : all values, json {t, v, r, l} 
 * all values are native, make a calculations in javascript
 * (t= degree С, l - distance to fuel,cm. 
 * r - pulses per second (1/rotation if hall, 6/rotation if generator, 
 * v - pure A0 value (calibrate it, note about a diode 0.7v)
 * mhr - motorhours in minutes
 * 
 * (C)sae762@gmail.com
  */
#include <OneWire.h>
#define DEBUG
#include <myesp.h> //SPIFFS included here

//sensors pins
#define TEMP_S 14 //gpio14 - D5
#define RPM_S 13 //gpio13 - D7
#define VOLT_S A0 //A0
#define SONAR_TRIGGER_PIN     5 //D1
#define SONAR_ECHO_PIN        4 //D2

MyESP esp;
OneWire  ds(TEMP_S);  
byte dataT[12];
byte addrT[8];
  
#define timeU 1200 //measure voltage every 1.2 sec
unsigned long timerU; //timer
//#define timeR 1000 //measure rpm every 1 sec
unsigned long timerR; //timer
#define timeT 1800 //measure temperature 
unsigned long timerT; //timer
#define timeMHR 60000 //motor hours, 60sec?
unsigned long timerMHR;
#define fileMHR "motorhours" //filename
 
int t; //integer temperature, C
int u; //voltage, adc counter 0-1023
int r; //motor speed, RPM , pulses per minute, 6 pulses per cycle
int l; //fuel level, сm (unrelated to temperature)
int mhr=0; //motor hours, in minutes?
char s[10]; //temp buffer for integers
char js[100]; //json buffer

int i; //counter for
int pulse_count;//pulse counter for rpm

void setup() {
  esp.begin();
  esp.web->on("/t", []() {
      itoa(t,s,10);
      esp.webSend(200, "", s); 
  });
  esp.web->on("/u", []() {
      itoa(u,s,10);
      esp.webSend(200, "", s); 
  });
  esp.web->on("/r", []() {
      itoa(r,s,10);
      esp.webSend(200, "", s); 
  });
  esp.web->on("/l", []() {
      itoa(l,s,10);
      esp.webSend(200, "", s); 
  });
  esp.web->on("/mhr", []() {
      itoa(mhr,s,10);
      esp.webSend(200, "", s); 
  });
  esp.web->on("/all", []() { //all is busy
      //jsonAll(); executed every 1sec in rotation count
      esp.webSend(200, "", js); 
  });  
  pinMode(TEMP_S, INPUT_PULLUP);
  setupT();
  pinMode(RPM_S, INPUT_PULLUP);
  attachInterrupt(RPM_S, &rpm_isp, FALLING);
  pinMode(SONAR_TRIGGER_PIN, OUTPUT);
  pinMode(SONAR_ECHO_PIN, INPUT);
  if (SPIFFS.exists(fileMHR)) { 
    File file = SPIFFS.open(fileMHR, "r"); 
    mhr=file.parseInt();
    file.close();
  }
}

void updateMHR() { //save motorhours to file
    File file = SPIFFS.open(fileMHR, "w"); 
    file.print(mhr);
    file.close();  
}

void jsonAll() {
  sprintf(js,"{\"u\":%d, \"t\":%d, \"r\":%d, \"l\":%d, \"mhr\":%d }",u,t,r,l,mhr);
}

void loop() {
  esp.handle();
  digitalWrite(LED_BUILTIN, digitalRead(RPM_S)); //test sensor 
  
  if(millis()-timerU>timeU) {//every TimeU
    u=analogRead(A0);
    //12.07=367 //measured values
    //u=u*11.35/367+0.7; //0.7 diode
    timerU=millis();//timer shift
   }
  if(millis()-timerT>timeT) {//every TimeT
    readT();
    timerT=millis();//timer shift
    l=readSonar();
  }
  if(millis()-timerR>1000) {//every 1sec
    //calcR();
    r=pulse_count;//*60/6;// pulses/sec*60sec / 6 pulses per rotation
    pulse_count=0;
    timerR=millis();//timer shift
    jsonAll();
    Serial.println(js);
  }
  if(millis()-timerMHR>timeMHR) {//every TimeMHR (normally 60sec, 1 min)
    if (r>100) { //if has some RPM 
      mhr++;
      updateMHR();
    }
  }
  delay(1); //ms
  }

/** init dallas ds18b20 sensor */
 void setupT() {   
  if ( !ds.search(addrT)) t=-404;
  if (OneWire::crc8(addrT, 7) != addrT[7]) t=-501;
  ds.reset();
  ds.select(addrT);
  ds.write(0x44, 0); 
 }

/** read temperature */
void readT() {
  if (t < -200) return; //sensor error
  ds.reset();
  ds.select(addrT);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    dataT[i] = 0;
    dataT[i] = ds.read();
  }
  ds.reset();
  ds.skip();
  ds.write(0x44, 0); //start next

  int16_t raw = (dataT[1] << 8) | dataT[0];
  byte cfg = (dataT[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time

  //float ft = (float)raw / 16.0;
  //dtostrf(ft, 1, 1, ts);
  t=raw/16;
}

ICACHE_RAM_ATTR void rpm_isp() { //interrupt handle
  pulse_count++;
}

//copypasted
unsigned int readSonar()
{
   // Trigger the SRF05:
   digitalWrite(SONAR_TRIGGER_PIN, HIGH);
   delayMicroseconds(10);
   digitalWrite(SONAR_TRIGGER_PIN, LOW);

   // Wait for Echo Pulse
   unsigned long pulse_length = pulseIn(SONAR_ECHO_PIN, HIGH);

   // Ensure the ultrasonic "beep" has faded away
   //delay(50);
   // pulse_length/58 = cm or pulse_length/148 = inches
   //return( (unsigned int) (pulse_length ) );
   return( (unsigned int) pulse_length / 58  );
}

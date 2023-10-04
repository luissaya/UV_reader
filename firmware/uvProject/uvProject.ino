#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SCK  18
#define MISO 19
#define MOSI 23
#define CS    5
#define UVOUT 2 //Output from the sensor
#define REF  15 //3.3V power on the ESP32 board
#define MODE 35 //Output from the operation mode

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define Sleep_Interval 10000 //10 seconds
#define time_HI 30
#define time_LO 60
#define led 2
/* create a hardware timer */
hw_timer_t * timer = NULL;
unsigned long previousMillis = 0; 

String nameFILE = "/uvData.csv"; // .csv file
float uvLevel = 0.0;
float refLevel = 0.0;
bool existName = false;
bool debug = false;
int TIME_TO_SLEEP = time_LO;        /* Time ESP32 will go to sleep (in seconds) */
int modeValue = 0;
RTC_DATA_ATTR int bootCount = 0;


void averageRead(int numberReadings);
float voltageToIntensity(float voltage);
void writeFile(fs::FS &fs, String name, String message);
void addData(fs::FS &fs, String name, float message);
void createFileTitle(fs::FS &fs, String name, String message);

SPIClass sdspi(VSPI);

volatile byte state = LOW;

void IRAM_ATTR onTimer(){
  state = !state;
  digitalWrite(led, state);
}

void setup(){
  if(debug) Serial.begin(115200);
  if(debug) delay(2000);    
  if(debug) Serial.println("Initializing");  
  pinMode(UVOUT, INPUT);
  pinMode(REF, INPUT);  
  pinMode(MODE, INPUT);
  pinMode(led, OUTPUT);

  modeValue = digitalRead(MODE);
  delay(50);
  if(modeValue == HIGH){
    TIME_TO_SLEEP = time_HI; // HIGH or LOW,  less time if it is activated
  }

  /* Use 1st timer of 4 */
  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  timer = timerBegin(0, 80, true);
  /* Attach onTimer function to our timer */
  timerAttachInterrupt(timer, &onTimer, true);
  /* Set alarm to call onTimer function every second 1 tick is 1us
  => 1 second is 1000000us */
  /* Repeat the alarm (third parameter) */
  timerAlarmWrite(timer, 0.2*uS_TO_S_FACTOR, true);
  timerAlarmEnable(timer);

  //boot count
  ++bootCount;
  if(debug) Serial.println("Boot number: " + String(bootCount));

  sdspi.begin(SCK, MISO, MOSI, CS);

  while(!SD.begin(CS,sdspi)){
    if(debug) Serial.println("Card Mount Failed");
    unsigned long currentMillis = millis();
    Serial.println(currentMillis);
    if (currentMillis >= Sleep_Interval) {
      if(debug) Serial.println("Setup ESP32 to sleep for every indefinitely");
      //esp_sleep_enable_timer_wakeup(120*uS_TO_S_FACTOR);
      esp_sleep_enable_touchpad_wakeup();
      esp_deep_sleep_start();
    }
  }
  if(debug) Serial.println("Card Mount successfully");
  uint8_t cardType = SD.cardType();

  while(cardType == CARD_NONE){
    if(debug) Serial.println("No SD card attached");
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= Sleep_Interval) {
      if(debug) Serial.println("Setup ESP32 to sleep for every indefinitely");
      //esp_sleep_enable_timer_wakeup(120*uS_TO_S_FACTOR);
      esp_sleep_enable_touchpad_wakeup();
      esp_deep_sleep_start();
    }
    delay(500);
  }
  if(debug) Serial.println("SD card attached successfully");

  // if(!SD.begin(CS,sdspi)){
  //   if(debug) Serial.println("Card Mount Failed");
  //   return;
  // }
  // uint8_t cardType = SD.cardType();
  // if(cardType == CARD_NONE){
  //   if(debug) Serial.println("No SD card attached");
  //   return;
  // }
  if(bootCount == 1){
    createFileTitle(SD,nameFILE, "Time, UV Intensity (mW/cm^2)\n");
    delay(1000);
  }
  timerAlarmDisable(timer);
  digitalWrite(led, LOW);

  float uvIntensity = voltageToIntensity(averageVoltage(20)); //Convert the voltage to a UV intensity level
  String timepass = timeElapsed(bootCount-1);
  if(debug) Serial.println(timepass);
  addData(SD, nameFILE, timepass + ", " + String(uvIntensity));
  //go to sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  if(debug) Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +" Seconds");
  if(debug) Serial.print("ESP is going to sleep");
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop(){
  //This is not going to be called
}

float averageVoltage(int numberReadings){ 
  for(int x = 0 ; x < numberReadings ; x++)
  {
    uvLevel += analogRead(UVOUT);
    refLevel += analogRead(REF);
  }
  uvLevel /= (float)numberReadings;
  refLevel /= (float)numberReadings;
  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  return (3.3 / refLevel * uvLevel);
}

float voltageToIntensity(float voltage){
  float in_min = 0.99;
  float in_max = 2.8;
  float out_min = 0.0;
  float out_max = 15.0;
  if (voltage <= in_min){
    return out_min;
  }
  return (voltage - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void addData(fs::FS &fs, String name, String message){
 if(debug) Serial.print("Appending to file: "+ name);

  File file = fs.open(name, FILE_APPEND);
  if(!file){
    if(debug) Serial.println(" Failed to open file for appending");
    return;
  }
  if(file.println(message)){
     if(debug) Serial.println(" Message appended");
  } else {
    if(debug) Serial.println(" Append failed");
  }
  file.close();
}

void createFileTitle(fs::FS &fs, String name, String message){
 if(debug) Serial.print("Appending to file: "+ name);

  File file = fs.open(name, FILE_APPEND);
  if(!file){
    if(debug) Serial.println(" Failed to open file for appending");
    return;
  }
  if(file.print(message)){
    if(debug) Serial.println(" Title appended");
  } else {
    if(debug) Serial.println(" Append failed");
  }
  file.close();
}

String timeElapsed(int boot){
  String timeDHM;
  int timeD = boot/3600;
  int timeH = (boot - timeD*3600)/60 ;
  int timeM = boot - timeD*3600 - timeH*60;
  String sTimeD;
  String sTimeH;
  String sTimeM;
  if (timeD<10){
      sTimeD = "0"+String(timeH);
    }else {
      sTimeD = String(timeH);
    }
   if (timeH<10){
      sTimeH = "0"+String(timeH);
    }else {
      sTimeH = String(timeH);
    }
   if (timeM<10){
      sTimeM = "0"+String(timeM);
    }else {
      sTimeM = String(timeM);
    }
  timeDHM = sTimeD+"D:"+sTimeH+"H:"+sTimeM+"M";
  return timeDHM;
}

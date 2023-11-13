#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include "RTClib.h"
/*
ESP32 
- I2C: SDA 21 / SCL 22
- SPI: CS 5 / MOSI 23 / MISO 19 / CLK 18
- LED 15
- ADC 34
*/
/*
ESP12E 
- I2C: SDA
- SPI: CS 15
- LED 4
- ADC A0
*/
#define CS  D8 //GPIO15 D8 
#define LED D4 //GPIO2 D4
#define UV  A0 // A0
#define sleep_waiting 10e3 //10 seconds
#define sleep_time 60e6 // time for each sleep

bool debug = false; // true to activate debug prints
String title = "uvData.csv";
String fileHeader = "Fecha(DD/MM/AAAA), Hora(HH:MM:SS), UV Intensity(mW/cm^2)";
String message = "";
float uvIntensity = 0.0;
float RES = 1023.0;//ADC resolution 1023
float REF = 3.684;//variable obtained through calibration
unsigned long previousMillis = 0; 

File myFile;
Ticker blinker;
RTC_DS1307 rtc;

void changeState();
float avgVoltage(int nReadings);
float voltToIntensity(float voltage);
void writingSD(String message, String file);
void readingSD(String file);
void printDirectory();
String currentTime();
String addZero(int number);

void setup() {
  // Open serial communications and wait for port to open:
  if(debug) Serial.begin(9600);
  pinMode(LED,OUTPUT);
  pinMode(UV, INPUT);
  if(debug) delay(1000);
  if(debug) Serial.println("Initializing");
  blinker.attach(0.2, changeState); //Initialize Ticker every 0.5s (Use attach_ms for time in ms)
  
  while (! rtc.begin()) {
    if(debug) Serial.println("Couldn't find RTC");
    if(debug) Serial.flush();
    // ADD SLEEP
    unsigned long currentMillis = millis();
    if (currentMillis >= sleep_waiting) {
      if(debug) Serial.println("Setup to sleep for every indefinitely");
      if(debug) Serial.flush();
      ESP.deepSleep(0); 
    }
  }
  if (!rtc.isrunning()) {
    if(debug) Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
/////////////////////////////////////////////////////
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//set the time with the compiled one
////////////////////////////////////////////////////
// rtc.adjust(DateTime(2023, 4, 9, 11, 47, 0));
  
  if(debug) Serial.print("Initializing SD card...");
  while (!SD.begin(CS)) {
    if(debug) Serial.println("initialization failed!");
    // ADD SLEEP
    unsigned long currentMillis = millis();
    if (currentMillis >= sleep_waiting) {
      if(debug) Serial.println("Setup to sleep for every indefinitely");
      if(debug) Serial.flush();
      ESP.deepSleep(0);
    }
    delay(1000);
  }
  if(debug) Serial.println("initialization done.");
  
  blinker.detach(); 
  digitalWrite(LED,HIGH);

  if (!SD.exists(title)) {
    if(debug) Serial.println(title + " doesn't exist");
    if(debug) Serial.println("File Header : " + fileHeader);
    writingSD(fileHeader, title);
  }
  if(debug) Serial.println(title + " exists");
  //Convert the voltage to a UV intensity level
  uvIntensity = voltToIntensity(avgVoltage(20));
  message = currentTime() +", "+ String(uvIntensity);
  writingSD(message, title);
  // if(debug) readingSD(title);
  if(debug) Serial.println(message);

  if(debug) Serial.println("I'm awake, but I'm going into deep sleep mode for 60 seconds");
  if(debug) Serial.flush();
  ESP.deepSleep(sleep_time);
}

void loop() {
  
}

void changeState(){
  digitalWrite(LED, !(digitalRead(LED)));  //Invert Current State of LED  
}

float avgVoltage(int nReadings){ 
  int uvLevel = 0;
  for(int x = 0 ; x < nReadings ; x++)
  {
    uvLevel += analogRead(UV);
  }
  uvLevel /= (float)nReadings;
  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  return (REF / RES * uvLevel);
}

float voltToIntensity(float voltage){
  float in_min = 0.99;
  float in_max = 2.8;
  float out_min = 0.0;
  float out_max = 15.0;
  if (voltage <= in_min){
    return out_min;
  }
  return (voltage - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void writingSD(String message, String file){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(file, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    if(debug) Serial.println("Writing to : " + file);
    myFile.println(message);
    // close the file:
    myFile.close();
    if(debug) Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    if(debug) Serial.println("error opening : " + file);
  }
}

void readingSD(String file){
  // open the file for reading:
  myFile = SD.open(file);
  if (myFile) {
    Serial.println(file);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening : " + file);
  }
}

String currentTime(){
  DateTime now = rtc.now();
  String ddmmyy = String(now.day()) +"/"+ String(now.month()) +"/"+ String(now.year());

  int hour = int(now.hour());
  int minute = int(now.minute());
  int second = int(now.second());
  String hhmmss = addZero(hour) +":"+ addZero(minute) +":"+ addZero(second);
  return ddmmyy + ", " + hhmmss;
}

String addZero(int number){
  if(number<10){
    return "0" + String(number);
  }
  return String(number);
}

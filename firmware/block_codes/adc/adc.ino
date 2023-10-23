#include <Ticker.h>
#define LED 2//D4
#define UV  A0//A0
Ticker blinker;

float avgVoltage(int nReadings);
float voltToIntensity(float voltage);
bool debug = true;
float uvIntensity = 0.0;
float RES = 1023.0;//1023.0;//ADC resolution
float REF = 3.65;//variable obtained through calibration
void setup() {
  // Open serial communications and wait for port to open:
  if(debug) Serial.begin(115200);
  
  pinMode(LED,OUTPUT);
  pinMode(UV, INPUT);
  
  blinker.attach(0.2, changeState); //Initialize Ticker every 0.5s (Use attach_ms for time in ms)
}
void loop(){
  Serial.println("Average voltage: "+ String(avgVoltage(20)));
  uvIntensity = voltToIntensity(avgVoltage(20)); //Convert the voltage to a UV intensity level
  Serial.println("UV Intensity: "+ String(uvIntensity));
  delay(1000);
}


float avgVoltage(int nReadings){ 
  int uvLevel = 0;
  for(int x = 0 ; x < nReadings ; x++)
  {
    uvLevel += analogRead(UV);
  }
  uvLevel /= (float)nReadings;
  Serial.println("ADC value: "+String(uvLevel));
  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  float voltage = REF/RES*uvLevel;
  Serial.println("Voltage: "+String(voltage));
  return voltage;
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
void changeState(){
  digitalWrite(LED, !(digitalRead(LED)));  //Invert Current State of LED  
}

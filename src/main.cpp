#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <SdFat.h>

#define I2C_ADDRESS 0x27
#define SPI_SPEED SD_SCK_MHZ(4)
#define CS_PIN 10

SdFat sd;

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(I2C_ADDRESS, 16, 2);
RTC_DS1307 rtc;

float currentTemperature = 25; // Početna temperatura
unsigned long long int time = 0;
unsigned long temptimer = 0; 

float maxtemp = 0;
float mintemp = 2147483647;


float generateTemperature(float currentTemp) {
    // Generiraj promjenu temperature između -2°C i +2°C
    float temperatureChange = random(-2, 3); // Generira broj između -2 i 2
    // Ažuriraj trenutnu temperaturu
    currentTemp += temperatureChange;
    // Ograniči temperaturu unutar raspona 20°C - 30°C
    currentTemp = constrain(currentTemp, 20.0, 30.0);
    return currentTemp;
}

void outputlcd(float currentTemp, float maxt, float mint) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(currentTemperature);
    lcd.print(" C");

    
    lcd.setCursor(0, 1);
    lcd.print(maxt);
    lcd.print("C | ");
    lcd.print(mint);
    lcd.print("C  ");
}


void setup() {
  Serial.begin(9600);
  lcd.init();
  // SETUP RTC MODULE
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

}

void loop() {
  time = millis();
  DateTime now = rtc.now();

  if (time - temptimer >= 200) { //promjeni na neko normalno vreme
    currentTemperature = generateTemperature(currentTemperature);
    temptimer = time;
  }

  if (currentTemperature > maxtemp) {
      maxtemp = currentTemperature;
  } 
  else if (currentTemperature < mintemp) {
      mintemp = currentTemperature;
  } 

  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.println  (now.second(), DEC);
  
  // Ispiši temperaturu na LCD
  outputlcd(currentTemperature, maxtemp, mintemp);


}

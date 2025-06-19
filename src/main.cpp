#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>

#define I2C_ADDRESS 0x27
#define RCL_ADDRESS 0x68 

LiquidCrystal_AIP31068_I2C lcd = LiquidCrystal_AIP31068_I2C(I2C_ADDRESS, 16, 2);


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
}

void loop() {
  time = millis();

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



  // Ispiši temperaturu na LCD
  outputlcd(currentTemperature, maxtemp, mintemp);


}


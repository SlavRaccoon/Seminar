#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Sd.h>
#include <SPI.h>


#define I2C_ADDRESS 0x27 //adresa LCD-a
#define SPI_SPEED SD_SCK_MHZ(4) 
#define CS_PIN 10 // Pin za CS (Chip Select) SD kartice

File direktorijat;
File myFile;

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(I2C_ADDRESS, 16, 2); // Inicijalizacija LCD-a
RTC_DS1307 rtc; // stvaranje RTC objekta

float currentTemperature = 25; // Početna temperatura
unsigned long long int time = 0; // Varijabla za vrijeme
unsigned long temptimer = 0; //Vrijeme za generiranje temperature
unsigned long SDtimer = 0; //Vrijeme za zapisivanje u SD karticu

float maxtemp = 0; // Maksimalna temperatura
float mintemp = 2147483647; // Minimalna temperatura

String serialbuffer = "";  // Buffer za serijski unos
char ulazniserial = ' '; // Ulazni serijski znak
String trentuniDir = "/"; // Trenutni direktorij
String Dirbuffer = ""; // Buffer za direktorij



float generateTemperature(float currentTemp) {
    // Generiraj promjenu temperature između -2°C i +2°C
    float temperatureChange = random(-2, 3); // Generira broj između -2 i 2
    // Ažuriraj trenutnu temperaturu
    currentTemp += temperatureChange;
    // Ograniči temperaturu unutar raspona 20°C - 30°C
    currentTemp = constrain(currentTemp, 20.0, 30.0);
    return currentTemp;
}

void outputlcd(float currentTemp, float maxt, float mint) {// Ispisivanje na LCD
    //prvi red
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(currentTemperature);
    lcd.print(" C");

    // drugi red
    lcd.setCursor(0, 1);
    lcd.print(maxt);
    lcd.print("C | ");
    lcd.print(mint);
    lcd.print("C  ");
}

void printDirectory(File dir) { // Funkcija za ispis direktorija
  while (true) {
    File baffer =  dir.openNextFile();
    
    if (! baffer) {
      break;
    }
    Serial.print(baffer.name());
    if (baffer.isDirectory()) {
      Serial.print("/");
    }
    Serial.println("");
    baffer.close();
  }
}

void outputSDfile(String dir, float tp, DateTime vrijeme) { // Funkcija za ispisivanje u SD karticu
  File radni; // Deklaracija varijable za radni file
  radni = SD.open(dir, FILE_WRITE);
  radni.print(vrijeme.year());
  radni.print(":");
  radni.print(vrijeme.month());
  radni.print(":");
  radni.print(vrijeme.day());
  radni.print(" ");
  radni.print(vrijeme.hour());
  radni.print(":");
  radni.print(vrijeme.minute());
  radni.print(":");
  radni.print(vrijeme.second());
  radni.print(" | ");
  radni.print("Temperatura: ");
  radni.print(tp);
  radni.println(" C");
  radni.close();
}

void setup() {
  Serial.begin(9600); // Inicijalizacija serijskog porta
  lcd.init(); // Inicijalizacija LCD-a

  if (! rtc.begin()) { // Inicijalizacija RTC-a
    Serial.flush();
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Postavljanje RTC-a na vrijeme kompajliranja


  if (!SD.begin(CS_PIN)) { // Inicijalizacija SD kartice
    Serial.println("Card initialization failed!");
    while (true);
  }

  if (!SD.exists("/templog")) // Stvaranje direktorija za logove ako ne postoji
  {
    SD.mkdir("/templog");
  }
  myFile = SD.open("templog/templog.txt", FILE_WRITE); //stvaranje datoteke za logove
  myFile.println("Datum i vrijeme | Temperatura");
  myFile.close();
  
}

void loop() {
  time = millis(); // Dohvaćanje trenutnog vremena u milisekundama
  DateTime now = rtc.now(); // Dohvaćanje trenutnog vremena s RTC-a

  if (time - temptimer >= 2000) { // Generiranje nove temperature
    currentTemperature = generateTemperature(currentTemperature);
    temptimer = time;
  }

  if (currentTemperature > maxtemp) { // Provjera maksimalne temperature
      maxtemp = currentTemperature;
  } 
  else if (currentTemperature < mintemp) { // Provjera minimalne temperature
      mintemp = currentTemperature;
  } 
  
  outputlcd(currentTemperature, maxtemp, mintemp); // Ispisivanje trenutne temperature na LCD

  if (Serial.available() > 0) { //Citanje serijskog porta i komande
    while (Serial.available() > 0) {
      ulazniserial = Serial.read();

      if (ulazniserial == '\n' || ulazniserial == '\r') {

        Serial.print("Uneseno: ");
        Serial.println(serialbuffer);

        if (serialbuffer[0] == 'L' && serialbuffer[1] == 'S'){ // Listanje direktorija
          direktorijat = SD.open(trentuniDir);
          printDirectory(direktorijat);
          direktorijat.close();
        }
        
        if (serialbuffer[0] == 'C' && serialbuffer[1] == 'D'){ // Promjena direktorija
          Dirbuffer = serialbuffer.substring(3);

          if (Dirbuffer[0] == '.' && Dirbuffer[1] == '.') { //vraćanje u prijasnji direktorij
            for (int i = trentuniDir.length() - 2; i >= 0; i--) {
              if (trentuniDir[i] == '/' || i == 1) {
                trentuniDir = trentuniDir.substring(0, i);
                break;
              }
            }
          }

          else if (Dirbuffer[0] == '/') { //provjera jel apsolutno ili relativno
            if (SD.exists(Dirbuffer) || Dirbuffer == "/")
            {
              trentuniDir = Dirbuffer;
            }
            else
            {
              Serial.println("Nema taj direktorij.");
            }
          } 
          else 
          {
              if (SD.exists(trentuniDir + Dirbuffer)) {
                trentuniDir += Dirbuffer;
                if (!(Dirbuffer[Dirbuffer.length()-1] == '/'))
                {  
                  trentuniDir += "/";
                }
              } else if(! (Dirbuffer[0] == '.')) {
                Serial.println("Nema taj direktorij.");
              }
          }

          Serial.println(trentuniDir);
        }

        if (serialbuffer[0] == 'O' && serialbuffer[1] == 'P'){ // Otvaranje datoteke
          Dirbuffer = serialbuffer.substring(3);
            Serial.println(trentuniDir + Dirbuffer);
          if (SD.exists(trentuniDir + Dirbuffer)) {
            myFile = SD.open(trentuniDir + Dirbuffer, FILE_READ);
            while (myFile.available()) {
              Serial.write(myFile.read());
            }
            myFile.close();
          } else {
            Serial.println("Datoteka ne postoji.");
          }
        }

        if (serialbuffer[0] == 'T' && serialbuffer[1] == 'M' && serialbuffer[2] == 'P') { // Ispis trenutne temperature
          Serial.print("Trenutna temperatura: ");
          Serial.println(currentTemperature);
        }
        if (serialbuffer[0] == 'M' && serialbuffer[1] == 'A' && serialbuffer[2] == 'X') { //Ispisivanje maksimalne temperature
          Serial.print("Maksimalna temperatura: ");
          Serial.println(maxtemp);
        }
        if (serialbuffer[0] == 'M' && serialbuffer[1] == 'I' && serialbuffer[2] == 'N') { //Ispisivanje minimalne temperature
          Serial.print("Minimalna temperatura: ");
          Serial.println(mintemp);
        }
        serialbuffer = "";
        break;
      }
      serialbuffer += ulazniserial;
    }
  }


  if (now.second() == 0 && SDtimer + 5000 < time) // zapisivanje u SD karticu
  { 
    SDtimer = time;
    if (SD.exists("/templog/templog.txt")) {
      outputSDfile("/templog/templog.txt", currentTemperature, now);
    } else {
      Serial.println("Nema datoteke.");
    }
  }

}


#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Sd.h>
#include <SPI.h>



#define I2C_ADDRESS 0x27
#define SPI_SPEED SD_SCK_MHZ(4)
#define CS_PIN 10

File root;
File myFile;

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(I2C_ADDRESS, 16, 2);
RTC_DS1307 rtc;

float currentTemperature = 25; // Početna temperatura
unsigned long long int time = 0;
unsigned long temptimer = 0; 
unsigned long SDtimer = 0; // Promjeni na neko normalno vreme

float maxtemp = 0;
float mintemp = 2147483647;

String serialbuffer = ""; 
char ulazniserial = ' ';
String trentuniDir = "/";
String Dirbuffer = "";



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

void printDirectory(File dir) {
  while (true) {
    File baffer =  dir.openNextFile();
    Serial.println("radim i dalje");
    
    if (! baffer) {
      break;
      Serial.println("radim i dalje");
    }
    Serial.println("radim i dalje");
    Serial.print(baffer.name());
    if (baffer.isDirectory()) {
      Serial.println("/");
    }
    baffer.close();
  }
}

void outputSDfile(String dir, float tp, DateTime vrijeme) {
  File radni;
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
  Serial.begin(9600);
  lcd.init();

  if (! rtc.begin()) {
    Serial.flush();
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


  if (!SD.begin(CS_PIN)) {
    Serial.println("Card initialization failed!");
    while (true);
  }

  if (!SD.exists("/templog"))
  {
    SD.mkdir("/templog");
  }
  myFile = SD.open("templog/templog.txt", FILE_WRITE);
  myFile.println("Datum i vrijeme | Temperatura");
  myFile.close();
  
  Serial.println("Komande:");
  //Serial.println("LS - listanje direktorija");
  //Serial.println("CD <direktorij> - promjena direktorija");
  //Serial.println("OP <file> - otvaranje datoteke");
  //Serial.println("TMP - trenutna temperatura");
  //Serial.println("MAX - maksimalna temperatura");
  //Serial.println("MIN - minimalna temperatura");
  /*
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  myFile.close();
  myFile = SD.open("templog/test.txt", FILE_WRITE);
  myFile.close();
  /*
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  Serial.println("Files in the card:");
  root = SD.open("/");
  printDirectory(root);
  Serial.println("");
*/
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
  
    // Ispiši temperaturu na LCD
  outputlcd(currentTemperature, maxtemp, mintemp);

  if (Serial.available() > 0) { //Citanje serijskog porta i komande
    while (Serial.available() > 0) {
      ulazniserial = Serial.read();

      if (ulazniserial == '\n' || ulazniserial == '\r') {

        Serial.print("Uneseno: ");
        Serial.println(serialbuffer);

        if (serialbuffer[0] == 'L' && serialbuffer[1] == 'S'){ // Listanje direktorija
          root = SD.open(trentuniDir);
          printDirectory(root);
          root.close();
          Serial.println("LS radi");
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

          else if (Dirbuffer[0] == '/') { //provjera jel idemo od početka ili relativno
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

        if (serialbuffer[0] == 'O' && serialbuffer[1] == 'P'){ // Otvaranje file
          Dirbuffer = serialbuffer.substring(3);
            Serial.println(trentuniDir + Dirbuffer);
          if (SD.exists(trentuniDir + Dirbuffer)) {
            myFile = SD.open(trentuniDir + Dirbuffer, FILE_READ);
            while (myFile.available()) {
              Serial.write(myFile.read());
            }
            myFile.close();
          } else {
            Serial.println("File ne postoji.");
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


  if (now.second() == 0 && SDtimer + 5000 < time) // zapisivanje u SD karticu, stavi da ide svaku minutu ili tako nesto
  { 
    SDtimer = time;
    if (SD.exists("/templog/templog.txt")) {
      outputSDfile("/templog/templog.txt", currentTemperature, now);
    } else {
      Serial.println("Nema taj file.");
    }
  }

}


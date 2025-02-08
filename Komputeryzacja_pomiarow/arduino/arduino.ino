#include <SPI.h>
#include <SD.h>
#include <DS3231.h>
#include <Wire.h>
#include <ezButton.h>

// Button and interrupt defines
#define BUTTON_PIN 2 
ezButton button(BUTTON_PIN);

// SD card defines
#define chipSelect 10

// SD card variables
File myFile; 

// Clock variables
DS3231 myRTC;
bool century = false;
bool h12Flag;
bool pmFlag;
byte year;
byte month;
byte date;
byte dow;
byte hour;
byte minute;
byte second;

// Timer variables
unsigned long startMillis = 0;
unsigned long stopMillis = 0;
unsigned long lapMillis = 0;
unsigned long lastLapTime = 0;
unsigned long lapBuffer[10];
int lapCount = 0;
bool stoperRunning = false;

// Click detection variables
volatile int clickCount = 0;
unsigned long firstClickTime = 0;
String fileName;

//clock API
void inputDateFromSerial() {
  // Call this if you notice something coming in on
  // the serial port. The stuff coming in should be in
  // the order YYMMDDwHHMMSS, with an 'x' at the end.
  boolean isStrComplete = false;
  char inputChar;
  byte temp1, temp2;
  char inputStr[20];

  uint8_t currentPos = 0;
  while (!isStrComplete) {
    if (Serial.available()) {
      inputChar = Serial.read();
      inputStr[currentPos] = inputChar;
      currentPos += 1;

      // Check if string complete (end with "x")
      if (inputChar == 'x') {
        isStrComplete = true;
      }
    }
  }
  Serial.println(inputStr);

  // Find the end of char "x"
  int posX = -1;
  for (uint8_t i = 0; i < 20; i++) {
    if (inputStr[i] == 'x') {
      posX = i;
      break;
    }
  }

  // Consider 0 character in ASCII
  uint8_t zeroAscii = '0';

  // Read Year first
  temp1 = (byte)inputStr[posX - 13] - zeroAscii;
  temp2 = (byte)inputStr[posX - 12] - zeroAscii;
  year = temp1 * 10 + temp2;

  // now month
  temp1 = (byte)inputStr[posX - 11] - zeroAscii;
  temp2 = (byte)inputStr[posX - 10] - zeroAscii;
  month = temp1 * 10 + temp2;

  // now date
  temp1 = (byte)inputStr[posX - 9] - zeroAscii;
  temp2 = (byte)inputStr[posX - 8] - zeroAscii;
  date = temp1 * 10 + temp2;

  // now Day of Week
  dow = (byte)inputStr[posX - 7] - zeroAscii;

  // now Hour
  temp1 = (byte)inputStr[posX - 6] - zeroAscii;
  temp2 = (byte)inputStr[posX - 5] - zeroAscii;
  hour = temp1 * 10 + temp2;

  // now Minute
  temp1 = (byte)inputStr[posX - 4] - zeroAscii;
  temp2 = (byte)inputStr[posX - 3] - zeroAscii;
  minute = temp1 * 10 + temp2;

  // now Second
  temp1 = (byte)inputStr[posX - 2] - zeroAscii;
  temp2 = (byte)inputStr[posX - 1] - zeroAscii;
  second = temp1 * 10 + temp2;
}

// Helper functions
void logToSD(String filename, const char* data) {
  myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {
    myFile.println(data);
    myFile.close();
  } else {
    Serial.println("Error opening file.");
  }
}

String formatTimeName() {
  
    int day = myRTC.getDate();
    int hour = myRTC.getHour(h12Flag, pmFlag);
    int minute = myRTC.getMinute();
    int second = myRTC.getSecond();

    String formattedTime = String(day < 10 ? "0" : "") + String(day) + 
                           String(hour < 10 ? "0" : "") + String(hour) + 
                           String(minute < 10 ? "0" : "") + String(minute) + 
                           String(second < 10 ? "0" : "") + String(second);

    return formattedTime;
}

String formatTime() {
  
    int year = myRTC.getYear();    // Rok
    int month = myRTC.getMonth(century);  // Miesiąc
    int day = myRTC.getDate();     // Dzień
    int dow = myRTC.getDoW();      // Dzień tygodnia (0 = poniedziałek, 6 = niedziela)
    int hour = myRTC.getHour(h12Flag, pmFlag);    // Godzina
    int minute = myRTC.getMinute(); // Minuta
    int second = myRTC.getSecond(); // Sekunda

    // Przekształcenie roku na 2 ostatnie cyfry
    year = year % 100;  // Wykorzystanie ostatnich dwóch cyfr roku

    // Formatowanie daty i czasu w wymaganym formacie: YYMMDDwHHMMSS
    String formattedTime = String(year < 10 ? "0" : "") + String(year) + 
                           String(month < 10 ? "0" : "") + String(month) + 
                           String(day < 10 ? "0" : "") + String(day) + 
                           String(dow) + 
                           String(hour < 10 ? "0" : "") + String(hour) + 
                           String(minute < 10 ? "0" : "") + String(minute) + 
                           String(second < 10 ? "0" : "") + String(second);

    return formattedTime;
}

void setup() {
  button.setDebounceTime(100);
  
  if (Serial.available()) {
    inputDateFromSerial();
    myRTC.setClockMode(false);

    myRTC.setYear(year);
    myRTC.setMonth(month);
    myRTC.setDate(date);
    myRTC.setDoW(dow);
    myRTC.setHour(hour);
    myRTC.setMinute(minute);
    myRTC.setSecond(second);

    delay(50);
    Serial.flush();
  }

  // Serial initialization
  Serial.begin(9600);
  while (!Serial);

  // SD card initialization
  Serial.print("\nInitializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed.");
  } else {
    Serial.println("SD card initialized.");
  }

  // RTC initialization
  Wire.begin();
  Serial.println("Setup complete.");
}

void loop() {
  button.loop();

  if (Serial.available()) {
    inputDateFromSerial();
    myRTC.setClockMode(false);
    
    myRTC.setYear(year);
    myRTC.setMonth(month);
    myRTC.setDate(date);
    myRTC.setDoW(dow);
    myRTC.setHour(hour);
    myRTC.setMinute(minute);
    myRTC.setSecond(second);

    delay(50);
    Serial.flush();
  }

  if (millis() - firstClickTime > 1000) {
    clickCount = 0;
  }

  if (button.isPressed()) {
    if (!stoperRunning) {
      if (clickCount == 0) {
        firstClickTime = millis();
      }
      clickCount++;
      Serial.println(clickCount);

      if (clickCount == 3 && millis() - firstClickTime <= 1000) { 
        stoperRunning = true;
        Serial.println("Stoper started!");
        fileName = formatTimeName();
        Serial.println((fileName + ".txt"));
        logToSD((fileName + ".txt"), formatTime().c_str());
        clickCount = 0;
        button = ezButton(BUTTON_PIN);
        button.setDebounceTime(100);
      } else if (millis() - firstClickTime > 1000) {
        clickCount = 0;
        Serial.println("za wolno");
      }
    } else {
      if (clickCount == 0) {
        firstClickTime = millis();
      }
      clickCount++;
      Serial.println(clickCount);

      if (millis() - firstClickTime > 1000) {
        clickCount = 0;  
        Serial.println("za wolno");
      }

      if (clickCount == 3 && millis() - firstClickTime <= 1000) {
        stoperRunning = false;
        Serial.println("Stoper stopped!");
        logToSD((fileName + ".txt").c_str(), formatTime().c_str());
        clickCount = 0;
        delay(1005);
        button = ezButton(BUTTON_PIN);
        button.setDebounceTime(100);
      }
      else {
        Serial.println(formatTime().c_str());
        logToSD((fileName + ".txt").c_str(), formatTime().c_str());
      }
    }
  }
}

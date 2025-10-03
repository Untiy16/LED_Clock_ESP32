#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include "time.h"
#include <Preferences.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include "nvs_flash.h"


Preferences prefs;
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
WebServer server(80);


//wifi
String ssid = "";
String password = "";

//time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 2 * 60 * 60;
const int   daylightOffset_sec = 3600;
const char* timezone = "EET-2EEST,M3.5.0/3,M10.5.0/4"; //Europe/Kiev

byte NIGHT_START_HOUR = 23;
byte NIGHT_END_HOUR = 7;

//LED
#define DIGIT_LEDS 24 //number of leds per digit
#define DOTS_LEDS 2 //number of leds of dots
#define CURRENT_LIMIT 2000 //in milliamps


byte DAY_BRIGHTNESS = 90;
byte NIGHT_BRIGHTNESS = 1;
byte CURRENT_BRIGHTNESS = DAY_BRIGHTNESS;

byte DAY_SATUR = 255;
byte NIGHT_SATUR = 255;
byte CURRENT_SATUR = DAY_SATUR;

byte DAY_COLOR = HUE_GREEN;
byte NIGHT_COLOR = HUE_RED;
byte CURRENT_COLOR = DAY_COLOR;

#define DIGIT_1_PIN 16
#define DIGIT_2_PIN 17
#define DIGIT_3_PIN 18
#define DIGIT_4_PIN 19
#define DOTS_PIN 5

#define LDR_A_PIN 34
#define LDR_D_PIN 14

CRGB digit_1_leds[DIGIT_LEDS];
CRGB digit_2_leds[DIGIT_LEDS];
CRGB digit_3_leds[DIGIT_LEDS];
CRGB digit_4_leds[DIGIT_LEDS];
CRGB dots_leds[DOTS_LEDS];

char hourStr[3];    // "HH" + '\0'
char minuteStr[3];  // "MM" + '\0'
char dayStr[3];    // "DD" + '\0'
char monthStr[3];  // "MM" + '\0'

byte dotsState = 0;
int LDR_READS = 100; // number of readings

//modes
byte SHOW_DATE = 1;
byte SHOW_TEMPERATURE = 1;
byte SHOW_HUMIDITY = 1;
byte SHOW_PRESSURE = 1;

byte SHOW_TIME_SECONDS = 10;
byte SHOW_DATE_SECONDS = 5;
byte SHOW_TEMPERATURE_SECONDS = 5;
byte SHOW_HUMIDITY_SECONDS = 5;
byte SHOW_PRESSURE_SECONDS = 5;



/*
  HSV colors

  HUE_RED
  HUE_ORANGE
  HUE_YELLOW
  HUE_GREEN
  HUE_AQUA
  HUE_BLUE
  HUE_PURPLE
  HUE_PINK
*/


void setup() {
  Serial.begin(9600);
  while(!Serial){};
  delay(2000);
  //weather sensors
  aht.begin();
  bmp.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, /* Operating Mode. */
  Adafruit_BMP280::SAMPLING_X2, /* Temp. oversampling */
  Adafruit_BMP280::SAMPLING_X16, /* Pressure oversampling */
  Adafruit_BMP280::FILTER_X16, /* Filtering. */
  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  loadSettings();

  pinMode(LDR_A_PIN, INPUT);
  pinMode(LDR_D_PIN, INPUT);

  //prevent adding 1 hour when set time manually
  setenv("TZ", "UTC0", 1);
  tzset();

  //add all led strips
  FastLED.addLeds<WS2812, DIGIT_1_PIN, GRB>(digit_1_leds, DIGIT_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, DIGIT_2_PIN, GRB>(digit_2_leds, DIGIT_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, DIGIT_3_PIN, GRB>(digit_3_leds, DIGIT_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, DIGIT_4_PIN, GRB>(digit_4_leds, DIGIT_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, DOTS_PIN,    GRB>(dots_leds,     DOTS_LEDS).setCorrection(TypicalLEDStrip);
  
  FastLED.setDither(0);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(DAY_BRIGHTNESS);

  //fill all leds with green at full brightness
  fill_solid(digit_1_leds, DIGIT_LEDS, CRGB::Green);
  fill_solid(digit_2_leds, DIGIT_LEDS, CRGB::Green);
  fill_solid(digit_3_leds, DIGIT_LEDS, CRGB::Green);
  fill_solid(digit_4_leds, DIGIT_LEDS, CRGB::Green);
  fill_solid(dots_leds,    DOTS_LEDS,  CRGB::Green);
  FastLED.show();

  //wi-fi setup
  // Try to read stored credentials
  prefs.begin("wifiCreds", true);
  String savedSSID = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  prefs.end();
  if (savedSSID != "") {
    Serial.println("Connecting to saved Wi-Fi...");
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    int t = 0;
    while (WiFi.status() != WL_CONNECTED && t < 20) { // wait 10 seconds max
      delay(500);
      Serial.print(".");
      t++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected!");
      Serial.println(WiFi.localIP());

      server.on("/", handleRoot);
      server.on("/sensors", handleSensors);
      server.on("/set-settings", HTTP_POST, handleSetSettings);
      server.on("/set-time", HTTP_POST, handleSetTime);
      server.on("/reboot", handleReboot);
      server.on("/resetwifi", handleResesWifiCreds);
      server.begin();

      getTimeFromInternet();
      extractLocalTime();

      return; // skip AP mode
    }
  }

  // If no saved creds or failed to connect, start AP mode
  Serial.println("Starting AP for configuration...");
  WiFi.softAP("ESP32_Config"); // ESP32 AP
  Serial.println("AP started. Connect and configure at 192.168.4.1");

  server.on("/", handleWifiRoot);
  server.on("/save", HTTP_POST, handleWifiSave);
  server.on("/reboot", handleReboot);
  server.on("/resetwifi", handleResesWifiCreds);
  server.begin();
}

int ldrAnalog = 0;
int displayState = 0; // 0 = time, 1 = temperature, 2 = humidity
int lastPressure = 100;  // impossible initial value
float lastTemperature = -1000;  // impossible initial value
float lastHumidity = -1000;
const float TEMP_THRESHOLD = 0.2;  // only update if change >= 0.03
const float HUM_THRESHOLD  = 0.2;
const int LDR_THRESHOLD  = 300;


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    server.handleClient();
    return;
  }

  FastLED.clear();
  delay(1);
  ldrModule();
  // delay(50);

  server.handleClient();

  // Cycle through states
  EVERY_N_SECONDS(1) {   // check every second
    static int counter = 0;
    counter++;

    if (displayState == 0 && counter >= SHOW_TIME_SECONDS) {   // after 10s, go to date
      if (SHOW_DATE) {displayState = 1;}
      else if(SHOW_TEMPERATURE) {displayState = 2;}
      else if(SHOW_HUMIDITY) {displayState = 3;}
      else if(SHOW_PRESSURE) {displayState = 4;}
      counter = 0;
    } 
    else if (displayState == 1 && counter >= SHOW_DATE_SECONDS) { // after 3s, go to temperature
      if (SHOW_TEMPERATURE) {displayState = 2;}
      else if(SHOW_HUMIDITY) {displayState = 3;}
      else if(SHOW_PRESSURE) {displayState = 4;}
      else {displayState = 0;}
      counter = 0;
    } 
    else if (displayState == 2 && counter >= SHOW_TEMPERATURE_SECONDS) { // after 3s, back to humidity
      if (SHOW_HUMIDITY) {displayState = 3;}
      else if(SHOW_PRESSURE) {displayState = 4;}
      else {displayState = 0;}
      counter = 0;
    }
    else if (displayState == 3 && counter >= SHOW_HUMIDITY_SECONDS) { // after 3s, back to time
      if(SHOW_PRESSURE) {displayState = 4;}
      else {displayState = 0;}
      counter = 0;
    }
    else if (displayState == 4 && counter >= SHOW_PRESSURE_SECONDS) { // after 3s, back to time
      displayState = 0;
      counter = 0;
    }
  }

  extractLocalTime();
  // Render based on current state
  if (displayState == 0) {
    renderTime();
  } else if (displayState == 1) {
    renderDate();
  } else if (displayState == 2) {
    renderTemperatureAndHumidity(0);
  } else if (displayState == 3) {
    renderTemperatureAndHumidity(1);
  } else if (displayState == 4) {
    renderPressure();
  }


  // delay(50);
  FastLED.setBrightness(CURRENT_BRIGHTNESS);
  FastLED.show();

  EVERY_N_MINUTES(30) {
    updateTimeFromInternet();
  }
}

// 7-segment digit patterns (1 = on, 0 = off), segments A to G
const bool digitSegments[14][7] = {
  {1,1,1,1,1,1,0}, // 0
  {1,1,0,0,0,0,0}, // 1
  {0,1,1,0,1,1,1}, // 2
  {1,1,1,0,0,1,1}, // 3
  {1,1,0,1,0,0,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}, // 9
  {0,1,1,1,0,0,1}, // 10 - celsius degrees sign
  {1,1,0,1,1,0,1}, // 11 - H letter 
  {0,0,0,0,0,0,0}, // 12 - empty
  {0,1,1,1,1,0,1}  // 13 - P letter
};

// Maps each segment to 3 LEDs
const int segmentMap[7][2] = {
  {0,   1},  // Segment A
  {4,   5},  // B
  {8,   9},  // C
  {12, 13},  // D
  {16, 17},  // E
  {20, 21},  // F
  {22, 23}   // G
};

void renderDigit(CRGB* strip, int digit) {
  for (int seg = 0; seg < 7; seg++) {
    if (digitSegments[digit][seg]) {
      for (int i = 0; i < 2; i++) {
        int ledIndex = segmentMap[seg][i];
        strip[ledIndex] = CHSV(CURRENT_COLOR, CURRENT_SATUR, 255);
      }
    }
  }
}

void renderTime() {
  //blink dots
  EVERY_N_MILLISECONDS(500) {
    dotsState = !dotsState;
  }

  if (dotsState) {
    dots_leds[0] = dots_leds[1] = CHSV(CURRENT_COLOR, CURRENT_SATUR, 255);
  } else {
    dots_leds[0] = dots_leds[1] = CRGB::Black;
  }
  
  renderDigit(digit_1_leds, hourStr[0] - '0');
  renderDigit(digit_2_leds, hourStr[1] - '0');
  renderDigit(digit_3_leds, minuteStr[0] - '0');
  renderDigit(digit_4_leds, minuteStr[1] - '0');

  
}

void renderDate() {
  renderDigit(digit_1_leds, dayStr[0] - '0');
  renderDigit(digit_2_leds, dayStr[1] - '0');
  renderDigit(digit_3_leds, monthStr[0] - '0');
  renderDigit(digit_4_leds, monthStr[1] - '0');
}


void renderTemperatureAndHumidity(int mode) {
  EVERY_N_SECONDS(1) {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

    if (fabs(temp.temperature - lastTemperature) >= TEMP_THRESHOLD) {
      lastTemperature = temp.temperature;
    }
    if (fabs(humidity.relative_humidity - lastHumidity) >= HUM_THRESHOLD) {
      lastHumidity = humidity.relative_humidity;
    }
  }

  char buffer[10];
  dtostrf(mode == 0 ? lastTemperature : lastHumidity, 5, 2, buffer);
  renderDigit(digit_1_leds, buffer[0] == ' ' ? '12' : (buffer[0] - '0'));
  renderDigit(digit_2_leds, buffer[1] - '0');
  renderDigit(digit_3_leds, buffer[3] - '0');
  renderDigit(digit_4_leds, mode == 0 ? 10 : 11);
  dots_leds[0] = CHSV(CURRENT_COLOR, CURRENT_SATUR, 255);
}

void renderPressure() {
  EVERY_N_SECONDS(1) {
    lastPressure = (int)(bmp.readPressure() / 100 * 0.75);
  }

  char buffer[4];
  itoa(lastPressure, buffer, 10);
  renderDigit(digit_1_leds, buffer[0] - '0');
  renderDigit(digit_2_leds, buffer[1] - '0');
  renderDigit(digit_3_leds, buffer[2] - '0');
  renderDigit(digit_4_leds, 13);
}

void ldrModule() {
  EVERY_N_SECONDS(2) {
    long sum = 0;

    for (int i = 0; i < LDR_READS; i++) {
      sum += analogRead(LDR_A_PIN);
    }

    int avgRead = sum / LDR_READS;

    if (abs(avgRead - ldrAnalog) >= LDR_THRESHOLD) {
      ldrAnalog = avgRead;
    }
  }
}


void dd() {
    Serial.println(); // Конец строки
}

template <typename T, typename... Args>
void dd(T first, Args... args) {
    Serial.print(first); // Выводим первый аргумент
    Serial.print(" ");   // Добавляем пробел между аргументами
    dd(args...); // Рекурсивно вызываем функцию для оставшихся аргументов
}
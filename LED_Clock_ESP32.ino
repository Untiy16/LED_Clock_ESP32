#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include "time.h"
#include <Preferences.h>

Preferences prefs;

//wifi
const char* ssid = "";
const char* password = "";

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
byte NIGHT_COLOR = HUE_BLUE;
byte CURRENT_COLOR = DAY_COLOR;

#define DIGIT_1_PIN 16
#define DIGIT_2_PIN 17
#define DIGIT_3_PIN 18
#define DIGIT_4_PIN 19
#define DOTS_PIN 5

CRGB digit_1_leds[DIGIT_LEDS];
CRGB digit_2_leds[DIGIT_LEDS];
CRGB digit_3_leds[DIGIT_LEDS];
CRGB digit_4_leds[DIGIT_LEDS];
CRGB dots_leds[DOTS_LEDS];

char hourStr[3];    // "HH" + '\0'
char minuteStr[3];  // "MM" + '\0'

byte dotsState = 0;


WebServer server(80);


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
  loadSettings();

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
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, password);
  Serial.println(WiFi.localIP());

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/set-settings", HTTP_POST, handleSetSettings);
  server.on("/set-time", HTTP_POST, handleSetTime);
  server.begin();
  // Serial.println("Web server started.");

  getTimeFromInternet();
  extractLocalTime();
}

void loop() {
  // bluetoothTick();
  server.handleClient();
  FastLED.clear();

  renderTime();

  FastLED.show();

  EVERY_N_MINUTES(30) {
    updateTimeFromInternet();
  }
}

// 7-segment digit patterns (1 = on, 0 = off), segments A to G
const bool digitSegments[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {1,1,0,0,0,0,0}, // 1
  {0,1,1,0,1,1,1}, // 2
  {1,1,1,0,0,1,1}, // 3
  {1,1,0,1,0,0,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
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
  extractLocalTime();
  //blink dots
  EVERY_N_MILLISECONDS(500) {
    dotsState = !dotsState;
  }

  if (dotsState) {
    dots_leds[0] = dots_leds[1] = CHSV(CURRENT_COLOR, CURRENT_SATUR, 255);
  } else {
    dots_leds[0] = dots_leds[1] = CRGB::Black;
  }
  
  //digits
  renderDigit(digit_1_leds, hourStr[0] - '0');
  renderDigit(digit_2_leds, hourStr[1] - '0');
  renderDigit(digit_3_leds, minuteStr[0] - '0');
  renderDigit(digit_4_leds, minuteStr[1] - '0');

  FastLED.setBrightness(CURRENT_BRIGHTNESS);
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
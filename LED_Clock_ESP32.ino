#include <ElegantOTA.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include "time.h"
#include <Preferences.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include "nvs_flash.h"
#include <freertos/semphr.h>

#define DEBUG_MODE 0
#define ACCESS_POINT_NAME "ESP32_LED_Clock_AP"
const int LOG_SIZE = 4096;          // Розмір логу в байтах (можна збільшити)
char debugLog[LOG_SIZE] = {0};      // Фіксований масив у пам'яті
int logIndex = 0;                   // Поточна позиція запису

SemaphoreHandle_t logMutex = NULL;  // М'ютекс захисту пам'яті

//pins
#define DIGIT_1_PIN 16
#define DIGIT_2_PIN 17
#define DIGIT_3_PIN 18
#define DIGIT_4_PIN 19
#define DOTS_PIN 5

#define LDR_A_PIN 34
#define LDR_D_PIN 14
#define RESET_BTN_PIN 32

#define LIGHT_SENSOR_TYPE 2 // 1 -  Photoresistor Module, 2 - TSL2591

Preferences prefs;
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
volatile tsl2591Gain_t tsl2591CurrentGain = TSL2591_GAIN_HIGH;
WebServer server(80);


//wifi
String savedSSID = "";
String savedPass = "";

//time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 2 * 60 * 60;
const int   daylightOffset_sec = 3600;
const char* timezone = "EET-2EEST,M3.5.0/3,M10.5.0/4"; //Europe/Kiev

byte NIGHT_START_HOUR = 23;
byte NIGHT_END_HOUR = 7;

//LED
#define DIGIT_SEGMENTS 7 //number of segments per digit
#define LEDS_PER_DIGIT_SEGMENT 1 //number of leds per digit segment
#define EXTRA_LEDS 0 //number of unused leds (in first clock, strip was placed with extra leds unused between digit segments)
#define DIGIT_LEDS ((DIGIT_SEGMENTS * LEDS_PER_DIGIT_SEGMENT) + EXTRA_LEDS) //number of leds per digit
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



CRGB digit_1_leds[DIGIT_LEDS];
CRGB digit_2_leds[DIGIT_LEDS];
CRGB digit_3_leds[DIGIT_LEDS];
CRGB digit_4_leds[DIGIT_LEDS];
CRGB dots_leds[DOTS_LEDS];
byte USE_DITHER = 1;
byte DITHER_DELAY = 60;
byte DITHER_MAX_BRIGHTNESS = 2;

char hourStr[3];    // "HH" + '\0'
char minuteStr[3];  // "MM" + '\0'
char dayStr[3];    // "DD" + '\0'
char monthStr[3];  // "MM" + '\0'

byte dotsState = 0;

//effects
byte USE_RAINBOW = 0;
byte RAINBOW_SPEED = 20;

//sensors
byte USE_LDR = 1;
byte USE_LDR_DAY = 0;
byte USE_LDR_NIGHT = 1;
byte LDR_READS = 100; // number of readings
byte LDR_MIN_DAY_BRIGHTNESS = 1;
byte LDR_MIN_NIGHT_BRIGHTNESS = 1;

//display modes
byte SHOW_DATE_D = 1;
byte SHOW_TEMPERATURE_D = 1;
byte SHOW_HUMIDITY_D = 1;
byte SHOW_PRESSURE_D = 1;
byte SHOW_DATE_N = 1;
byte SHOW_TEMPERATURE_N = 1;
byte SHOW_HUMIDITY_N = 1;
byte SHOW_PRESSURE_N = 1;

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


bool isNight = false;
int ldrAnalog = 4095;
volatile float luxGlobal = 0;
volatile uint16_t visibleGlobal = 1000;
TaskHandle_t TslTaskHandle = NULL;
int displayState = 0; // 0 = time, 1 = temperature, 2 = humidity
int lastPressure = 100;  // impossible initial value
float lastTemperature = -1000;  // impossible initial value
float lastHumidity = -1000;
const float TEMP_THRESHOLD = 0.2;  // only update if change >= 0.03
const float HUM_THRESHOLD  = 0.2;
const int LDR_MAX = 4095;
const int LDR_MIN = 0;
const int LDR_THRESHOLD = 300;
const float LUX_MAX = 250;
const float LUX_MIN = 0;
const float LUX_THRESHOLD = 0.1;
const int VISIBLE_MAX_HIGH = 3000;
const int VISIBLE_MAX_LOW = 500;
const int VISIBLE_MIN = 0;
const int VISIBLE_THRESHOLD_HIGH = 10;
const int VISIBLE_THRESHOLD_LOW = 2;
byte rainbowHue = 0; 

void setup() {
  pinMode(LDR_A_PIN, INPUT);
  pinMode(LDR_D_PIN, INPUT);
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  
  if (DEBUG_MODE) {
    //seril init
    Serial.begin(9600);
    while(!Serial){};
    delay(2000);
    logMutex = xSemaphoreCreateMutex(); 
  }

  //add all led strips
  FastLED.addLeds<WS2812, DIGIT_1_PIN, GRB>(digit_1_leds, DIGIT_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, DIGIT_2_PIN, GRB>(digit_2_leds, DIGIT_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, DIGIT_3_PIN, GRB>(digit_3_leds, DIGIT_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, DIGIT_4_PIN, GRB>(digit_4_leds, DIGIT_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, DOTS_PIN,    GRB>(dots_leds,     DOTS_LEDS).setCorrection(TypicalLEDStrip);
  
  FastLED.setDither(USE_DITHER);//BINARY_DITHER or DISABLE_DITHER
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  if (LIGHT_SENSOR_TYPE == 1) {
    FastLED.setBrightness(map(analogRead(LDR_A_PIN), 0, 4095, 255, 1));
  } else {
    FastLED.setBrightness(10);
  }

  //fill all leds with green at full brightness
  fill_solid(digit_1_leds, DIGIT_LEDS, CRGB::Green);
  fill_solid(digit_2_leds, DIGIT_LEDS, CRGB::Green);
  fill_solid(digit_3_leds, DIGIT_LEDS, CRGB::Green);
  fill_solid(digit_4_leds, DIGIT_LEDS, CRGB::Green);
  fill_solid(dots_leds,    DOTS_LEDS,  CRGB::Green);
  FastLED.show();


  //weather sensors
  aht.begin();
  bmp.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, /* Operating Mode. */
  Adafruit_BMP280::SAMPLING_X2, /* Temp. oversampling */
  Adafruit_BMP280::SAMPLING_X16, /* Pressure oversampling */
  Adafruit_BMP280::FILTER_X16, /* Filtering. */
  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  loadSettings();

  //prevent adding 1 hour when set time manually
  setenv("TZ", "UTC0", 1);
  tzset();

  if (LIGHT_SENSOR_TYPE == 2) {
    // Setup gain (LOW, MED, HIGH, MAX)
    tsl.setGain(tsl2591CurrentGain);      
    // Setup integration time (100ms, 200ms, 300ms, 400ms, 500ms, 600ms)
    tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS); 

    // Creating task on CORE 0
      xTaskCreatePinnedToCore(
        tsl2591Worker,     // Функція з кодом датчика
        "TslTask",         // Назва задачі (для відладки)
        4096,              // Розмір виділеної пам'яті (стек у байтах)
        NULL,              // Параметри
        1,                 // Пріоритет
        &TslTaskHandle,    // Хендл задачі
        0                  // 📌 Індекс ядра: 0
      );
  }

  //wi-fi setup
  // Try to read stored credentials
  prefs.begin("wifiCreds", true);
  savedSSID = prefs.getString("ssid", "");
  savedPass = prefs.getString("pass", "");
  prefs.end();
  if (savedSSID != "") {
    Serial.println("Connecting to saved Wi-Fi...");
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    int t = 0;
    while (WiFi.status() != WL_CONNECTED && t < 120) { // wait 2 minutes max for router boot
      delay(1000);
      Serial.print(".");
      t++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected!");
      Serial.println(WiFi.localIP());

      serverBegin();

      getTimeFromInternet();
      extractLocalTime();

      return; // skip AP mode
    }
  }

  // If no saved creds or failed to connect, start AP mode
  Serial.println("Starting AP for configuration...");
  WiFi.softAP(ACCESS_POINT_NAME); // ESP32 AP
  Serial.println("AP started. Connect and configure at 192.168.4.1");

  // server.on("/", handleWifiRoot);
  serverBegin();
  ElegantOTA.setAutoReboot(true);
}

void loop() {
  ElegantOTA.loop();

  EVERY_N_MILLISECONDS_DYNAMIC(RAINBOW_SPEED) {
    if (rainbowHue == 255) {rainbowHue = 0;}
    rainbowHue++;
  }

  FastLED.clear();

  prefsResetButton();

  if (USE_DITHER && CURRENT_BRIGHTNESS <= DITHER_MAX_BRIGHTNESS) {
    FastLED.delay(DITHER_DELAY);
  } else {
    delay(10);
  }

  if (USE_LDR) {
    if (LIGHT_SENSOR_TYPE == 1) {
      ldrModule();
    } else if (LIGHT_SENSOR_TYPE == 2) {
      // tsl2591Module();
    }
  }

  server.handleClient();

  // Cycle through states
  EVERY_N_SECONDS(1) {   // check every second
    static int counter = 0;
    counter++;

    if (displayState == 0 && counter >= SHOW_TIME_SECONDS) {   // after 10s, go to date
      if (isNight ? SHOW_DATE_N : SHOW_DATE_D) {displayState = 1;}
      else if(isNight ? SHOW_TEMPERATURE_N : SHOW_TEMPERATURE_D) {displayState = 2;}
      else if(isNight ? SHOW_HUMIDITY_N : SHOW_HUMIDITY_D) {displayState = 3;}
      else if(isNight ? SHOW_PRESSURE_N : SHOW_PRESSURE_D) {displayState = 4;}
      counter = 0;
    } 
    else if (displayState == 1 && counter >= SHOW_DATE_SECONDS) { // after 3s, go to temperature
      if (isNight ? SHOW_TEMPERATURE_N : SHOW_TEMPERATURE_D) {displayState = 2;}
      else if(isNight ? SHOW_HUMIDITY_N : SHOW_HUMIDITY_D) {displayState = 3;}
      else if(isNight ? SHOW_PRESSURE_N : SHOW_PRESSURE_D) {displayState = 4;}
      else {displayState = 0;}
      counter = 0;
    } 
    else if (displayState == 2 && counter >= SHOW_TEMPERATURE_SECONDS) { // after 3s, back to humidity
      if (isNight ? SHOW_HUMIDITY_N : SHOW_HUMIDITY_D) {displayState = 3;}
      else if(isNight ? SHOW_PRESSURE_N : SHOW_PRESSURE_D) {displayState = 4;}
      else {displayState = 0;}
      counter = 0;
    }
    else if (displayState == 3 && counter >= SHOW_HUMIDITY_SECONDS) { // after 3s, back to time
      if(isNight ? SHOW_PRESSURE_N : SHOW_PRESSURE_D) {displayState = 4;}
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

  FastLED.setBrightness(CURRENT_BRIGHTNESS);
  FastLED.show();

  EVERY_N_MINUTES(30) {
    updateTimeFromInternet();
  }
}

// 7-segment digit patterns (1 = on, 0 = off), segments A to G
const bool digitSegments[16][7] = {
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
  {0,1,1,1,1,0,1},  // 13 - P letter
  {0,0,0,0,1,0,1},  // 14 - r letter
  {0,0,1,1,1,1,1}  // 15 - E letter
};

// Maps each segment to 3 LEDs
const int segmentMap[DIGIT_SEGMENTS][LEDS_PER_DIGIT_SEGMENT] = {
  {0},  // Segment A
  {1},  // B
  {2},  // C
  {3},  // D
  {4},  // E
  {5},  // F
  {6}   // G
};

void renderDigit(CRGB* strip, int digit) {
  for (int seg = 0; seg < DIGIT_SEGMENTS; seg++) {
    if (digitSegments[digit][seg]) {
      for (int i = 0; i < LEDS_PER_DIGIT_SEGMENT; i++) {
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

    if (avgRead == LDR_MAX || avgRead == LDR_MIN || abs(avgRead - ldrAnalog) >= LDR_THRESHOLD) {
      ldrAnalog = avgRead;
    }
  }
}

// void tsl2591Module() {
//   EVERY_N_SECONDS(2) {
//     uint32_t lum = tsl.getFullLuminosity();
//     uint16_t ir = lum >> 16;
//     uint16_t full = lum & 0xFFFF;
//     uint16_t visible = full - ir;
    
//     float lux = tsl.calculateLux(full, ir);
//     if (isnan(lux) || lux < 0.1) {
//       lux = 0;
//     }

//     if (lux == LUX_MAX || lux == LUX_MIN || abs(lux - luxGlobal) >= LUX_THRESHOLD) {
//       luxGlobal = lux;
//     }
    
//     if (visible <= 2) {
//       visible = 0;
//     }

//     if (
//       visible == VISIBLE_MAX
//       || visible == VISIBLE_MIN
//       || (visibleGlobal == 0 && visible > 0)
//       || (((visible > 1 && visible < 15) || (visibleGlobal > 1 && visibleGlobal < 15)) && abs(visible - visibleGlobal) >= VISIBLE_THRESHOLD_LOW)
//       || ((visible > 15 || visibleGlobal > 15) && abs(visible - visibleGlobal) >= VISIBLE_THRESHOLD_HIGH)
//     ) {
//       visibleGlobal = visible;
//     }
//   }
// }

void tsl2591Worker(void * pvParameters) {
  for(;;) { // Нескінченний цикл для фонової задачі
    
    // Читання датчика (саме тут відбувається блокування I2C, яке раніше викликало миготіння)
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir = lum >> 16;
    uint16_t full = lum & 0xFFFF;
    uint16_t visible = full - ir;
    
    // 📌 ПЕРЕВІРКА НА НАСИЧЕННЯ (ПЕРЕПОВНЕННЯ) ДАТЧИКА
    // Якщо значення наближаються до макс. місткості 16-бітного АЦП (65535)
    if (full > 63000 || ir > 63000) {
      if (tsl2591CurrentGain == TSL2591_GAIN_HIGH) {
        tsl2591CurrentGain = TSL2591_GAIN_MED;
        tsl.setGain(tsl2591CurrentGain);
        //logMessage("Датчик переповнений! Перемикаємо на GAIN_MED");
        vTaskDelay(pdMS_TO_TICKS(100)); // даємо датчику час перебудуватися
        continue; // перезапускаємо цикл для нового зчитування
      } else if (tsl2591CurrentGain == TSL2591_GAIN_MED) {
        tsl2591CurrentGain = TSL2591_GAIN_LOW;
        tsl.setGain(tsl2591CurrentGain);
        //logMessage("Занадто яскраво! Перемикаємо на GAIN_LOW");
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }
    }
    
    // 📌 ПЕРЕВІРКА НА ЗАНАДТО СЛАБКЕ СВІТЛО
    if (full < 200 && ir < 200) {
      if (tsl2591CurrentGain == TSL2591_GAIN_LOW) {
        tsl2591CurrentGain = TSL2591_GAIN_MED;
        tsl.setGain(tsl2591CurrentGain);
        //logMessage("Світло згасло. Перемикаємо на GAIN_MED");
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      } else if (tsl2591CurrentGain == TSL2591_GAIN_MED) {
        tsl2591CurrentGain = TSL2591_GAIN_HIGH;
        tsl.setGain(tsl2591CurrentGain);
        //logMessage("Повна темрява. Повертаємо макс. чутливість GAIN_HIGH");
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }
    }
    
    float lux = tsl.calculateLux(full, ir);
    if (isnan(lux) || lux < 0.1) {
      lux = 0;
    }

    // Ваша логіка фільтрації люксів
    if (lux == LUX_MAX || lux == LUX_MIN || abs(lux - luxGlobal) >= LUX_THRESHOLD) {
      luxGlobal = lux; 
    }

    if (full == 0xFFFF || ir == 0xFFFF || visible >= VISIBLE_MAX_HIGH) {
      visible = VISIBLE_MAX_HIGH;
    }
 
    if (visible <= 1) {  //<= 2
      visible = 0;
    }
    
    visible = constrain(visible, VISIBLE_MIN, VISIBLE_MAX_HIGH);

    // Ваша логіка фільтрації видимого спектра
    if (
      visible >= VISIBLE_MAX_HIGH
      || visible == VISIBLE_MIN
      || (visibleGlobal == 0 && visible > 0)
      || (((visible > 1 && visible < 15) || (visibleGlobal > 1 && visibleGlobal < 15)) && abs(visible - visibleGlobal) >= VISIBLE_THRESHOLD_LOW)
      || ((visible >= 15 || visibleGlobal >= 15) && abs(visible - visibleGlobal) >= VISIBLE_THRESHOLD_HIGH)
    ) {
      visibleGlobal = visible;
    }

    // Замість EVERY_N_SECONDS(2) використовуємо vTaskDelay.
    // Ця функція повністю звільняє Ядро 0 на 2 секунди (2000 мс) для інших завдань мікроконтролера.
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

unsigned long buttonDownTime = 0;
bool buttonPressed = false;

void prefsResetButton() {
  if (digitalRead(RESET_BTN_PIN) == LOW) {  // button pressed
    if (!buttonPressed) {
      buttonPressed = true;
      buttonDownTime = millis();
    } else {
      // if held more than 5 seconds
      if (millis() - buttonDownTime > 5000) {
          //fill all leds with green at full brightness
        FastLED.clear();
        renderDigit(digit_1_leds, 14);//r
        renderDigit(digit_2_leds, 15);//E
        renderDigit(digit_3_leds, 5);//S (5)
        renderDigit(digit_4_leds, 15);//E
        FastLED.setBrightness(50);
        FastLED.show();
        Serial.println("Factory reset!");
        // Clear WiFi credentials
        WiFi.disconnect(true, true);  // forget WiFi and erase NVS
        resetSettings();
        delay(500);
        resetWifiCreds();
        delay(500);
        ESP.restart();                // restart device
      }
    }
  } else {
    buttonPressed = false;
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

struct VarEntry {
  const char* name;
  const char* shortName;
  byte* ptr;
};

// Приклад 1: Просто текст logMessage("Тест датчика TSL2591");
// Приклад 2: Виведення ваших двох змінних (%d — для int/uint16_t) logMessage("Visible: %d, Global: %d", visible, visibleGlobal);
// Приклад 3: Виведення дробових люксів (%f — для float, .2 — два знаки після коми) logMessage("Поточні люкси: %.2f Lx", luxGlobal);
void logMessage(const char* format, ...) {
  if (DEBUG_MODE) {
    char textBuffer[256]; // Тимчасовий буфер для одного рядка логу
    
    // 1. Форматуємо мітку часу [14s]
    unsigned long seconds = millis() / 1000;
    int timeLen = snprintf(textBuffer, sizeof(textBuffer), "[%lus] ", seconds);
    
    // 2. Додаємо туди текст, який ви передали у функцію
    va_list args;
    va_start(args, format);
    // Записуємо текст у буфер одразу за міткою часу
    int msgLen = vsnprintf(&textBuffer[timeLen], sizeof(textBuffer) - timeLen, format, args);
    va_end(args);
    
    int totalLen = timeLen + msgLen;
    
    // 3. Додаємо HTML перенос рядка в кінець
    if (totalLen < (int)sizeof(textBuffer) - 6) {
      strcat(textBuffer, "<br>\n");
      totalLen += 5;
    }

    // 4. Виводимо в Serial (потокобезпечно)
    Serial.print(textBuffer);

    // 5. Записуємо в глобальний debugLog із захистом м'ютексу
    if (logMutex != NULL && xSemaphoreTake(logMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      
      // Якщо новий рядок не влазить в масив, повертаємось на початок (циклічний буфер)
      if (logIndex + totalLen >= LOG_SIZE - 1) {
        logIndex = 0;
        debugLog[0] = '\0';
      }
      
      // Копіюємо сформований рядок у загальний лог
      memcpy(&debugLog[logIndex], textBuffer, totalLen);
      logIndex += totalLen;
      debugLog[logIndex] = '\0'; // Маркер кінця тексту

      xSemaphoreGive(logMutex); // Відкриваємо доступ для іншого ядра
    }
  }
}

//settings/server variables
VarEntry vars[] = {
  {"DAY_BRIGHTNESS", "DAY_BRT", &DAY_BRIGHTNESS},
  {"NIGHT_BRIGHTNESS", "NIGHT_BRT", &NIGHT_BRIGHTNESS},
  {"DAY_SATUR", "DAY_SATUR", &DAY_SATUR},
  {"NIGHT_SATUR", "NIGHT_SATUR", &NIGHT_SATUR},
  {"DAY_COLOR", "DAY_COLOR", &DAY_COLOR},
  {"NIGHT_COLOR", "NIGHT_COLOR", &NIGHT_COLOR},
  {"NIGHT_START_HOUR", "NIGHT_START_H", &NIGHT_START_HOUR},
  {"NIGHT_END_HOUR", "NIGHT_END_H", &NIGHT_END_HOUR},
  {"USE_DITHER", "USE_DITHER", &USE_DITHER},
  {"DITHER_DELAY", "DITHER_DELAY", &DITHER_DELAY},
  {"DITHER_MAX_BRIGHTNESS", "DITHER_MAX_BRT", &DITHER_MAX_BRIGHTNESS},
  {"USE_LDR", "USE_LDR", &USE_LDR},
  {"USE_LDR_DAY", "USE_LDR_DAY", &USE_LDR_DAY},
  {"USE_LDR_NIGHT", "USE_LDR_NIGHT", &USE_LDR_NIGHT},
  {"LDR_MIN_DAY_BRIGHTNESS", "LDR_MIN_D_BRT", &LDR_MIN_DAY_BRIGHTNESS},
  {"LDR_MIN_NIGHT_BRIGHTNESS", "LDR_MIN_N_BRT", &LDR_MIN_NIGHT_BRIGHTNESS},
  {"LDR_READS", "LDR_READS", &LDR_READS},
  {"SHOW_DATE_D", "SHOW_DATE_D", &SHOW_DATE_D},
  {"SHOW_TEMPERATURE_D", "SHOW_TEMP_D", &SHOW_TEMPERATURE_D},
  {"SHOW_HUMIDITY_D", "SHOW_HUM_D", &SHOW_HUMIDITY_D},
  {"SHOW_PRESSURE_D", "SHOW_PRESS_D", &SHOW_PRESSURE_D},
  {"SHOW_DATE_N", "SHOW_DATE_N", &SHOW_DATE_N},
  {"SHOW_TEMPERATURE_N", "SHOW_TEMP_N", &SHOW_TEMPERATURE_N},
  {"SHOW_HUMIDITY_N", "SHOW_HUM_N", &SHOW_HUMIDITY_N},
  {"SHOW_PRESSURE_N", "SHOW_PRESS_N", &SHOW_PRESSURE_N},
  {"SHOW_TIME_SECONDS", "SHOW_TIME_SEC", &SHOW_TIME_SECONDS},
  {"SHOW_DATE_SECONDS", "SHOW_DATE_SEC", &SHOW_DATE_SECONDS},
  {"SHOW_TEMPERATURE_SECONDS", "SHOW_TEMP_SEC", &SHOW_TEMPERATURE_SECONDS},
  {"SHOW_HUMIDITY_SECONDS", "SHOW_HUM_SEC", &SHOW_HUMIDITY_SECONDS},
  {"SHOW_PRESSURE_SECONDS", "SHOW_PRESS_SEC", &SHOW_PRESSURE_SECONDS},
  {"USE_RAINBOW", "USE_RAINBOW", &USE_RAINBOW},
  {"RAINBOW_SPEED", "RAINBOW_SPEED", &RAINBOW_SPEED},
};
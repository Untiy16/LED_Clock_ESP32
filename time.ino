void extractLocalTime() {
  struct tm timeinfo;
  for (int i = 0; i <= 5; i++) {
    if (getLocalTime(&timeinfo)) {
      break;
    } else if (i == 5) {
      setManualTime(0, 0, 0, 12, 4, 2025);
    } else {
      delay(1000);
    }
  }
  
  byte ldrBrightness = 5;
  if (LIGHT_SENSOR_TYPE == 1) {
    byte ldrBrightness = map(ldrAnalog, 0, 4095, 255, 1);
  } else if (LIGHT_SENSOR_TYPE == 2) {
    ldrBrightness = brightnessFromVisible(visibleGlobal);
  }

  if (isHourInRange(timeinfo.tm_hour, NIGHT_START_HOUR, NIGHT_END_HOUR)) {
    isNight = true;
    if (USE_LDR && USE_LDR_NIGHT) {
      CURRENT_BRIGHTNESS = ldrBrightness > LDR_MAX_NIGHT_BRIGHTNESS ? LDR_MAX_NIGHT_BRIGHTNESS : ldrBrightness;
      CURRENT_BRIGHTNESS = CURRENT_BRIGHTNESS < LDR_MIN_NIGHT_BRIGHTNESS ? LDR_MIN_NIGHT_BRIGHTNESS : CURRENT_BRIGHTNESS;
      fixInvisibleColorsOnLowBrightness();
    } else {
      CURRENT_BRIGHTNESS = NIGHT_BRIGHTNESS;
    }
    CURRENT_COLOR = NIGHT_COLOR;
    CURRENT_SATUR = NIGHT_SATUR;
  } else {
    isNight = false;
    if (USE_LDR && USE_LDR_DAY) {
      CURRENT_BRIGHTNESS = ldrBrightness > LDR_MAX_DAY_BRIGHTNESS ? LDR_MAX_DAY_BRIGHTNESS : ldrBrightness;
      CURRENT_BRIGHTNESS = CURRENT_BRIGHTNESS < LDR_MIN_DAY_BRIGHTNESS ? LDR_MIN_DAY_BRIGHTNESS : CURRENT_BRIGHTNESS;
      fixInvisibleColorsOnLowBrightness();
    } else {
      CURRENT_BRIGHTNESS = DAY_BRIGHTNESS;
    }
    CURRENT_COLOR = USE_RAINBOW && CURRENT_BRIGHTNESS >= 5 ? rainbowHue : DAY_COLOR;
    CURRENT_SATUR = DAY_SATUR;
  }

  // smoothBrightnessTransition(timeinfo.tm_hour, timeinfo.tm_min);

  snprintf(hourStr, sizeof(hourStr), "%02d", timeinfo.tm_hour);
  snprintf(minuteStr, sizeof(minuteStr), "%02d", timeinfo.tm_min);
  snprintf(dayStr, sizeof(dayStr), "%02d", timeinfo.tm_mday);    // day 01-31
  snprintf(monthStr, sizeof(monthStr), "%02d", timeinfo.tm_mon + 1); // month 01-12
}

void getTimeFromInternet() {
  byte tries = 0;
  bool success = true;
  while (WiFi.status() != WL_CONNECTED) {
    tries++;
    delay(1000);
    // Serial.println("Connecting to WiFi...");
    if (tries >= 15) {
      success = false;
      break;
    }
  }

  // Serial.println(success ? "Connected to WiFi" : "Fail to connect to WiFi");
  if (success) {
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    configTzTime(timezone, ntpServer);
  }
}

void updateTimeFromInternet() {
  struct tm lastKnownTime;
  getLocalTime(&lastKnownTime);
  configTzTime(timezone, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    // Serial.println("Time updated from NTP");
  } else {
    // Serial.println("Failed to update time from NTP, using last known time.");
    time_t t = mktime(&timeinfo);  // Convert to time_t
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, nullptr);  // Set the system time
  }
}

void setManualTime(int hour, int minute, int second, int day, int month, int year) {
  struct tm timeinfo = {};
  timeinfo.tm_year = year - 1900;  // Years since 1900
  timeinfo.tm_mon  = month - 1;    // Months since January (0–11)
  timeinfo.tm_mday = day;
  timeinfo.tm_hour = hour;
  timeinfo.tm_min  = minute;
  timeinfo.tm_sec  = second;

  time_t t = mktime(&timeinfo); // Convert to time_t
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, nullptr);
}

bool isHourInRange(int hour, int startHour, int endHour) {
  if (startHour == endHour) {
    return false;
  }
  if (startHour <= endHour) {
    return (hour >= startHour && hour < endHour);
  } else {  // Range wraps around midnight (e.g., 10 PM to 5 AM)
    return (hour >= startHour || hour < endHour);
  }
}

void smoothBrightnessTransition(int hours, int minutes) {
  if (hours == NIGHT_END_HOUR) {
    CURRENT_BRIGHTNESS = map(minutes, 0, 59, NIGHT_BRIGHTNESS, DAY_BRIGHTNESS);
  } else if (hours == (NIGHT_START_HOUR == 0 ? 23 : NIGHT_START_HOUR - 1)) {
    CURRENT_BRIGHTNESS = map(59 - minutes, 0, 59, constrain(NIGHT_BRIGHTNESS + 5, NIGHT_BRIGHTNESS, DAY_BRIGHTNESS), DAY_BRIGHTNESS);
  }
}

void fixInvisibleColorsOnLowBrightness() {
  if (CURRENT_BRIGHTNESS == 1 && (CURRENT_COLOR > 72 && CURRENT_COLOR < 208)) {
    CURRENT_BRIGHTNESS = 2;
  }
}

// Координати для запиту (Кропивницький: Latitude: 48.51, Longitude: 32.26)
const String lat = "48.51";
const String lon = "32.26";

// URL-адреса API Open-Meteo для отримання поточної температури
const String serverPath = "https://api.open-meteo.com/v1/forecast?latitude=" + lat + "&longitude=" + lon + "&current=temperature_2m&temperature_unit=celsius";

void getTemperatureWeb() {
  HTTPClient http;

  http.begin(serverPath);
  http.setTimeout(4000);
  http.setConnectTimeout(4000);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      // {"current": {"temperature_2m": X.X}} — store whole degrees only
      webTemperature = lround(doc["current"]["temperature_2m"].as<float>());
    }
  }

  http.end();
}

void formatWebTemperature(char* out, size_t outLen) {
  int temperature = webTemperature;
  if (temperature <= -1000) {
    strlcpy(out, "° --", outLen);
    return;
  }

  char digits[8];
  snprintf(digits, sizeof(digits), "%02d", abs(temperature));
  snprintf(out, outLen, "%s%s", temperature >= 0 ? "° " : "°-", digits);
}

void temperatureWebWorker(void *pvParameters) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      getTemperatureWeb();
      vTaskDelay(pdMS_TO_TICKS(10 * 60 * 1000));
    } else {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void startTemperatureWebTask() {
  if (TempWebTaskHandle != NULL) return;

  xTaskCreatePinnedToCore(
    temperatureWebWorker,
    "TempWeb",
    12288,
    NULL,
    1,
    &TempWebTaskHandle,
    0
  );
}
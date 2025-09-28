void extractLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    // Serial.println("Failed to obtain time");
    setManualTime(0, 0, 0, 12, 4, 2025);
  }

  
  if (isHourInRange(timeinfo.tm_hour, NIGHT_START_HOUR, NIGHT_END_HOUR)) {
    // CURRENT_BRIGHTNESS = NIGHT_BRIGHTNESS;
    CURRENT_BRIGHTNESS = map(ldrAnalog, 0, 4095, 255, 1);
    // CURRENT_BRIGHTNESS = map(ldrAnalog, 0, 4095, 255, 1);
    // dd(ldrAnalog);
    // dd(CURRENT_BRIGHTNESS);
    CURRENT_COLOR = NIGHT_COLOR;
    CURRENT_SATUR = NIGHT_SATUR;
  } else {
    CURRENT_BRIGHTNESS = DAY_BRIGHTNESS;
    CURRENT_COLOR = DAY_COLOR;
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
  if (startHour <= endHour) {
    return (hour >= startHour && hour <= endHour);
  } else {  // Range wraps around midnight (e.g., 10 PM to 5 AM)
    return (hour >= startHour || hour <= endHour);
  }
}

void smoothBrightnessTransition(int hours, int minutes) {
  if (hours == NIGHT_END_HOUR) {
    CURRENT_BRIGHTNESS = map(minutes, 0, 59, NIGHT_BRIGHTNESS, DAY_BRIGHTNESS);
  } else if (hours == (NIGHT_START_HOUR == 0 ? 23 : NIGHT_START_HOUR - 1)) {
    CURRENT_BRIGHTNESS = map(59 - minutes, 0, 59, constrain(NIGHT_BRIGHTNESS + 5, NIGHT_BRIGHTNESS, DAY_BRIGHTNESS), DAY_BRIGHTNESS);
  }
}
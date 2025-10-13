// !!! There is a key length limit <= 15 !!!
// DAY_BRIGHTNESS --> DAY_BRT
// NIGHT_BRIGHTNESS --> NIGHT_BRT
// NIGHT_START_HOUR --> NIGHT_START_H
// NIGHT_END_HOUR --> NIGHT_END_H
// SHOW_TEMPERATURE --> SHOW_TEMP
// SHOW_HUMIDITY --> SHOW_HUM
// SHOW_PRESSURE --> SHOW_PRESS
// SHOW_TIME_SECONDS --> SHOW_TIME_SEC
// SHOW_DATE_SECONDS --> SHOW_DATE_SEC
// SHOW_TEMPERATURE_SECONDS --> SHOW_TEMP_SEC
// SHOW_HUMIDITY_SECONDS --> SHOW_HUM_SEC
// SHOW_PRESSURE_SECONDS --> SHOW_PRESS_SEC


void loadSettings() {
  prefs.begin("settings", true);       // Open in read-only mode

  NIGHT_START_HOUR = prefs.getInt("NIGHT_START_H", NIGHT_START_HOUR);
  NIGHT_END_HOUR = prefs.getInt("NIGHT_END_H", NIGHT_END_HOUR);
  DAY_BRIGHTNESS = prefs.getInt("DAY_BRT", DAY_BRIGHTNESS);
  NIGHT_BRIGHTNESS = prefs.getInt("NIGHT_BRT", NIGHT_BRIGHTNESS);
  DAY_SATUR = prefs.getInt("DAY_SATUR", DAY_SATUR);
  NIGHT_SATUR = prefs.getInt("NIGHT_SATUR", NIGHT_SATUR);
  DAY_COLOR = prefs.getInt("DAY_COLOR", DAY_COLOR);
  NIGHT_COLOR = prefs.getInt("NIGHT_COLOR", NIGHT_COLOR);
  USE_DITHER = prefs.getInt("USE_DITHER", USE_DITHER);
  USE_LDR = prefs.getInt("USE_LDR", USE_LDR);
  USE_LDR_DAY = prefs.getInt("USE_LDR_DAY", USE_LDR_DAY);
  USE_LDR_NIGHT = prefs.getInt("USE_LDR_NIGHT", USE_LDR_NIGHT);
  LDR_READS = prefs.getInt("LDR_READS", LDR_READS);
  SHOW_DATE_D = prefs.getInt("SHOW_DATE_D", SHOW_DATE_D);
  SHOW_TEMPERATURE_D = prefs.getInt("SHOW_TEMP_D", SHOW_TEMPERATURE_D);
  SHOW_HUMIDITY_D = prefs.getInt("SHOW_HUM_D", SHOW_HUMIDITY_D);
  SHOW_PRESSURE_D = prefs.getInt("SHOW_PRESS_D", SHOW_PRESSURE_D);
  SHOW_DATE_N = prefs.getInt("SHOW_DATE_N", SHOW_DATE_N);
  SHOW_TEMPERATURE_N = prefs.getInt("SHOW_TEMP_N", SHOW_TEMPERATURE_N);
  SHOW_HUMIDITY_N = prefs.getInt("SHOW_HUM_N", SHOW_HUMIDITY_N);
  SHOW_PRESSURE_N = prefs.getInt("SHOW_PRESS_N", SHOW_PRESSURE_N);
  SHOW_TIME_SECONDS = prefs.getInt("SHOW_TIME_SEC", SHOW_TIME_SECONDS);
  SHOW_DATE_SECONDS = prefs.getInt("SHOW_DATE_SEC", SHOW_DATE_SECONDS);
  SHOW_TEMPERATURE_SECONDS = prefs.getInt("SHOW_TEMP_SEC", SHOW_TEMPERATURE_SECONDS);
  SHOW_HUMIDITY_SECONDS = prefs.getInt("SHOW_HUM_SEC", SHOW_HUMIDITY_SECONDS);
  SHOW_PRESSURE_SECONDS = prefs.getInt("SHOW_PRESS_SEC", SHOW_PRESSURE_SECONDS);

  prefs.end();
}

void saveSettings() {
  prefs.begin("settings", false);      // Open namespace "settings" in RW mode

  prefs.putInt("DAY_BRT", DAY_BRIGHTNESS);
  prefs.putInt("NIGHT_BRT", NIGHT_BRIGHTNESS);
  prefs.putInt("DAY_SATUR", DAY_SATUR);
  prefs.putInt("NIGHT_SATUR", NIGHT_SATUR);
  prefs.putInt("DAY_COLOR", DAY_COLOR);
  prefs.putInt("NIGHT_COLOR", NIGHT_COLOR);
  prefs.putInt("NIGHT_START_H", NIGHT_START_HOUR);
  prefs.putInt("NIGHT_END_H", NIGHT_END_HOUR);
  prefs.putInt("USE_DITHER", USE_DITHER);
  prefs.putInt("USE_LDR", USE_LDR);
  prefs.putInt("USE_LDR_DAY", USE_LDR_DAY);
  prefs.putInt("USE_LDR_NIGHT", USE_LDR_NIGHT);
  prefs.putInt("LDR_READS", LDR_READS);
  prefs.putInt("SHOW_DATE_D", SHOW_DATE_D);
  prefs.putInt("SHOW_TEMP_D", SHOW_TEMPERATURE_D);
  prefs.putInt("SHOW_HUM_D", SHOW_HUMIDITY_D);
  prefs.putInt("SHOW_PRESS_D", SHOW_PRESSURE_D);
  prefs.putInt("SHOW_DATE_N", SHOW_DATE_N);
  prefs.putInt("SHOW_TEMP_N", SHOW_TEMPERATURE_N);
  prefs.putInt("SHOW_HUM_N", SHOW_HUMIDITY_N);
  prefs.putInt("SHOW_PRESS_N", SHOW_PRESSURE_N);
  prefs.putInt("SHOW_TIME_SEC", SHOW_TIME_SECONDS);
  prefs.putInt("SHOW_DATE_SEC", SHOW_DATE_SECONDS);
  prefs.putInt("SHOW_TEMP_SEC", SHOW_TEMPERATURE_SECONDS);
  prefs.putInt("SHOW_HUM_SEC", SHOW_HUMIDITY_SECONDS);
  prefs.putInt("SHOW_PRESS_SEC", SHOW_PRESSURE_SECONDS);
  
  prefs.end();
}

//Erase persistant memory. Run once is setup before any prefs actions
void eraseNVS() {
  //Erase all NVS data (ONLY if you want a fresh start)
  esp_err_t err = nvs_flash_erase();
  if (err == ESP_OK) {
    Serial.println("NVS flash erased.");
  }

  //Initialize NVS
  err = nvs_flash_init();
  if (err == ESP_OK) {
    Serial.println("NVS initialized.");
  } else {
    Serial.printf("NVS init failed: %d\n", err);
  }
}
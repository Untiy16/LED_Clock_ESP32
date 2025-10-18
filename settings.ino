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
  
  for (auto &v : vars) {
    *v.ptr = prefs.getInt(v.shortName, *v.ptr);
  }

  prefs.end();
}

void saveSettings() {
  prefs.begin("settings", false);      // Open namespace "settings" in RW mode
  
  for (auto &v : vars) {
    prefs.putInt(v.shortName, *v.ptr);
  }

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
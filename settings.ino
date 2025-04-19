void loadSettings() {
  prefs.begin("settings", true);       // Open in read-only mode
  NIGHT_START_HOUR = prefs.getInt("NIGHT_START_HOUR", NIGHT_START_HOUR);
  NIGHT_END_HOUR = prefs.getInt("NIGHT_END_HOUR", NIGHT_END_HOUR);
  DAY_BRIGHTNESS = prefs.getInt("DAY_BRIGHTNESS", DAY_BRIGHTNESS);
  NIGHT_BRIGHTNESS = prefs.getInt("NIGHT_BRIGHTNESS", NIGHT_BRIGHTNESS);
  DAY_SATUR = prefs.getInt("DAY_SATUR", DAY_SATUR);
  NIGHT_SATUR = prefs.getInt("NIGHT_SATUR", NIGHT_SATUR);
  DAY_COLOR = prefs.getInt("DAY_COLOR", DAY_COLOR);
  NIGHT_COLOR = prefs.getInt("NIGHT_COLOR", NIGHT_COLOR);
  prefs.end();
}

void saveSettings() {
  prefs.begin("settings", false);      // Open namespace "settings" in RW mode
  prefs.putInt("DAY_BRIGHTNESS", DAY_BRIGHTNESS);
  prefs.putInt("NIGHT_BRIGHTNESS", NIGHT_BRIGHTNESS);
  prefs.putInt("DAY_SATUR", DAY_SATUR);
  prefs.putInt("NIGHT_SATUR", NIGHT_SATUR);
  prefs.putInt("DAY_COLOR", DAY_COLOR);
  prefs.putInt("NIGHT_COLOR", NIGHT_COLOR);
  prefs.putInt("NIGHT_START_HOUR", NIGHT_START_HOUR);
  prefs.putInt("NIGHT_END_HOUR", NIGHT_END_HOUR);
  prefs.end();
}
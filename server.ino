void serverBegin() {
  ElegantOTA.begin(&server);
  server.on("/", handleRoot);
  server.on("/save-wifi-creds", HTTP_POST, handleSaveWifiCredsPost);
  server.on("/settings", HTTP_GET, handleSettingsGet);
  server.on("/settings", HTTP_POST, handleSettingsPost);
  server.on("/setting-preview", HTTP_POST, handleSettingPreviewPost);
  server.on("/sensors", handleSensors);
  server.on("/set-time", HTTP_POST, handleSetTime);
  server.on("/reboot", handleReboot);
  server.on("/resetwifi", handleResetWifiCreds);
  server.begin();
}

String successResponse(String message = "Success!", String redirectTo = "", int delay = 3) {
  if (redirectTo != "") {
    return "<html><body><meta http-equiv='refresh' content='" + String(delay) + "; url=" + redirectTo + "' />" + message + " You will be redirected to " + redirectTo + " in <span id='timer'>" + String(delay) + "</span> seconds!</body><script>setInterval(()=>{timer.textContent=--timer.textContent<=0?0:timer.textContent;},1000);</script></html>";
  } else {
    return "<html><body>" + message + "</body></html>";
  }
}

String htmlTemplate(String html) {
  return "<html><head><meta charset='UTF-8'><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><style>*{box-sizing:border-box;}body,form{display:flex}body{font-family:Arial,sans-serif;background-color:#f4f4f4;color:#333;margin:0;padding:0;justify-content:center;align-items:center;flex-direction: column;}h2{margin-bottom:20px;color:#007bff}.checkbox-wrapper+h2{margin-top:0;}form{background:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,.1);width:450px;flex-direction:column}input[type=number],input[type=text],input[type=password]{margin-top: 10px;margin-bottom:5px;padding:10px;border:1px solid #ccc;border-radius:4px;font-size:16px;width:100%;box-sizing:border-box}input[type=submit]{background-color:#28a745;color:#fff;border:none;border-radius:4px;padding:12px;cursor:pointer;font-size:16px;transition:background-color .3s}input[type=submit]:hover{background-color:#218838}@media (max-width:600px){form{width:100%; width:-webkit-fill-available; margin:5px 10px;}.footer-links{flex-direction: column; text-align: center;}}.color-preview-parent{display:flex;margin-bottom:15px}.color-preview-parent>div{display:flex;flex-direction:column;justify-content:center;align-items:center;width:50%}.color-preview{margin-top:5px;width:50px;height:50px;border:1px solid #000}.color-preview-fullbrightness{content:'';display:block;width:50%;height:100%;}.checkbox-wrapper{display:flex;justify-content:space-between;margin-bottom:26px;}.tooltip{border-bottom:1px dotted black;}.footer-links a{color:#0000ef;}.footer-links{margin-bottom: 50px;margin-top: 20px;gap: 15px;display: flex;}hr{margin: 0 0 26px 0;}/* input ranges styles */.hsv-range{-webkit-appearance:none;margin:10px 0;width:100%}.hsv-range:focus{outline:none}.hsv-range::-webkit-slider-runnable-track{width:100%;cursor:pointer;animate:0.2s;box-shadow:0 0 0 #000000,0 0 0 #0d0d0d;background:linear-gradient(to right,red,orange,yellow,lawngreen,aqua,blue,blueviolet,magenta,red);border-radius:25px;border:0 solid #000101}.hsv-range::-webkit-slider-thumb{box-shadow:0 0 0 #000000,0 0 0 #0d0d0d;border:3px solid #fff;height:20px;width:20px;border-radius:50%;background:#fff0;cursor:pointer;-webkit-appearance:none}.hsv-range::-moz-range-track{width:100%;cursor:pointer;animate:0.2s;box-shadow:0 0 0 #000000,0 0 0 #0d0d0d;background:linear-gradient(to right,#c90015,#583700,#0f5d00,#025a5b,#003fcb,#a018a1,#c90015);border-radius:25px;border:0 solid #000101}.hsv-range::-moz-range-thumb{box-shadow:0 0 0 #000000,0 0 0 #0d0d0d;border:3px solid #fff;height:20px;width:20px;border-radius:50%;background:#fff0;cursor:pointer}.hsv-range::-ms-track{width:100%;height:20px;cursor:pointer;animate:0.2s;background:linear-gradient(to right,#c90015,#583700,#0f5d00,#025a5b,#003fcb,#a018a1,#c90015);border-color:#fff0;border-width:0;color:#fff0}.hsv-range::-ms-thumb{box-shadow:0 0 0 #000000,0 0 0 #0d0d0d;border:3px solid #fff;height:20px;width:20px;border-radius:50%;background:#fff0;cursor:pointer}.range-wrapper{display:flex;margin-top:15px;}.range-wrapper input[type=text]{width:35px;padding:0;text-align:center;margin:0 5px 0 0;height:min-content;}.range-wrapper input[type=number]{width:min-content;}.range-wrapper input[type=range]{width:100%;margin:0;}</style></head><body>" + html + "<div class='footer-links'><a href='/'>Main</a><a href='/settings'>Settings</a><a href='/settings?ajax=1'>Settings (ajax)</a><a href='/sensors'>Sensors data</a><a href='/reboot'>Reboot ESP</a><a href='/resetwifi'>Reset WiFi creds</a><a href='/update'>Update firmware</a></div></body></html>";
}

String renderRangeInput(int value, int min = 0, int max = 255, bool isHsv = false) {
  return "<input value=\"" + String(value) + "\" type=\"range\" min=\"" + String(min) + "\" max=\"" + String(max) + "\" class=\"" + String(isHsv ? "hsv-range" : "") + "\" oninput=\"if (this.previousElementSibling.value != this.value) {this.previousElementSibling.value = this.value;this.previousElementSibling.dispatchEvent(new Event('input', {bubbles: true}));}\">";
}

String renderInput(int value, String name, int min = 0, int max = 255, String type = "number") {
  return "<input value=\"" + String(value) + "\" type=\"" + type + "\" min=\"" + String(min) + "\" max=\"" + String(max) + "\" name=\"" + name + "\" oninput=\"if (this.nextElementSibling.value != this.value) {this.nextElementSibling.value = this.value;this.nextElementSibling.dispatchEvent(new Event('input', {bubbles: true}));}\">";
}

String renderInputWithRange(int value, String name, bool isHsv = false, int min = 0, int max = 255, String type = "text") {
  return  "<div class='range-wrapper'>" + renderInput(value, name, min, max, type) + renderRangeInput(value, min, max, isHsv) + "</div>";
}

String renderCheckbox(int value, String name, String label, bool wrapOpen = false, bool wrapClose = false, String tooltip = "") {
  return String(wrapOpen ? "<div class=\"checkbox-wrapper\">" : "") + "<div><label for=\"" + name + "\"" + String(tooltip != "" ? "class=\"tooltip\" title=\"" + tooltip + "\"" : "") + ">" + label + "</label><input type=\"checkbox\" id=\"" + name + "\"" + String(value ? "checked" : "") + " onchange=\"this.nextElementSibling.value = this.checked ? 1 : 0; this.nextElementSibling.dispatchEvent(new Event('input', {bubbles: true}));\"><input type=\"hidden\" value=\"" + String(value) + "\" name=\"" + name + "\"></div>" + String(wrapClose ? "</div>" : "");
}

void handleSettingsGet() {
  String html = "<h2>LED Clock Settings</h2>";
  html += "<form action='/settings' method='POST'>";
  html += "Day color (0-255): " + renderInputWithRange(DAY_COLOR, "DAY_COLOR", true) + "<br>";
  html += "Day brightness (0-255): " + renderInputWithRange(DAY_BRIGHTNESS, "DAY_BRIGHTNESS") + "<br>";
  html += "Day saturation (0-255): " + renderInputWithRange(DAY_SATUR, "DAY_SATUR") + "<br>";
  html += "Night color (0-255): " + renderInputWithRange(NIGHT_COLOR, "NIGHT_COLOR", true) + "<br>";
  html += "Night brightness (0-255): " + renderInputWithRange(NIGHT_BRIGHTNESS, "NIGHT_BRIGHTNESS") + "<br>";
  html += "Night saturation (0-255): " + renderInputWithRange(NIGHT_SATUR, "NIGHT_SATUR") + "<br>";
  html += "Night start hour (0-23): " + renderInputWithRange(NIGHT_START_HOUR, "NIGHT_START_HOUR", false, 0, 23) + "<br>";
  html += "Night end hour (0-23): " + renderInputWithRange(NIGHT_END_HOUR, "NIGHT_END_HOUR", false, 0, 23) + "<br>";
  html += renderCheckbox(USE_DITHER, "USE_DITHER", "Use dither", true, true, "Allows to reduce brightness by flickering LEDs");
  html += "Dither delay (1-255): " + renderInputWithRange(DITHER_DELAY, "DITHER_DELAY", false, 1, 255) + "<br>";
  html += "Dither max brightness (0-255): " + renderInputWithRange(DITHER_MAX_BRIGHTNESS, "DITHER_MAX_BRIGHTNESS") + "<br>";
  
  html += "<h2>Modes</h2>";
  html += "Show time for X seconds: <input type='number' name='SHOW_TIME_SECONDS' value='" + String(SHOW_TIME_SECONDS) + "' min='1'><br>";
  html += "<div>Day modes:</div><br>" + renderCheckbox(SHOW_DATE_D, "SHOW_DATE_D", "Date", true) + renderCheckbox(SHOW_TEMPERATURE_D, "SHOW_TEMPERATURE_D", "Temp") + renderCheckbox(SHOW_HUMIDITY_D, "SHOW_HUMIDITY_D", "Humidity") + renderCheckbox(SHOW_PRESSURE_D, "SHOW_PRESSURE_D", "Pressure", false, true) + "<hr>";
  html += "<div>Night modes:</div><br>" + renderCheckbox(SHOW_DATE_N, "SHOW_DATE_N", "Date", true) + renderCheckbox(SHOW_TEMPERATURE_N, "SHOW_TEMPERATURE_N", "Temp") + renderCheckbox(SHOW_HUMIDITY_N, "SHOW_HUMIDITY_N", "Humidity") + renderCheckbox(SHOW_PRESSURE_N, "SHOW_PRESSURE_N", "Pressure", false, true) + "<hr>";
  html += "Show mode for X seconds: <div style='display: flex;justify-content: space-between;gap: 25px;'><input type='number' name='SHOW_DATE_SECONDS' value='" + String(SHOW_DATE_SECONDS) + "' min='1'><input type='number' name='SHOW_TEMPERATURE_SECONDS' value='" + String(SHOW_TEMPERATURE_SECONDS) + "' min='1'><input type='number' name='SHOW_HUMIDITY_SECONDS' value='" + String(SHOW_HUMIDITY_SECONDS) + "' min='1'><input type='number' name='SHOW_PRESSURE_SECONDS' value='" + String(SHOW_PRESSURE_SECONDS) + "' min='1'></div>";
  
  html += "<h2>Sensors</h2>";
  html += renderCheckbox(USE_LDR, "USE_LDR", "Use LDR", true, false, "Brightness will be automatically adjusted based on the ambient light") + renderCheckbox(USE_LDR_DAY, "USE_LDR_DAY", "Day") + renderCheckbox(USE_LDR_NIGHT, "USE_LDR_NIGHT", "Night", false, true);
  html += "Minimal brightness whe using LDR: <div style=\"display:flex;justify-content: flex-end;gap:85px;\"><input type=\"number\" name=\"LDR_MIN_DAY_BRIGHTNESS\" min=\"0\" max=\"255\" value=\"" + String(LDR_MIN_DAY_BRIGHTNESS) + "\" style=\"width: 85px;\"><input type=\"number\" name=\"LDR_MIN_NIGHT_BRIGHTNESS\" min=\"0\" max=\"255\" value=\"" + String(LDR_MIN_NIGHT_BRIGHTNESS) + "\" style=\"width: 85px;\"></div><br>";
  
  html += "LRD: number of readings <input type='number' name='LDR_READS' value='" + String(LDR_READS) + "' min='0'><br>";
  
  html += "<h2>Effects</h2>";
  html += renderCheckbox(USE_RAINBOW, "USE_RAINBOW", "Use rainbow animation", true, true, "Colors will smoothly cycle through the full rainbow spectrum in day mode");
  html += "Rainbow animation speed (ms): <input type='number' name='RAINBOW_SPEED' value='" + String(RAINBOW_SPEED) + "' min='5'><br>";
  
  html += "<input type='submit' value='Save'>";
  html += "</form>";
  
  html += "<form action='/set-time' method='POST'>Set time:<div><input type='text' name='time' style='width: 80%;'><input type='number' min='-1' max='1' name='time-adjust' value='0' style='width: calc(20% - 10px);margin-left: 10px;'></div><br><input type='submit' value='Save'></form>";
  html += "<script>setInterval(function () { let currDate = new Date(); document.querySelector('[name=time]').value = `${currDate.getHours() - document.querySelector('[name=time-adjust]').value},${currDate.getMinutes()},${currDate.getSeconds()},${currDate.getDate()},${currDate.getMonth() + 1},${currDate.getFullYear()}`;}, 1000);</script>";
  html += "<script>if(location.href.includes('ajax')){document.querySelectorAll('form[action=\"/settings\"] input[name]').forEach(input => { input.addEventListener('input', async function() { const data = {}; data[this.name] = this.value; await fetch('/setting-preview', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: new URLSearchParams(data) }); });}); }</script>";//ajax settings
  
  server.send(200, "text/html", htmlTemplate(html));
}



void handleSettingsPost() {
  for (auto &v : vars) {
    if (server.hasArg(v.name)) {
      *v.ptr = server.arg(v.name).toInt();
    }
  }
  
  FastLED.setDither(USE_DITHER);

  saveSettings();
  // Respond and redirect
  server.send(200, "text/html", successResponse("Settings saved successfully!", "/settings", 0));
}

void handleSettingPreviewPost() {
  for (auto &v : vars) {
    if (server.hasArg(v.name)) {
      *v.ptr = server.arg(v.name).toInt();
      break;
    }
  }
  
  FastLED.setDither(USE_DITHER);
  server.send(200, "text/html", "");
}

void handleSetTime() {
  if (server.hasArg("time")) { 
    String input = server.arg("time");
    int values[6]; 
    int index = 0;
    int lastIndex = 0;

    while (index < 6) {
      int commaIndex = input.indexOf(',', lastIndex);
      if (commaIndex == -1) commaIndex = input.length();  // Last item

      String part = input.substring(lastIndex, commaIndex);
      values[index] = part.toInt();

      lastIndex = commaIndex + 1;
      index++;
    }
    // dd(values[0], values[1], values[2], values[3], values[4], values[5]);

    setManualTime(values[0], values[1], values[2], values[3], values[4], values[5]);
  }

  // Respond and redirect
  server.send(200, "text/html", successResponse("Time was successfully set!", "/settings"));
}

void handleSensors() {
  // sensors_event_t humidity, temp;
  // aht.getEvent(&humidity, &temp);
  
  String html = "<h2>Sensors values</h2><br>";
  html += "<form style='min-width: 50%;'>";
  html += "<h2>LDR</h2>";
  html += "<h3>Analog: " + String(analogRead(LDR_A_PIN)) + "</h3>";
  // html += "<h3>Digital: " + String(digitalRead(LDR_A_PIN)) + "</h3>";
  // html += "<h2>Temperature</h2>";
  // html += "<h3>" + String(temp.temperature) + " °C</h3>";
  // html += "<h3>" + String(temp.temperature * 1.8 + 32) + " °F</h3>";
  // html += "<h2>Humidity</h2>";
  // html += "<h3>" + String(humidity.relative_humidity) + " %</h3>";
  // html += "<h2>Pressure</h2>";
  // html += "<h3>" + String(bmp.readPressure() / 100 * 0.75) + " mmHg</h3>";
  // html += "<h3>" + String(bmp.readPressure()) + " Pa</h3>";
  // html += "<h3>" + String(bmp.readPressure() / 100) + " hPa</h3>";
  html += "</form>";

  server.send(200, "text/html", htmlTemplate(html));
}

void handleRoot() {
  String trimmed = savedSSID;
  trimmed.trim();  
  dd("savedSSID", savedSSID, "savedSSID");
  dd("savedSSID", trimmed, "savedSSID");
  if (trimmed.length() != 0) {
    server.send(200, "text/html", successResponse("", "/settings", 0));
  }
  // Simple HTML form to enter SSID and password
  String html = "<h2>WiFi Credentials</h2>";
  html += "<form action='/save-wifi-creds' method='POST'>";
  html += "SSID: <input type='text' name='ssid'><br>";
  html += "Password: <input type='password' name='pass' type='password'><br>";
  html += "<input type='submit' value='Save'>";
  html += "</form>";
  server.send(200, "text/html", htmlTemplate(html));
}

void handleSaveWifiCredsPost() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    // Save credentials in Preferences
    prefs.begin("wifiCreds", false);
    prefs.putString("ssid", server.arg("ssid"));
    prefs.putString("pass", server.arg("pass"));
    prefs.end();

    server.send(200, "text/html", successResponse("Saved! Rebooting...", "http://192.168.1.81", 5));
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/html", "Missing fields");
  }
}

void handleReboot() {
    server.send(200, "text/html", successResponse("Rebooting... ", "/", 5));
    delay(2000);
    ESP.restart();
}

void handleResetWifiCreds() {
    server.send(200, "text/html", successResponse("Rebooting... Connect to ESP32_LED_Clock_AP_2 and go to 192.168.4.1 to set a new WiFi creds", "http://192.168.4.1", 15));
    prefs.begin("wifiCreds", false);
    prefs.clear();               // clears everything in this namespace
    prefs.end();
    // ssid = "";
    // password = "";
    delay(2000);
    ESP.restart();
}
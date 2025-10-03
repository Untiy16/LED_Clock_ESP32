void serverBegin() {
  server.on("/", handleRoot);
  server.on("/save-wifi-creds", HTTP_POST, handleSaveWifiCredsPost);
  server.on("/settings", HTTP_GET, handleSettingsGet);
  server.on("/settings", HTTP_POST, handleSettingsPost);
  server.on("/sensors", handleSensors);
  server.on("/set-time", HTTP_POST, handleSetTime);
  server.on("/reboot", handleReboot);
  server.on("/resetwifi", handleResesWifiCreds);
  server.begin();
}

String responseWithRedirect(String message = "Settings saved successfully!", String redirectTo = "/settings", int delay = 2) {
  return "<html><body><meta http-equiv='refresh' content='" + String(delay) + "; url=" + redirectTo + "' />" + message + "</body></html>";
}

String htmlTemplate(String html) {
  return "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><style>body,form{display:flex}body{font-family:Arial,sans-serif;background-color:#f4f4f4;color:#333;margin:0;padding:0;justify-content:center;align-items:center;flex-direction: column;}h2{margin-bottom:20px;color:#007bff}.checkbox-wrapper+h2{margin-top:0;}form{background:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,.1);width:300px;flex-direction:column}input[type=number],input[type=text],input[type=password]{margin-top: 10px;margin-bottom:5px;padding:10px;border:1px solid #ccc;border-radius:4px;font-size:16px;width:100%;box-sizing:border-box}input[type=submit]{background-color:#28a745;color:#fff;border:none;border-radius:4px;padding:12px;cursor:pointer;font-size:16px;transition:background-color .3s}input[type=submit]:hover{background-color:#218838}@media (max-width:600px){form{width:90%}}.color-preview-parent{display:flex;margin-bottom:15px}.color-preview-parent>div{display:flex;flex-direction:column;justify-content:center;align-items:center;width:50%}.color-preview{margin-top:5px;width:50px;height:50px;border:1px solid #000}.checkbox-wrapper{display:flex;justify-content:space-between;margin-bottom:26px;}.tooltip{border-bottom:1px dotted black;}.footer-links{margin-bottom: 50px;margin-top: 20px;gap: 15px;display: flex;}</style></head><body>" + html + "</body></html>";
}

void handleSettingsGet() {
  String html = "<h2>LED Clock Settings</h2>";
  html += "<form action='/settings' method='POST'>";
  html += "<div class='color-preview-parent'><div>Day<div class='color-preview color-preview-day'></div></div><div>Night<div class='color-preview color-preview-night'></div></div></div>";
  html += "Day color (0-255): <input type='number' name='DAY_COLOR' value='" + String(DAY_COLOR) + "' min='0' max='255'><br>";
  html += "Day saturation (0-255): <input type='number' name='DAY_SATUR' value='" + String(DAY_SATUR) + "' min='0' max='255'><br>";
  html += "Day brightness (0-255): <input type='number' name='DAY_BRIGHTNESS' value='" + String(DAY_BRIGHTNESS) + "' min='0' max='255'><br>";
  html += "Night color (0-255): <input type='number' name='NIGHT_COLOR' value='" + String(NIGHT_COLOR) + "' min='0' max='255'><br>";
  html += "Night saturation (0-255): <input type='number' name='NIGHT_SATUR' value='" + String(NIGHT_SATUR) + "' min='0' max='255'><br>";
  html += "Night brightness (0-255): <input type='number' name='NIGHT_BRIGHTNESS' value='" + String(NIGHT_BRIGHTNESS) + "' min='0' max='255'><br>";
  html += "Night start hour (0-23): <input type='number' name='NIGHT_START_HOUR' value='" + String(NIGHT_START_HOUR) + "'  min='0' max='23'><br>";
  html += "Night end hour (0-23): <input type='number' name='NIGHT_END_HOUR' value='" + String(NIGHT_END_HOUR) + "'  min='0' max='23'><br>";
  html += "<div class='checkbox-wrapper'><div><label title='Allows to reduce brightness by flickering LEDs' class='tooltip' for='USE_DITHER'>Use dither</label><input type='checkbox' name='USE_DITHER' id='USE_DITHER' value='" + String(USE_DITHER) + "' " + String(USE_DITHER ? "checked" : "") + " onchange='this.value=this.checked?1:0'></div></div>";
  
  html += "<h2>Modes</h2>";
  html += "Show time for X seconds: <input type='number' name='SHOW_TIME_SECONDS' value='" + String(SHOW_TIME_SECONDS) + "' min='1'><br>";
  html += "<div class='checkbox-wrapper'><div><label for='SHOW_DATE'>Date</label><input type='checkbox' name='SHOW_DATE' id='SHOW_DATE' value='" + String(SHOW_DATE) + "' " + String(SHOW_DATE ? "checked" : "") + " onchange='this.value=this.checked?1:0'></div><div><label for='SHOW_TEMPERATURE'>Temp</label><input type='checkbox' name='SHOW_TEMPERATURE' id='SHOW_TEMPERATURE' value='" + String(SHOW_TEMPERATURE) + "' " + String(SHOW_TEMPERATURE ? "checked" : "") + " onchange='this.value=this.checked?1:0'></div><div><label for='SHOW_HUMIDITY'>Humidity</label><input type='checkbox' name='SHOW_HUMIDITY' id='SHOW_HUMIDITY' value='" + String(SHOW_HUMIDITY) + "' " + String(SHOW_HUMIDITY ? "checked" : "") + " onchange='this.value=this.checked?1:0'></div><div><label for='SHOW_PRESSURE'>Pressure</label><input type='checkbox' name='SHOW_PRESSURE' id='SHOW_PRESSURE' value='" + String(SHOW_PRESSURE) + "' " + String(SHOW_PRESSURE ? "checked" : "") + " onchange='this.value=this.checked?1:0'></div></div>";
  html += "Show mode for X seconds: <div style='display: flex;justify-content: space-between;gap: 25px;'><input type='number' name='SHOW_DATE_SECONDS' value='" + String(SHOW_DATE_SECONDS) + "' min='1'><input type='number' name='SHOW_TEMPERATURE_SECONDS' value='" + String(SHOW_TEMPERATURE_SECONDS) + "' min='1'><input type='number' name='SHOW_HUMIDITY_SECONDS' value='" + String(SHOW_HUMIDITY_SECONDS) + "' min='1'><input type='number' name='SHOW_PRESSURE_SECONDS' value='" + String(SHOW_PRESSURE_SECONDS) + "' min='1'></div>";
  
  html += "<h2>Sensors</h2>";
  html += "<div class='checkbox-wrapper'><div><label for='USE_LDR' title='Brightness will be automatically adjusted based on the ambient light' class='tooltip'>Use LRD</label><input type='checkbox' name='USE_LDR' id='USE_LDR' value='" + String(USE_LDR) + "' " + String(USE_LDR ? "checked" : "") + " onchange='this.value=this.checked?1:0'></div><div><label for='USE_LDR_DAY'>Day</label><input type='checkbox' name='USE_LDR_DAY' id='USE_LDR_DAY' value='" + String(USE_LDR_DAY) + "' " + String(USE_LDR_DAY ? "checked" : "") + " onchange='this.value=this.checked?1:0'></div><div><label for='USE_LDR_NIGHT'>Night</label><input type='checkbox' name='USE_LDR_NIGHT' id='USE_LDR_NIGHT' value='" + String(USE_LDR_NIGHT) + "' " + String(USE_LDR_NIGHT ? "checked" : "") + " onchange='this.value=this.checked?1:0'></div></div>";
  html += "LRD: number of readings <input type='number' name='LDR_READS' value='" + String(LDR_READS) + "' min='0'><br>";
  html += "<input type='submit' value='Save'></form>";
  
  html += "<form action='/set-time' method='POST'>Set time:<div><input type='text' name='time' style='width: 80%;'><input type='number' min='-1' max='1' name='time-adjust' value='0' style='width: calc(20% - 10px);margin-left: 10px;'></div><br><input type='submit' value='Save'></form>";
  html += "<script>setInterval(function () { let currDate = new Date(); document.querySelector('[name=time]').value = `${currDate.getHours() - document.querySelector('[name=time-adjust]').value},${currDate.getMinutes()},${currDate.getSeconds()},${currDate.getDate()},${  currDate.getMonth() + 1 },${currDate.getFullYear()}`;}, 1000);function hsvToRgb(h, s, v) { function mapFloat(val, min1, max1, min2, max2) {  return min2 + ((val - min1) * (max2 - min2)) / (max1 - min1); } h = mapFloat(mapFloat(h, 0, 255, 0, 360), 0, 360, 0, 1); s = mapFloat(s, 0, 255, 0, 1); v = mapFloat(v, 0, 255, 0, 1); var r, g, b; var i = Math.floor(h * 6); var f = h * 6 - i; var p = v * (1 - s); var q = v * (1 - f * s); var t = v * (1 - (1 - f) * s); switch (i % 6) {  case 0:   (r = v), (g = t), (b = p);   break;  case 1:   (r = q), (g = v), (b = p);   break;  case 2:   (r = p), (g = v), (b = t);   break;  case 3:   (r = p), (g = q), (b = v);   break;  case 4:   (r = t), (g = p), (b = v);   break;  case 5:   (r = v), (g = p), (b = q);   break; } return [r * 255, g * 255, b * 255];}const inputs = document.querySelectorAll('input[type=number]');inputs.forEach((input) => { input.addEventListener('input', (event) => {  let dayRgb = hsvToRgb(document.querySelector('[name=DAY_COLOR]').value, document.querySelector('[name=DAY_SATUR]').value, /* document.querySelector('[name=DAY_BRIGHTNESS]').value */ 255);  let nightRgb = hsvToRgb(document.querySelector('[name=NIGHT_COLOR]').value, document.querySelector('[name=NIGHT_SATUR]').value, /* document.querySelector('[name=NIGHT_BRIGHTNESS]').value */ 255);  let dayOpacity = (document.querySelector('[name=DAY_BRIGHTNESS]').value * 100 / 255) / 100;  let nightOpacity = (document.querySelector('[name=NIGHT_BRIGHTNESS]').value * 100 / 255) / 100;  dayOpacity = dayOpacity < 0.15 ? 0.15 : dayOpacity;  nightOpacity = nightOpacity < 0.15 ? 0.15 : nightOpacity;  document.querySelector('.color-preview-day').style.backgroundColor = `rgba(${dayRgb[0]}, ${dayRgb[1]}, ${dayRgb[2]}, ${dayOpacity})`;  document.querySelector('.color-preview-night').style.backgroundColor = `rgba(${nightRgb[0]}, ${nightRgb[1]}, ${nightRgb[2]}, ${nightOpacity})`; });});inputs[0].dispatchEvent(new Event('input', { bubbles: true }));</script>";
  
  html += "<div class='footer-links'><a href='/sensors'>Sensors data</a><a href='/reboot'>Reboot ESP</a><a href='/resetwifi'>Reset WiFi creds</a></div>";

  server.send(200, "text/html", htmlTemplate(html));
}

void handleSettingsPost() {
  if (server.hasArg("DAY_BRIGHTNESS")) { DAY_BRIGHTNESS = server.arg("DAY_BRIGHTNESS").toInt(); }
  if (server.hasArg("NIGHT_BRIGHTNESS")) { NIGHT_BRIGHTNESS = server.arg("NIGHT_BRIGHTNESS").toInt(); }
  if (server.hasArg("DAY_SATUR")) { DAY_SATUR = server.arg("DAY_SATUR").toInt(); }
  if (server.hasArg("NIGHT_SATUR")) { NIGHT_SATUR = server.arg("NIGHT_SATUR").toInt(); }
  if (server.hasArg("DAY_COLOR")) { DAY_COLOR = server.arg("DAY_COLOR").toInt(); }
  if (server.hasArg("NIGHT_COLOR")) { NIGHT_COLOR = server.arg("NIGHT_COLOR").toInt(); }
  if (server.hasArg("NIGHT_START_HOUR")) { NIGHT_START_HOUR = server.arg("NIGHT_START_HOUR").toInt(); }
  if (server.hasArg("NIGHT_END_HOUR")) { NIGHT_END_HOUR = server.arg("NIGHT_END_HOUR").toInt(); }
  if (server.hasArg("LDR_READS")) { LDR_READS = server.arg("LDR_READS").toInt(); }
  if (server.hasArg("SHOW_DATE")) { SHOW_DATE = server.arg("SHOW_DATE").toInt(); } else { SHOW_DATE = 0;}
  if (server.hasArg("USE_DITHER")) { USE_DITHER = server.arg("USE_DITHER").toInt(); } else { USE_DITHER = 0;}
  if (server.hasArg("USE_LDR")) { USE_LDR = server.arg("USE_LDR").toInt(); } else { USE_LDR = 0;}
  if (server.hasArg("USE_LDR_DAY")) { USE_LDR_DAY = server.arg("USE_LDR_DAY").toInt(); } else { USE_LDR_DAY = 0;}
  if (server.hasArg("USE_LDR_NIGHT")) { USE_LDR_NIGHT = server.arg("USE_LDR_NIGHT").toInt(); } else { USE_LDR_NIGHT = 0;}
  if (server.hasArg("SHOW_TEMPERATURE")) { SHOW_TEMPERATURE = server.arg("SHOW_TEMPERATURE").toInt(); } else { SHOW_TEMPERATURE = 0;}
  if (server.hasArg("SHOW_HUMIDITY")) { SHOW_HUMIDITY = server.arg("SHOW_HUMIDITY").toInt(); } else { SHOW_HUMIDITY = 0;}
  if (server.hasArg("SHOW_PRESSURE")) { SHOW_PRESSURE = server.arg("SHOW_PRESSURE").toInt(); } else { SHOW_PRESSURE = 0;}
  if (server.hasArg("SHOW_TIME_SECONDS")) { SHOW_TIME_SECONDS = server.arg("SHOW_TIME_SECONDS").toInt(); }
  if (server.hasArg("SHOW_DATE_SECONDS")) { SHOW_DATE_SECONDS = server.arg("SHOW_DATE_SECONDS").toInt(); }
  if (server.hasArg("SHOW_TEMPERATURE_SECONDS")) { SHOW_TEMPERATURE_SECONDS = server.arg("SHOW_TEMPERATURE_SECONDS").toInt(); }
  if (server.hasArg("SHOW_HUMIDITY_SECONDS")) { SHOW_HUMIDITY_SECONDS = server.arg("SHOW_HUMIDITY_SECONDS").toInt(); }
  if (server.hasArg("SHOW_PRESSURE_SECONDS")) { SHOW_PRESSURE_SECONDS = server.arg("SHOW_PRESSURE_SECONDS").toInt(); }

  FastLED.setDither(USE_DITHER);

  saveSettings();
  // Respond and redirect
  server.send(200, "text/html", responseWithRedirect());
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
  server.send(200, "text/html", responseWithRedirect());
}

void handleSensors() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  
  String html = "<html>";
  html += "<body><h2>Sensors values</h2><br>";
  html += "<h2>LDR Analog: " + String(analogRead(LDR_A_PIN)) + "</h2>";
  html += "<h2>LDR Digital: " + String(digitalRead(LDR_A_PIN)) + "</h2>";
  html += "<h2>Temperature: " + String(temp.temperature) + " °C</h2>";
  html += "<h2>Humidity: " + String(humidity.relative_humidity) + " %</h2>";
  html += "<h2>Pressure: " + String(bmp.readPressure() / 100 * 0.75) + " mmHg ---> " + bmp.readPressure() + " Pa ---> " + String(bmp.readPressure() / 100) + " hPa</h2>";
  html += "<div style='margin-bottom: 50px;margin-top: 20px;'><a href='/'>Back to main page</a></div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleRoot() {
  String trimmed = savedSSID;
  trimmed.trim();  
  dd("savedSSID", savedSSID, "savedSSID");
  dd("savedSSID", trimmed, "savedSSID");
  if (trimmed.length() != 0) {
    server.send(200, "text/html", responseWithRedirect("", "/settings", 0));
  }
  // Simple HTML form to enter SSID and password
  String html = "<h2>WiFi Credentials</h2>";
  html += "<form action='/save-wifi-creds' method='POST'>";
  html += "SSID: <input type='text' name='ssid'><br>";
  html += "Password: <input type='password' name='pass' type='password'><br>";
  html += "<input type='submit' value='Save'>";
  html += "</form>";
  html += "<div class='footer-links'><a href='/settings'>Settings</a><a href='/sensors'>Sensors data</a><a href='/reboot'>Reboot ESP</a></div>";
  server.send(200, "text/html", htmlTemplate(html));
}

void handleSaveWifiCredsPost() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    // ssid = server.arg("ssid");
    // password = server.arg("pass");

    // Save credentials in Preferences
    prefs.begin("wifiCreds", false);
    prefs.putString("ssid", server.arg("ssid"));
    prefs.putString("pass", server.arg("pass"));
    prefs.end();

    server.send(200, "text/html", responseWithRedirect("Saved! Rebooting...", "/", 5));
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/html", "Missing fields");
  }
}

void handleReboot() {
    server.send(200, "text/html", responseWithRedirect("Rebooting... Redirect in 5...", "/settings", 5));
    delay(2000);
    ESP.restart();
}

void handleResesWifiCreds() {
    server.send(200, "text/html", responseWithRedirect("Rebooting... Connect to ESP32_Config and go to 192.168.4.1 to set a new WiFi creds", "192.168.4.1"));
    prefs.begin("wifiCreds", false);
    prefs.clear();               // clears everything in this namespace
    prefs.end();
    // ssid = "";
    // password = "";
    delay(2000);
    ESP.restart();
}
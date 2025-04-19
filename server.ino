void handleRoot() {
  String html = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><style>body,form{display:flex}body{font-family:Arial,sans-serif;background-color:#f4f4f4;color:#333;margin:0;padding:0;justify-content:center;align-items:center;flex-direction: column;}h2{margin-bottom:20px;color:#007bff}form{background:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,.1);width:300px;flex-direction:column}input[type=number],input[type=text]{margin-top: 10px;margin-bottom:5px;padding:10px;border:1px solid #ccc;border-radius:4px;font-size:16px;width:100%;box-sizing:border-box}input[type=submit]{background-color:#28a745;color:#fff;border:none;border-radius:4px;padding:12px;cursor:pointer;font-size:16px;transition:background-color .3s}input[type=submit]:hover{background-color:#218838}@media (max-width:600px){form{width:90%}}.color-preview-parent{display:flex;margin-bottom:15px}.color-preview-parent>div{display:flex;flex-direction:column;justify-content:center;align-items:center;width:50%}.color-preview{margin-top:5px;width:50px;height:50px;border:1px solid #000}</style><head>";
  html += "<body><h2>LED Clock Settings</h2>";
  html += "<form action='/set-settings' method='POST'>";
  html += "<div class='color-preview-parent'><div>Day<div class='color-preview color-preview-day'></div></div><div>Night<div class='color-preview color-preview-night'></div></div></div>";
  html += "Day color (0-255): <input type='number' name='DAY_COLOR' value='" + String(DAY_COLOR) + "' min='0' max='255'><br>";
  html += "Day saturation (0-255): <input type='number' name='DAY_SATUR' value='" + String(DAY_SATUR) + "' min='0' max='255'><br>";
  html += "Day brightness (0-255): <input type='number' name='DAY_BRIGHTNESS' value='" + String(DAY_BRIGHTNESS) + "' min='0' max='255'><br>";
  html += "Night color (0-255): <input type='number' name='NIGHT_COLOR' value='" + String(NIGHT_COLOR) + "' min='0' max='255'><br>";
  html += "Night saturation (0-255): <input type='number' name='NIGHT_SATUR' value='" + String(NIGHT_SATUR) + "' min='0' max='255'><br>";
  html += "Night brightness (0-255): <input type='number' name='NIGHT_BRIGHTNESS' value='" + String(NIGHT_BRIGHTNESS) + "' min='0' max='255'><br>";
  html += "Night start hour (0-23): <input type='number' name='NIGHT_START_HOUR' value='" + String(NIGHT_START_HOUR) + "'  min='0' max='23'><br>";
  html += "Night end hour (0-23): <input type='number' name='NIGHT_END_HOUR' value='" + String(NIGHT_END_HOUR) + "'  min='0' max='23'><br>";
  html += "<input type='submit' value='Save'></form>";
  html += "<form action='/set-time' method='POST'>Set time:<div><input type='text' name='time' style='width: 80%;'><input type='number' min='-1' max='1' name='time-adjust' value='0' style='width: calc(20% - 10px);margin-left: 10px;'></div><br><input type='submit' value='Save'></form></form>";
  html += "<script>setInterval(function() {let currDate = new Date();document.querySelector('[name=time]').value = `${currDate.getHours() - document.querySelector('[name=time-adjust]').value },${currDate.getMinutes()},${currDate.getSeconds()},${currDate.getDate()},${currDate.getMonth() + 1},${currDate.getFullYear()}`;}, 1000);function hsvToRgb(h, s, v) {      function mapFloat( val,  min1,  max1,  min2,  max2) {        return min2 + (val - min1) * (max2 - min2) / (max1 - min1);      }      h = mapFloat(mapFloat(h, 0, 255, 0, 360), 0, 360, 0, 1);      s = mapFloat(s, 0, 255, 0, 1);      v = mapFloat(v, 0, 255, 0, 1);      var r, g, b;      var i = Math.floor(h * 6);      var f = h * 6 - i;      var p = v * (1 - s);      var q = v * (1 - f * s);      var t = v * (1 - (1 - f) * s);      switch (i % 6) {        case 0: r = v, g = t, b = p; break;        case 1: r = q, g = v, b = p; break;        case 2: r = p, g = v, b = t; break;        case 3: r = p, g = q, b = v; break;        case 4: r = t, g = p, b = v; break;        case 5: r = v, g = p, b = q; break;      }      return [ r * 255, g * 255, b * 255 ];    }    const inputs = document.querySelectorAll('input[type=number]');    inputs.forEach(input => {      input.addEventListener('input', (event) => {        let dayRgb = hsvToRgb(document.querySelector('[name=DAY_COLOR]').value, document.querySelector('[name=DAY_SATUR]').value, document.querySelector('[name=DAY_BRIGHTNESS]').value);        let nightRgb = hsvToRgb(document.querySelector('[name=NIGHT_COLOR]').value, document.querySelector('[name=NIGHT_SATUR]').value, document.querySelector('[name=NIGHT_BRIGHTNESS]').value);        document.querySelector('.color-preview-day').style.backgroundColor = `rgb(${dayRgb[0]}, ${dayRgb[1]}, ${dayRgb[2]})`;        document.querySelector('.color-preview-night').style.backgroundColor = `rgb(${nightRgb[0]}, ${nightRgb[1]}, ${nightRgb[2]})`;      });    });    inputs[0].dispatchEvent(new Event('input', { bubbles: true }));</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleSetSettings() {
  if (server.hasArg("DAY_BRIGHTNESS")) { DAY_BRIGHTNESS = server.arg("DAY_BRIGHTNESS").toInt(); }
  if (server.hasArg("NIGHT_BRIGHTNESS")) { NIGHT_BRIGHTNESS = server.arg("NIGHT_BRIGHTNESS").toInt(); }
  if (server.hasArg("DAY_SATUR")) { DAY_SATUR = server.arg("DAY_SATUR").toInt(); }
  if (server.hasArg("NIGHT_SATUR")) { NIGHT_SATUR = server.arg("NIGHT_SATUR").toInt(); }
  if (server.hasArg("DAY_COLOR")) { DAY_COLOR = server.arg("DAY_COLOR").toInt(); }
  if (server.hasArg("NIGHT_COLOR")) { NIGHT_COLOR = server.arg("NIGHT_COLOR").toInt(); }
  if (server.hasArg("NIGHT_START_HOUR")) { NIGHT_START_HOUR = server.arg("NIGHT_START_HOUR").toInt(); }
  if (server.hasArg("NIGHT_END_HOUR")) { NIGHT_END_HOUR = server.arg("NIGHT_END_HOUR").toInt(); }

  saveSettings();

  // Respond and redirect
  server.send(200, "text/html", successResponse());
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
  server.send(200, "text/html", successResponse());
}

String successResponse() {
  return "<html><body><script>alert('Settings saved successfully!');</script><meta http-equiv='refresh' content='0; url=/' /></body></html>";
}
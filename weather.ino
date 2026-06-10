// weather.ino — OpenWeatherMap fetch + icon display for WordClock

#include <HTTPClient.h>
#include "weather_icons.h"

// Icons fill the full 8x16 logical grid — DrawPixel lights both LEDs of each pair.

// Map a color index to an RGB color
static uint32_t weatherColor(uint8_t idx) {
  switch (idx) {
    case 3: return strip.Color(255, 200,   0);  // sun yellow
    case 4: return strip.Color(  0, 100, 255);  // rain blue
    case 5: return strip.Color( 90, 100, 120);  // crater dark gray
    case 6: return strip.Color(200, 220, 255);  // moon white-blue
    case 7: return strip.Color(180, 210, 255);  // snow light-blue
    case 9: return strip.Color(180, 180, 180);  // cloud gray
    case 1: return strip.Color(255, 255, 100);  // lightning
    default: return 0;
  }
}

static void renderWeatherIcon(const uint8_t icon[8][16]) {
  ClearDisplay();
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 16; c++) {
      uint8_t idx = pgm_read_byte(&icon[r][c]);
      if (idx > 0) DrawPixel(r + 1, c + 1, weatherColor(idx));
    }
  }
  showStrip();
}

void showWeatherIcon() {
  const uint8_t (*icon)[16] = nullptr;
  String code = weatherIconCode;
  bool isNight = code.endsWith("n");
  code.replace("d", ""); code.replace("n", "");

  if      (code == "01") icon = isNight ? WI_CLEAR_NIGHT  : WI_CLEAR;
  else if (code == "02") icon = isNight ? WI_PARTLY_NIGHT : WI_PARTLY;
  else if (code == "03" || code == "04") icon = WI_CLOUD;
  else if (code == "09") icon = WI_SHOWERRAIN;
  else if (code == "10") icon = isNight ? WI_RAIN_NIGHT : WI_RAIN;
  else if (code == "11") icon = WI_THUNDER;
  else if (code == "13") icon = WI_SNOW;
  else if (code == "50") icon = WI_MIST;

  if (icon) renderWeatherIcon(icon);
}

void fetchWeather() {
  if (weatherApiKey.length() == 0 || weatherLat.length() == 0 || weatherLon.length() == 0) return;

  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + weatherLat
             + "&lon=" + weatherLon + "&appid=" + weatherApiKey + "&units=metric";
  http.begin(url);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    int idx = body.indexOf("\"icon\":\"");
    if (idx >= 0) {
      weatherIconCode = body.substring(idx + 8, idx + 11);
    }
    int dIdx = body.indexOf("\"description\":\"");
    if (dIdx >= 0) {
      int s = dIdx + 15, e = body.indexOf("\"", s);
      weatherDesc = body.substring(s, e);
    }
    int tIdx = body.indexOf("\"temp\":");
    if (tIdx >= 0) {
      int s = tIdx + 7, e = body.indexOf(",", s);
      weatherTemp = body.substring(s, e);
    }
    int nIdx = body.indexOf("\"name\":\"");
    if (nIdx >= 0) {
      int s = nIdx + 8, e = body.indexOf("\"", s);
      weatherCity = body.substring(s, e);
    }
    Serial.println("Weather: " + weatherCity + " — " + weatherDesc + " " + weatherTemp + "°C icon=" + weatherIconCode);
  } else {
    Serial.println("Weather fetch failed, HTTP " + String(code));
  }
  http.end();
  lastWeatherFetch = millis();
}

void weatherLoop() {
  if (UseOnlineMode != 1 || !WiFIsetup) return;

  unsigned long now = millis();

  // Stop showing weather after duration expires (test always 5 seconds)
  if (weatherShowing) {
    unsigned long dur = weatherTestActive ? 5000UL : (unsigned long)weatherDuration * 1000UL;
    if (now - weatherShowStart >= dur) {
      weatherShowing    = false;
      weatherTestActive = false;
      updatenow = true;
    }
    return;
  }

  // Fetch new weather data once per hour
  if (weatherApiKey.length() > 0) {
    if (lastWeatherFetch == 0 || now - lastWeatherFetch > 3600000UL) {
      fetchWeather();
    }
  }

  // Show icon at configured interval
  if (weatherIconCode.length() > 0 && weatherInterval > 0) {
    if (lastWeatherShow == 0 || now - lastWeatherShow >= (unsigned long)weatherInterval * 60000UL) {
      showWeatherIcon();
      weatherShowing  = true;
      weatherShowStart = now;
      lastWeatherShow  = now;
    }
  }
}

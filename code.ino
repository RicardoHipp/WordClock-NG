// ###########################################################################################################################################
// #
// # WordClock code for the printables.com AWSW WordClock 16x8 LED matrix projects of 2023/2024:
// # https://www.printables.com/de/search/models?q=WordClock%2016x8%20@awsw&ctx=models
// #
// # Code by https://github.com/AWSW-de
// #
// # Released under licenses:
// # GNU General Public License v3.0: https://github.com/AWSW-de/WordClock-16x8-LED-matrix/blob/main/LICENSE and
// # Creative Commons Attribution-NonCommercial 3.0 Unported License http://creativecommons.org/licenses/by-nc/3.0
// # Important: NonCommercial — You may not use the material for commercial purposes !
// #
// ###########################################################################################################################################
/*

      ___           ___           ___           ___           ___           ___       ___           ___           ___     
     /\__\         /\  \         /\  \         /\  \         /\  \         /\__\     /\  \         /\  \         /\__\    
    /:/ _/_       /::\  \       /::\  \       /::\  \       /::\  \       /:/  /    /::\  \       /::\  \       /:/  /    
   /:/ /\__\     /:/\:\  \     /:/\:\  \     /:/\:\  \     /:/\:\  \     /:/  /    /:/\:\  \     /:/\:\  \     /:/__/     
  /:/ /:/ _/_   /:/  \:\  \   /::\~\:\  \   /:/  \:\__\   /:/  \:\  \   /:/  /    /:/  \:\  \   /:/  \:\  \   /::\__\____ 
 /:/_/:/ /\__\ /:/__/ \:\__\ /:/\:\ \:\__\ /:/__/ \:|__| /:/__/ \:\__\ /:/__/    /:/__/ \:\__\ /:/__/ \:\__\ /:/\:::::\__\
 \:\/:/ /:/  / \:\  \ /:/  / \/_|::\/:/  / \:\  \ /:/  / \:\  \  \/__/ \:\  \    \:\  \ /:/  / \:\  \  \/__/ \/_|:|~~|~   
  \::/_/:/  /   \:\  /:/  /     |:|::/  /   \:\  /:/  /   \:\  \        \:\  \    \:\  /:/  /   \:\  \          |:|  |    
   \:\/:/  /     \:\/:/  /      |:|\/__/     \:\/:/  /     \:\  \        \:\  \    \:\/:/  /     \:\  \         |:|  |    
    \::/  /       \::/  /       |:|  |        \::/__/       \:\__\        \:\__\    \::/  /       \:\__\        |:|  |    
     \/__/         \/__/         \|__|         ~~            \/__/         \/__/     \/__/         \/__/         \|__|    

*/


// ###########################################################################################################################################
// # Includes:
// #
// # You will need to add the following libraries to your Arduino IDE to use the project:
// # - Adafruit NeoPixel      // by Adafruit:                     https://github.com/adafruit/Adafruit_NeoPixel
// # - AsyncTCP               // by me-no-dev:                    https://github.com/me-no-dev/AsyncTCP
// # - ESPAsyncWebServer      // by me-no-dev:                    https://github.com/me-no-dev/ESPAsyncWebServer
// # - ESPUI                  // by s00500:                       https://github.com/s00500/ESPUI/archive/refs/tags/2.2.3.zip
// # - ArduinoJson            // by bblanchon:                    https://github.com/bblanchon/ArduinoJson
// # - LITTLEFS               // by lorol:                        https://github.com/lorol/LITTLEFS
// # - ESP32Time              // by fbiego:                       https://github.com/fbiego/ESP32Time
// #
// ###########################################################################################################################################
#include <WiFi.h>               // Used to connect the ESP32 to your WiFi
#include <WebServer.h>          // ESP32 OTA update function
#include <Update.h>             // ESP32 OTA update function
#include <Adafruit_NeoPixel.h>  // Used to drive the NeoPixel LEDs
#include "time.h"               // Used for NTP time requests
#include <AsyncTCP.h>           // Used for the internal web server
#include <ESPAsyncWebServer.h>  // Used for the internal web server
#include <DNSServer.h>          // Used for the internal web server
#include "webui.h"               // Custom web UI (replaces ESPUI)
#include "esp_log.h"            // Disable WiFi debug warnings
#include <Preferences.h>        // Used to save the configuration to the ESP32 flash
#include <ESP32Time.h>          // Used for the Offline Mode ESP32 time function
#include "settings.h"           // Settings are stored in a seperate file to make to code better readable and to be able to switch to other settings faster
#include <Wire.h>               // I2C bus (optional DS3231 RTC)
#include <RTClib.h>             // Optional DS3231 RTC for improved offline accuracy
#include <esp_sntp.h>           // SNTP sync status (daily NTP resync)
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "esp_mac.h"

RTC_DS3231 ds3231;
bool ds3231Found = false;
bool ds3231SyncEnabled = true;     // Kann über die WebUI pausiert werden, um Drift-Tests durchzuführen

void showStrip() {
  strip.show();
}

extern bool bleActive;
void showStripSafe() {
  if (bleActive) BLEDevice::getAdvertising()->stop();
  strip.show();
  if (bleActive) BLEDevice::getAdvertising()->start();
}


// ###########################################################################################################################################
// # Forward declarations:
// ###########################################################################################################################################
void WIFI_SETUP();
void OfflinePotalSetup();
void setupOTAupate();
void callStartText();
void ShowOfflineIPaddress();
void showtext(char letter, int wait, uint32_t c);
String getValue(String data, char separator, int index);
void DrawLineX(int startrow, int startcolum, int endrow, int endcolum, uint32_t c1, uint32_t c2);
void checkforNightMode();
void setFlashValues();
void ResetTextLEDs(uint32_t c);
void handleLEDupdate();
void parseHexColor(String hex, int &r, int &g, int &b);
void weatherLoop();
void fetchWeather();
void showWeatherIcon();
void animationStart();
void animationTick();
void showStripSafe();
void showNtpSyncLED(uint32_t color);


// ###########################################################################################################################################
// # Version number of the code:
// ###########################################################################################################################################
const char* WORD_CLOCK_VERSION = "1.3.0";


// ###########################################################################################################################################
// # Internal web server settings:
// ###########################################################################################################################################
AsyncWebServer server(80);       // Web server for config
WebServer otaserver(8080);       // Web OTA ESP32 update server
AsyncWebServer ledserver(2023);  // Web server for HTML commands
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;


// ###########################################################################################################################################
// # Declartions and variables used in the functions:
// ###########################################################################################################################################
Preferences preferences;  // ESP32 flash storage
ESP32Time rtc;            // Setup Offline ESP32 time function
bool updatedevice = true;
bool updatenow = false;
bool updatemode = false;
bool changedvalues = false;
bool WiFIsetup = false;
unsigned long pendingRestartAt = 0;  // millis()-Zeitstempel für geplanten Neustart
int wifiTimeout = 0;
bool wifiShutDown = false;
unsigned long wifiTimerStart = 0;
unsigned long lastWebServerActivity = 0;
bool webSessionActive = false;
bool webSessionSilent = false;
bool bleActive = false;
bool triggerWifiRestore = false;
BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic *pCharacteristic = NULL;
int iHour, iMinute, iSecond, iDay, iMonth, iYear, intensity, intensity_day, intensity_night, set_web_intensity, intensity_web;
int redVal_back, greenVal_back, blueVal_back, redVal_time, greenVal_time, blueVal_time;
#define langLEDlayout LANGUAGE  // preprocessor constant — compiler eliminates all other language blocks
int usenightmode, useshowip, usesinglemin, useStartupText, RandomColor;
int UseOnlineMode, showOMhints, ntpSyncHour, ntpSyncMinute;
bool nightModeWarning = false;
bool ntpSyncing = false;
String iStartTime, myTimeZone, myTimeServer, Timezone, NTPserver, day_time_start, day_time_stop, statusNightModeIDtxt;

// Animation
#define ANIM_NONE     0
#define ANIM_MATRIX   1
#define ANIM_TELETYPE 2
#define ANIM_FADE     3
#define ANIM_SQUEEZE  4
#define ANIM_SNAKE    5
#define ANIM_FLICKER  6
int  animationMode    = ANIM_NONE;
bool animationRunning = false;

// Weather
String weatherApiKey = "", weatherLat = "", weatherLon = "";
int weatherInterval_default = 30, weatherDuration_default = 5;
int weatherInterval, weatherDuration;
bool weatherShowing = false;
bool weatherTestActive = false;
unsigned long weatherShowStart = 0, lastWeatherFetch = 0;
unsigned long lastWeatherShow = 1;  // non-zero prevents immediate show on startup
unsigned long wifiConnectDeadline = 0;  // captive portal connection timeout
String weatherIconCode = "";
String weatherCity = "", weatherDesc = "", weatherTemp = "";


// ###########################################################################################################################################
// # Helper: find next DST transition date/time and type (Sommerzeit/Winterzeit)
// ###########################################################################################################################################
static String computeNextDstTransition() {
  setenv("TZ", Timezone.c_str(), 1); tzset();
  time_t now; time(&now);
  struct tm curTm; localtime_r(&now, &curTm);
  int curDst = curTm.tm_isdst;
  if (curDst < 0) return "unbekannt";

  // Stündlich vorwärts suchen bis DST wechselt (max. ~6 Monate = ~4380h)
  time_t trans = 0;
  for (long h = 1; h <= 366 * 24; h++) {
    time_t check = now + h * 3600L;
    struct tm chk; localtime_r(&check, &chk);
    if (chk.tm_isdst != curDst) { trans = check; break; }
  }
  if (trans == 0) return "unbekannt";

  time_t transBefore = trans - 3600;
  struct tm tb; localtime_r(&transBefore, &tb);
  struct tm ta; localtime_r(&trans, &ta);
  const char* typ = (ta.tm_isdst > 0) ? "Sommerzeit" : "Winterzeit";
  char buf[48];
  snprintf(buf, sizeof(buf), "%s: %02d.%02d.%04d %02d:00 \xe2\x86\x92 %02d:00",
           typ, tb.tm_mday, tb.tm_mon+1, tb.tm_year+1900, tb.tm_hour+1, ta.tm_hour);
  return String(buf);
}

static bool dstCacheInvalid = false;

String nextDstTransition() {
  static String cached = "";
  static unsigned long lastCalc = 0;
  if (cached.length() == 0 || dstCacheInvalid || millis() - lastCalc > 3600000UL) {
    cached = computeNextDstTransition();
    lastCalc = millis();
    dstCacheInvalid = false;
  }
  return cached;
}

void invalidateDstCache() {
  dstCacheInvalid = true;
}


// ###########################################################################################################################################
// # Helper: set ESP32 system clock from DS3231 (UTC stored in DS3231, TZ handles DST)
// ###########################################################################################################################################
static time_t utcToEpoch(struct tm *t) {
  static const int md[] = {0,31,59,90,120,151,181,212,243,273,304,334};
  int y = t->tm_year + 1900;
  int m = t->tm_mon + 1;
  long d = (y-1970)*365L + (y-1969)/4 - (y-1901)/100 + (y-1601)/400;
  d += md[m-1] + t->tm_mday - 1;
  if (m > 2 && (y%4==0 && (y%100!=0 || y%400==0))) d++;
  return d * 86400L + t->tm_hour * 3600L + t->tm_min * 60L + t->tm_sec;
}

void ds3231SyncSystemClock() {
  DateTime rtcUtc = ds3231.now();
  struct tm t = {};
  t.tm_year  = rtcUtc.year() - 1900;
  t.tm_mon   = rtcUtc.month() - 1;
  t.tm_mday  = rtcUtc.day();
  t.tm_hour  = rtcUtc.hour();
  t.tm_min   = rtcUtc.minute();
  t.tm_sec   = rtcUtc.second();
  t.tm_isdst = 0;
  struct timeval tv = { utcToEpoch(&t), 0 };
  settimeofday(&tv, NULL);
}

#define BLE_SERVICE_UUID        "4fa87109-9029-4cc9-b1b9-50c724722333"
#define BLE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyBleCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        if (value == "1" || value == "WLAN_ON") {
            Serial.println("# BLE-Befehl empfangen: WLAN einschalten!");
            triggerWifiRestore = true;
        }
    }
};
static MyBleCallbacks bleCallbacks;

String getMacAddress() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char macStr[13];
  snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

void startBLE() {
  if (bleActive) return;
  
  // Eindeutigen Namen aus der MAC-Adresse generieren (z. B. "WordClock BLE - 8C2F")
  String mac = getMacAddress();
  String bleName = "WordClock BLE - " + mac.substring(8, 12);
  
  Serial.println("# Starte BLE unter dem Namen: " + bleName);
  BLEDevice::init(bleName.c_str());
  pServer = BLEDevice::createServer();
  pService = pServer->createService(BLE_SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      BLE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic->setCallbacks(&bleCallbacks);
  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->start();
  bleActive = true;
}

void stopBLE() {
  if (!bleActive) return;
  Serial.println("# Stoppe BLE...");
  BLEDevice::getAdvertising()->stop();
  BLEDevice::deinit(true);
  bleActive = false;
  pServer = NULL;
  pService = NULL;
  pCharacteristic = NULL;
}


static void wifiEventLog(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("# [WiFi] AP gestartet  IP=" + WiFi.softAPIP().toString() + "  Kanal=" + String(WiFi.channel()));
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
      Serial.println("# [WiFi] AP gestoppt");
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.printf("# [WiFi] Client verbunden    MAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
        info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1],
        info.wifi_ap_staconnected.mac[2], info.wifi_ap_staconnected.mac[3],
        info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5]);
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.printf("# [WiFi] Client getrennt     MAC=%02X:%02X:%02X:%02X:%02X:%02X  Grund=%d\n",
        info.wifi_ap_stadisconnected.mac[0], info.wifi_ap_stadisconnected.mac[1],
        info.wifi_ap_stadisconnected.mac[2], info.wifi_ap_stadisconnected.mac[3],
        info.wifi_ap_stadisconnected.mac[4], info.wifi_ap_stadisconnected.mac[5],
        info.wifi_ap_stadisconnected.reason);
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("# [WiFi] STA verbunden mit Router");
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("# [WiFi] STA getrennt  Grund=" + String(info.wifi_sta_disconnected.reason));
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("# [WiFi] IP erhalten: " + WiFi.localIP().toString());
      break;
    default: break;
  }
}


// ###########################################################################################################################################
// # Setup function that runs once at startup of the ESP:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("######################################################################");
  Serial.println("# WordClock startup of version: " + String(WORD_CLOCK_VERSION));
  Serial.println("######################################################################");
  WiFi.persistent(false);
  WiFi.onEvent(wifiEventLog);

  preferences.begin("wordclock", false);         // Init ESP32 flash
  getFlashValues();                              // Read settings from flash
  strip.begin();                                 // Init the LEDs
  showStrip();                                  // Init the LEDs --> Set them to OFF
  Wire.begin(DS3231_SDA, DS3231_SCL);
  if (ds3231.begin()) {
    ds3231Found = true;
    Serial.println("# DS3231 RTC found — using hardware clock for offline mode");
  } else {
    Serial.println("# DS3231 RTC not found — using ESP32 internal clock");
  }
  intensity = intensity_day;                     // Set the intenity to day mode for startup
  strip.setBrightness(intensity);                // Set LED brightness
  if (UseOnlineMode == 1) WIFI_SETUP();          // ONLINE MODE WiFi login and startup of web services
  if (UseOnlineMode == 0) {                      // OFFLINE MODE
    setenv("TZ", Timezone.c_str(), 1); tzset();  // DST-aware timezone for offline mode
    if (ds3231Found) {
      ds3231SyncSystemClock();                   // Set system clock from DS3231 UTC
      struct tm local; getLocalTime(&local);
      iHour = local.tm_hour; iMinute = local.tm_min; iSecond = local.tm_sec;
    } else {
      iHour = 9;                                 // Default hour in Offline Mode
      iMinute = 41;                              // Default minute in Offline Mode
      rtc.setTime(0, iMinute, iHour, 1, 1, 2024);
    }
    OfflinePotalSetup();                         // Offline mode setup access point
    wifiTimerStart = millis();
    updatenow = true;                            // Update the display 1x after startup
    update_display();                            // Update LED display
    Serial.println("######################################################################");
    Serial.println("# WordClock startup in OFFLINE MODE finished...");
    Serial.println("######################################################################");
  }
}


// ###########################################################################################################################################
// # Loop function which runs all the time after the startup was done:
// ###########################################################################################################################################
void loop() {
  if (pendingRestartAt > 0 && millis() > pendingRestartAt) {
    pendingRestartAt = 0;
    ESP.restart();
  }
  // WiFi timeout shutdown logic (Offline Mode only):
  if (UseOnlineMode == 0 && wifiTimeout > 0 && !wifiShutDown) {
    if (lastWebServerActivity > 0 && millis() - lastWebServerActivity < 5000UL) {
      wifiTimerStart = millis(); // Setzt den Timer zurück, solange die Webseite aktiv offen ist
      if (!webSessionActive) {
        webSessionActive = true;
        Serial.println("# Web-Verbindung aktiv. WLAN-Timeout wird pausiert.");
      }
      if (webSessionSilent) {
        webSessionSilent = false;
        Serial.println("# Web-Verbindung reaktiviert.");
      }
    } else if (lastWebServerActivity > 0 && millis() - lastWebServerActivity >= 5000UL && millis() - lastWebServerActivity < 30000UL) {
      wifiTimerStart = millis(); // Hält den Hotspot während der Karenzzeit noch aktiv
      if (webSessionActive && !webSessionSilent) {
        webSessionSilent = true;
        Serial.println("# Web-Verbindung inaktiv (5 Sek. Funkstille). Starte 30 Sek. Karenzzeit...");
      }
    } else {
      if (webSessionActive) {
        webSessionActive = false;
        webSessionSilent = false;
        Serial.println("# Web-Verbindung getrennt (30 Sek. Inaktivität überschritten). WLAN-Timeout läuft wieder.");
      }
      if (millis() - wifiTimerStart > (unsigned long)wifiTimeout * 60000UL) {
        Serial.println("Offline WiFi timeout reached - shutting down SoftAP...");
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        wifiShutDown = true;
        startBLE();
      }
    }
  }
  // BLE-gesteuerte WLAN-Reaktivierung:
  if (triggerWifiRestore) {
    triggerWifiRestore = false;
    stopBLE();
    wifiShutDown = false;
    Serial.println("# Starte Offline WiFi SoftAP via BLE-Befehl neu...");
    OfflinePotalSetup(); 
    updatenow = true;
    update_display();
  }
  if (UseOnlineMode == 1) {  // Online Mode actions:
    if ((WiFIsetup == true) || (testTime == 1)) {
      printLocalTime();                               // Locally get the time (NTP server requests done 1x per hour)
      if (updatedevice == true && !weatherShowing && !animationRunning) {   // Allow display updates (normal usage)
        if (changedvalues == true) setFlashValues();  // Write settings to flash
        update_display();                             // Update display (1x per minute regulary)
      }
      if (updatemode == true) otaserver.handleClient();  // ESP32 OTA update
    }
  } else {                                               // Offline Mode actions:
    struct tm timeinfo;
    if (ds3231Found) {
      getLocalTime(&timeinfo);                           // DST-aware via TZ env var
    } else {
      timeinfo = rtc.getTimeStruct();
    }
    if (debugtexts == 1) Serial.println(rtc.getTime());
    iHour = timeinfo.tm_hour;
    iMinute = timeinfo.tm_min;
    iSecond = timeinfo.tm_sec;
    if (ds3231SyncEnabled && iMinute % 5 == 0 && iSecond == 30) {
      Wire.beginTransmission(0x68);
      ds3231Found = (Wire.endTransmission() == 0);
      if (ds3231Found) ds3231SyncSystemClock();          // Resync system clock from DS3231 UTC
    }
    if (updatedevice == true && !weatherShowing && !animationRunning) {
      if (changedvalues == true) setFlashValues();  // Write settings to flash
      update_display();
    }
    if (updatemode == true) otaserver.handleClient();  // OTA auch im Offline-Modus
    if (!animationRunning && !weatherShowing) delay(1000);
  }
  // Captive portal connection watchdog
  if (wifiConnectDeadline > 0) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnectDeadline = 0;
    } else if (millis() > wifiConnectDeadline) {
      wifiConnectDeadline = 0;
      WiFi.disconnect(true);
      Serial.println("WiFi connection timeout — stopped after 60s");
    } else {
      static unsigned long lastConnLog = 0;
      if (millis() - lastConnLog >= 2000) {
        lastConnLog = millis();
        int remaining = (wifiConnectDeadline - millis()) / 1000;
        Serial.println("Connecting... " + String(remaining) + "s remaining | status: " + String(WiFi.status()));
      }
    }
  }
  animationTick();
  weatherLoop();
  dnsServer.processNextRequest();  // Update the web server
}


// ###########################################################################################################################################
// # Helper: parse hex color string #RRGGBB into r,g,b components
// ###########################################################################################################################################
void parseHexColor(String hex, int &r, int &g, int &b) {
  if (hex.length() < 7) return;
  r = strtol(hex.substring(1, 3).c_str(), NULL, 16);
  g = strtol(hex.substring(3, 5).c_str(), NULL, 16);
  b = strtol(hex.substring(5, 7).c_str(), NULL, 16);
}

// ###########################################################################################################################################
// # Setup the internal web server configuration page:
// ###########################################################################################################################################
void setupWebInterface() {
  dnsServer.start(DNS_PORT, "*", apIP);

  // Serve main UI
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", WEBUI_HTML);
  });

  // API: get all current values as JSON
  server.on("/api/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    lastWebServerActivity = millis();
    char hex_time[8], hex_back[8];
    sprintf(hex_time, "#%02X%02X%02X", redVal_time, greenVal_time, blueVal_time);
    sprintf(hex_back, "#%02X%02X%02X", redVal_back, greenVal_back, blueVal_back);
    String ip = (UseOnlineMode == 1) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    String json = "{";
    json += "\"version\":\"" + String(WORD_CLOCK_VERSION) + "\",";
    json += "\"online_mode\":" + String(UseOnlineMode) + ",";
    json += "\"color_time\":\"" + String(hex_time) + "\",";
    json += "\"color_back\":\"" + String(hex_back) + "\",";
    json += "\"random_color\":" + String(RandomColor) + ",";
    json += "\"single_min\":" + String(usesinglemin) + ",";
    json += "\"night_mode\":" + String(usenightmode) + ",";
    json += "\"brightness_day\":" + String(intensity_day) + ",";
    json += "\"brightness_night\":" + String(intensity_night) + ",";
    json += "\"day_start\":\"" + day_time_start + "\",";
    json += "\"day_stop\":\"" + day_time_stop + "\",";
    json += "\"startup_text\":" + String(useStartupText) + ",";
    json += "\"show_ip\":" + String(useshowip) + ",";
    json += "\"language\":" + String(LANGUAGE) + ",";
    const char* langNames[] = {"Deutsch","Englisch","Niederlaendisch","Schwedisch","Italienisch","Franzoesisch","Schweizerdeutsch","Chinesisch","Schwaebisch","Bayrisch","Luxemburgisch","Ostdeutsch"};
    json += "\"lang_name\":\"" + String(langNames[LANGUAGE]) + "\",";
    json += "\"ntp_server\":\"" + myTimeServer + "\",";
    json += "\"timezone\":\"" + myTimeZone + "\",";
    json += "\"ssid\":\"" + WiFi.SSID() + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"ip\":\"" + ip + "\",";
    json += "\"mac\":\"" + WiFi.macAddress() + "\",";
    json += "\"hostname\":\"" + String(WiFi.getHostname()) + "\",";
    json += "\"dns\":\"" + WiFi.dnsIP().toString() + "\",";
    json += "\"gateway\":\"" + WiFi.gatewayIP().toString() + "\",";
    json += "\"startup_time\":\"" + iStartTime + "\",";
    json += "\"night_status\":\"" + statusNightModeIDtxt + "\",";
    json += "\"web_control\":" + String(set_web_intensity) + ",";
    json += "\"night_warn\":" + String(nightModeWarning ? 1 : 0) + ",";
    json += "\"show_om_hints\":" + String(showOMhints) + ",";
    json += "\"weather_key\":\"" + weatherApiKey + "\",";
    json += "\"weather_lat\":\"" + weatherLat + "\",";
    json += "\"weather_lon\":\"" + weatherLon + "\",";
    json += "\"weather_interval\":" + String(weatherInterval) + ",";
    json += "\"weather_duration\":" + String(weatherDuration) + ",";
    json += "\"weather_city\":\"" + weatherCity + "\",";
    json += "\"weather_desc\":\"" + weatherDesc + "\",";
    json += "\"weather_temp\":\"" + weatherTemp + "\",";
    json += "\"weather_icon\":\"" + weatherIconCode + "\",";
    json += "\"animation_mode\":" + String(animationMode) + ",";
    json += "\"wifi_timeout\":" + String(wifiTimeout) + ",";
    char timeBuf[9];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", iHour, iMinute, iSecond);
    json += "\"current_time\":\"" + String(timeBuf) + "\",";
    float chipTemp = temperatureRead();
    json += "\"chip_temp\":" + String(chipTemp, 1) + ",";
    json += "\"ds3231_found\":" + String(ds3231Found ? 1 : 0) + ",";
    json += "\"ntp_sync_hour\":" + String(ntpSyncHour) + ",";
    json += "\"ntp_sync_minute\":" + String(ntpSyncMinute) + ",";
    json += "\"dst_next\":\"" + nextDstTransition() + "\"";
    json += "}";
    request->send(200, "application/json", json);
  });

  // API: set a value
  server.on("/api/set", HTTP_GET, [](AsyncWebServerRequest* request) {
    lastWebServerActivity = millis();
    bool needsRestart = false;

    if (request->hasParam("color_time")) {
      parseHexColor(request->getParam("color_time")->value(), redVal_time, greenVal_time, blueVal_time);
      changedvalues = true; updatenow = true;
    }
    if (request->hasParam("color_back")) {
      parseHexColor(request->getParam("color_back")->value(), redVal_back, greenVal_back, blueVal_back);
      changedvalues = true; updatenow = true;
    }
    if (request->hasParam("random_color")) {
      RandomColor = request->getParam("random_color")->value().toInt();
      changedvalues = true; updatenow = true;
    }
    if (request->hasParam("single_min")) {
      usesinglemin = request->getParam("single_min")->value().toInt();
      changedvalues = true; updatenow = true;
    }
    if (request->hasParam("night_mode")) {
      usenightmode = request->getParam("night_mode")->value().toInt();
      changedvalues = true; checkforNightMode(); updatenow = true;
    }
    if (request->hasParam("brightness_day")) {
      intensity_day = request->getParam("brightness_day")->value().toInt();
      if (set_web_intensity == 0) strip.setBrightness(intensity_day);
      changedvalues = true; updatenow = true;
    }
    if (request->hasParam("brightness_night")) {
      intensity_night = request->getParam("brightness_night")->value().toInt();
      changedvalues = true; updatenow = true;
    }
    if (request->hasParam("day_start")) {
      day_time_start = request->getParam("day_start")->value();
      changedvalues = true; checkforNightMode(); updatenow = true;
    }
    if (request->hasParam("day_stop")) {
      day_time_stop = request->getParam("day_stop")->value();
      changedvalues = true; checkforNightMode(); updatenow = true;
    }
    if (request->hasParam("startup_text")) {
      useStartupText = request->getParam("startup_text")->value().toInt();
      changedvalues = true;
    }
    if (request->hasParam("show_ip")) {
      useshowip = request->getParam("show_ip")->value().toInt();
      changedvalues = true;
    }
    if (request->hasParam("show_om_hints")) {
      showOMhints = request->getParam("show_om_hints")->value().toInt();
      changedvalues = true;
    }
    if (request->hasParam("weather_key")) {
      weatherApiKey = request->getParam("weather_key")->value();
      changedvalues = true;
    }
    if (request->hasParam("weather_lat")) {
      weatherLat = request->getParam("weather_lat")->value();
      changedvalues = true;
    }
    if (request->hasParam("weather_lon")) {
      weatherLon = request->getParam("weather_lon")->value();
      changedvalues = true;
    }
    if (request->hasParam("weather_interval")) {
      weatherInterval = max(1, (int)request->getParam("weather_interval")->value().toInt());
      changedvalues = true;
    }
    if (request->hasParam("weather_duration")) {
      weatherDuration = max(3, (int)request->getParam("weather_duration")->value().toInt());
      changedvalues = true;
    }
    if (request->hasParam("animation_mode")) {
      animationMode = request->getParam("animation_mode")->value().toInt();
      changedvalues = true;
    }
    if (request->hasParam("wifi_timeout")) {
      wifiTimeout = request->getParam("wifi_timeout")->value().toInt();
      changedvalues = true;
      Serial.print("# WiFi Timeout aktualisiert auf: ");
      Serial.print(wifiTimeout);
      Serial.println(" Min.");
    }
    if (request->hasParam("ntp_sync_hour")) {
      int h = request->getParam("ntp_sync_hour")->value().toInt();
      if (h >= 0 && h <= 23) { ntpSyncHour = h; changedvalues = true; }
    }
    if (request->hasParam("ntp_sync_minute")) {
      int m = request->getParam("ntp_sync_minute")->value().toInt();
      if (m >= 0 && m <= 59) { ntpSyncMinute = m; changedvalues = true; }
    }
    if (request->hasParam("ntp_server")) {
      myTimeServer = request->getParam("ntp_server")->value();
      NTPserver = myTimeServer;
      changedvalues = true; needsRestart = true;
    }
    if (request->hasParam("timezone")) {
      myTimeZone = request->getParam("timezone")->value();
      Timezone = myTimeZone;
      changedvalues = true; needsRestart = true;
    }
    if (request->hasParam("online_mode")) {
      UseOnlineMode = request->getParam("online_mode")->value().toInt();
      changedvalues = true; needsRestart = true;
    }
    if (request->hasParam("device_time")) {
      // Sync time from browser: local ISO timestamp YYYY-MM-DDTHH:MM:SS.000Z
      String ts = request->getParam("device_time")->value();
      struct tm localTm = {};
      localTm.tm_year  = ts.substring(0, 4).toInt() - 1900;
      localTm.tm_mon   = ts.substring(5, 7).toInt() - 1;
      localTm.tm_mday  = ts.substring(8, 10).toInt();
      localTm.tm_hour  = ts.substring(11, 13).toInt();
      localTm.tm_min   = ts.substring(14, 16).toInt();
      localTm.tm_sec   = ts.substring(17, 19).toInt();
      localTm.tm_isdst = -1;                             // let mktime detect DST
      time_t epoch = mktime(&localTm);                   // local → UTC epoch (TZ must be set)
      struct timeval tv = { epoch, 0 };
      settimeofday(&tv, NULL);                           // set system clock
      if (ds3231Found) {
        struct tm utcTm; gmtime_r(&epoch, &utcTm);       // epoch → UTC struct
        ds3231.adjust(DateTime(utcTm.tm_year + 1900, utcTm.tm_mon + 1, utcTm.tm_mday,
                               utcTm.tm_hour, utcTm.tm_min, utcTm.tm_sec));
      }
      struct tm local; getLocalTime(&local);
      iHour = local.tm_hour; iMinute = local.tm_min; iSecond = local.tm_sec;
      checkforNightMode();
      invalidateDstCache();
      updatenow = true;
    }

    // changedvalues=true → main loop calls setFlashValues() (thread-safe on ESP32-C3)
    request->send(200, "application/json", needsRestart ? "{\"ok\":true,\"restart\":true}" : "{\"ok\":true}");
    if (needsRestart) { setFlashValues(); pendingRestartAt = millis() + 500; }
  });

  // API: lightweight time-only endpoint, polled every second by the web UI
  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest* request) {
    lastWebServerActivity = millis();
    Wire.beginTransmission(0x68);
    ds3231Found = (Wire.endTransmission() == 0);
    // UTC offset from system clock + timezone (independent of DS3231 content)
    time_t nowEpoch; time(&nowEpoch);
    struct tm localTm, utcTm;
    localtime_r(&nowEpoch, &localTm);
    gmtime_r(&nowEpoch, &utcTm);
    int offset = localTm.tm_hour - utcTm.tm_hour;
    if (offset > 12) offset -= 24;
    if (offset < -12) offset += 24;
    char espBuf[20], rtcLocalBuf[20], utcBuf[20], offsetBuf[6], tempBuf[8];
    snprintf(espBuf,    sizeof(espBuf),    "%02d.%02d.%04d %02d:%02d:%02d",
             localTm.tm_mday, localTm.tm_mon+1, localTm.tm_year+1900,
             localTm.tm_hour, localTm.tm_min,   localTm.tm_sec);
    snprintf(offsetBuf, sizeof(offsetBuf), "%+dh", offset);
    snprintf(utcBuf,    sizeof(utcBuf),    "%02d.%02d.%04d %02d:%02d:%02d",
             utcTm.tm_mday, utcTm.tm_mon+1, utcTm.tm_year+1900,
             utcTm.tm_hour, utcTm.tm_min,   utcTm.tm_sec);
    if (ds3231Found) {
      DateTime rtcUtc = ds3231.now();
      // utcBuf bleibt ESP32-Systemuhr (NTP-Zeit) — nur rtcLocalBuf kommt vom DS3231
      struct tm rtcUtcTm = {};
      rtcUtcTm.tm_year = rtcUtc.year()-1900; rtcUtcTm.tm_mon = rtcUtc.month()-1;
      rtcUtcTm.tm_mday = rtcUtc.day();       rtcUtcTm.tm_hour = rtcUtc.hour();
      rtcUtcTm.tm_min  = rtcUtc.minute();    rtcUtcTm.tm_sec  = rtcUtc.second();
      time_t rtcEpoch = utcToEpoch(&rtcUtcTm);
      struct tm rtcLocal; localtime_r(&rtcEpoch, &rtcLocal);
      snprintf(rtcLocalBuf, sizeof(rtcLocalBuf), "%02d.%02d.%04d %02d:%02d:%02d",
               rtcLocal.tm_mday, rtcLocal.tm_mon+1, rtcLocal.tm_year+1900,
               rtcLocal.tm_hour, rtcLocal.tm_min,   rtcLocal.tm_sec);
      snprintf(tempBuf, sizeof(tempBuf), "%.2f°C", ds3231.getTemperature());
    } else {
      snprintf(rtcLocalBuf, sizeof(rtcLocalBuf), "--");
      snprintf(tempBuf, sizeof(tempBuf), "--");
    }
    String json = "{\"esp_time\":\"" + String(espBuf) + "\",\"rtc_local\":\"" + String(rtcLocalBuf) + "\",\"utc_time\":\"" + String(utcBuf) + "\",\"utc_offset\":\"" + String(offsetBuf) + "\",\"ds3231_found\":" + String(ds3231Found ? 1 : 0) + ",\"ds3231_sync\":" + String(ds3231SyncEnabled ? 1 : 0) + ",\"ds3231_temp\":\"" + String(tempBuf) + "\"}";
    request->send(200, "application/json", json);
  });

  // API: set time directly as UTC (no timezone/DST ambiguity)
  // Usage: /api/set_utc?time=2026-10-25T00:59:00Z
  server.on("/api/set_utc", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("time")) {
      String ts = request->getParam("time")->value();
      struct tm utcTm = {};
      utcTm.tm_year = ts.substring(0, 4).toInt() - 1900;
      utcTm.tm_mon  = ts.substring(5, 7).toInt() - 1;
      utcTm.tm_mday = ts.substring(8, 10).toInt();
      utcTm.tm_hour = ts.substring(11, 13).toInt();
      utcTm.tm_min  = ts.substring(14, 16).toInt();
      utcTm.tm_sec  = ts.substring(17, 19).toInt();
      struct timeval tv = { utcToEpoch(&utcTm), 0 };
      settimeofday(&tv, NULL);
      if (ds3231Found) ds3231.adjust(DateTime(utcTm.tm_year+1900, utcTm.tm_mon+1, utcTm.tm_mday,
                                              utcTm.tm_hour, utcTm.tm_min, utcTm.tm_sec));
      struct tm local; getLocalTime(&local);
      iHour = local.tm_hour; iMinute = local.tm_min; iSecond = local.tm_sec;
      checkforNightMode(); invalidateDstCache(); updatenow = true;
    }
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // API: actions
  server.on("/api/restart", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", "{\"ok\":true}");
    pendingRestartAt = millis() + 500;
  });

  server.on("/api/wifi_reset", HTTP_GET, [](AsyncWebServerRequest* request) {
    preferences.putString("WIFIssid", "");
    preferences.putString("WIFIpass", "");
    request->send(200, "application/json", "{\"ok\":true}");
    pendingRestartAt = millis() + 500;
  });

  server.on("/api/settings_reset", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Keep WiFi, language and operation mode — reset everything else
    String tmpSSID = preferences.getString("WIFIssid");
    String tmpPASS = preferences.getString("WIFIpass");
    int tmpMode = preferences.getUInt("UseOnlineMode", 1);
    preferences.clear();
    preferences.putString("WIFIssid", tmpSSID);
    preferences.putString("WIFIpass", tmpPASS);
    preferences.putUInt("UseOnlineMode", tmpMode);
    request->send(200, "application/json", "{\"ok\":true}");
    pendingRestartAt = millis() + 500;
  });

  // Drift-Test: ESP-Systemuhr um n Sekunden verstellen, DS3231 bleibt unberührt.
  // Damit lässt sich prüfen, ob der 5-Min-Abgleich die ESP-Uhr wieder auf die RTC zurückholt.
  // Usage: /api/esp_shift?sec=60  (negativ erlaubt)
  server.on("/api/esp_shift", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("sec")) {
      long shift = request->getParam("sec")->value().toInt();
      time_t nowEpoch; time(&nowEpoch);
      struct timeval tv = { nowEpoch + shift, 0 };
      settimeofday(&tv, NULL);
      struct tm local; getLocalTime(&local);
      iHour = local.tm_hour; iMinute = local.tm_min; iSecond = local.tm_sec;
      checkforNightMode(); invalidateDstCache(); updatenow = true;
    }
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // Drift-Test: RTC-Resync der ESP-Systemuhr pausieren/fortsetzen (Offline Mode)
  server.on("/api/rtc_sync_off", HTTP_GET, [](AsyncWebServerRequest* request) {
    ds3231SyncEnabled = false;
    request->send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/rtc_sync_on", HTTP_GET, [](AsyncWebServerRequest* request) {
    ds3231SyncEnabled = true;
    request->send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/ota", HTTP_GET, [](AsyncWebServerRequest* request) {
    updatemode = true;
    request->send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/ota_off", HTTP_GET, [](AsyncWebServerRequest* request) {
    updatemode = false;
    request->send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/animation_test", HTTP_GET, [](AsyncWebServerRequest* request) {
    int mode = request->hasParam("mode") ? request->getParam("mode")->value().toInt() : animationMode;
    if (mode != ANIM_NONE) {
      animationMode = mode;
      animationStart();
    }
    request->send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/weather_test", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("icon")) {
      weatherIconCode = request->getParam("icon")->value();
    }
    weatherTestActive = true;
    showWeatherIcon();
    weatherShowing   = true;
    weatherShowStart = millis();
    request->send(200, "application/json", "{\"ok\":true}");
  });

  checkforNightMode();
  server.begin();
}



// ###########################################################################################################################################
// # Read settings from flash:
// ###########################################################################################################################################
void getFlashValues() {
  if (debugtexts == 1) Serial.println("Read settings from flash: START");
  myTimeZone = preferences.getString("myTimeZone", Timezone_default);
  Timezone = myTimeZone;
  myTimeServer = preferences.getString("myTimeServer", NTPserver_default);
  NTPserver = myTimeServer;
  UseOnlineMode = preferences.getUInt("UseOnlineMode", 1);
  showOMhints = preferences.getUInt("showOMhints", showOMhints_default);
  redVal_time = preferences.getUInt("redVal_time", redVal_time_default);
  greenVal_time = preferences.getUInt("greenVal_time", greenVal_time_default);
  blueVal_time = preferences.getUInt("blueVal_time", blueVal_time_default);
  redVal_back = preferences.getUInt("redVal_back", redVal_back_default);
  greenVal_back = preferences.getUInt("greenVal_back", greenVal_back_default);
  blueVal_back = preferences.getUInt("blueVal_back", blueVal_back_default);
  intensity_day = preferences.getUInt("intensity_day", intensity_day_default);
  intensity_night = preferences.getUInt("intensity_night", intensity_night_default);
  usenightmode = preferences.getUInt("usenightmode", usenightmode_default);
  day_time_start = preferences.getString("day_time_start", day_time_start_default);
  day_time_stop = preferences.getString("day_time_stop", day_time_stop_default);
  useshowip = preferences.getUInt("useshowip", useshowip_default);
  useStartupText = preferences.getUInt("useStartupText", useStartupText_default);
  usesinglemin = preferences.getUInt("usesinglemin", usesinglemin_default);
  RandomColor = preferences.getUInt("RandomColor", RandomColor_default);
  weatherApiKey  = preferences.getString("weatherApiKey", "");
  weatherLat     = preferences.getString("weatherLat", "");
  weatherLon     = preferences.getString("weatherLon", "");
  weatherInterval = preferences.getUInt("weatherInterval", weatherInterval_default);
  weatherDuration = preferences.getUInt("weatherDuration", weatherDuration_default);
  animationMode   = preferences.getUInt("animationMode",   ANIM_NONE);
  wifiTimeout     = preferences.getUInt("wifiTimeout",     wifiTimeout_default);
  ntpSyncHour     = preferences.getUInt("ntpSyncHour",     ntpSyncHour_default);
  ntpSyncMinute   = preferences.getUInt("ntpSyncMinute",   ntpSyncMinute_default);
  if (debugtexts == 1) Serial.println("Read settings from flash: END");
}


// ###########################################################################################################################################
// # Write settings to flash:
// # Note: Preferences.put*() checks the existing value first and only writes if it changed — no unnecessary flash wear.
// ###########################################################################################################################################
void setFlashValues() {
  if (debugtexts == 1) Serial.println("Write settings to flash: START");
  changedvalues = false;
  preferences.putString("myTimeZone", myTimeZone);
  preferences.putString("myTimeServer", myTimeServer);
  preferences.putUInt("UseOnlineMode", UseOnlineMode);
  preferences.putUInt("showOMhints", showOMhints);
  preferences.putUInt("redVal_time", redVal_time);
  preferences.putUInt("greenVal_time", greenVal_time);
  preferences.putUInt("blueVal_time", blueVal_time);
  preferences.putUInt("redVal_back", redVal_back);
  preferences.putUInt("greenVal_back", greenVal_back);
  preferences.putUInt("blueVal_back", blueVal_back);
  preferences.putUInt("intensity_day", intensity_day);
  preferences.putUInt("intensity_night", intensity_night);
  preferences.putUInt("usenightmode", usenightmode);
  preferences.putString("day_time_start", day_time_start);
  preferences.putString("day_time_stop", day_time_stop);
  preferences.putUInt("useshowip", useshowip);
  preferences.putUInt("useStartupText", useStartupText);
  preferences.putUInt("usesinglemin", usesinglemin);
  preferences.putUInt("RandomColor", RandomColor);
  preferences.putString("weatherApiKey",  weatherApiKey);
  preferences.putString("weatherLat",     weatherLat);
  preferences.putString("weatherLon",     weatherLon);
  preferences.putUInt("weatherInterval",  weatherInterval);
  preferences.putUInt("weatherDuration",  weatherDuration);
  preferences.putUInt("animationMode",    animationMode);
  preferences.putUInt("wifiTimeout",      wifiTimeout);
  preferences.putUInt("ntpSyncHour",      ntpSyncHour);
  preferences.putUInt("ntpSyncMinute",    ntpSyncMinute);
  if (debugtexts == 1) Serial.println("Write settings to flash: END");
  checkforNightMode();
  updatenow = true;  // Update display now...
}




// ###########################################################################################################################################
// # Clear the display:
// ###########################################################################################################################################
void ClearDisplay() {
  uint32_t c0 = strip.Color(0, 0, 0);
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, c0);
  }
}


// ###########################################################################################################################################
// # Show the IP-address on the display:
// ###########################################################################################################################################
void ShowIPaddress() {
  Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.localIP()));
  int ipdelay = 2000;

  // Testing the digits:
  // for (int i = 0; i < 10; i++) {
  //   ClearDisplay();
  //   numbers(i, 3);
  //   numbers(i, 2);
  //   numbers(i, 1);
  //   showStrip();
  //   delay(ipdelay);
  // }

  // Octet 1:
  ClearDisplay();
  numbers(getDigit(int(WiFi.localIP()[0]), 2), 3);
  numbers(getDigit(int(WiFi.localIP()[0]), 1), 2);
  numbers(getDigit(int(WiFi.localIP()[0]), 0), 1);
  setLED(160, 160, 1);
  setLED(236, 239, 1);
  showStrip();
  delay(ipdelay);

  // // Octet 2:
  ClearDisplay();
  numbers(getDigit(int(WiFi.localIP()[1]), 2), 3);
  numbers(getDigit(int(WiFi.localIP()[1]), 1), 2);
  numbers(getDigit(int(WiFi.localIP()[1]), 0), 1);
  setLED(160, 160, 1);
  setLED(232, 239, 1);
  showStrip();
  delay(ipdelay);

  // // Octet 3:
  ClearDisplay();
  numbers(getDigit(int(WiFi.localIP()[2]), 2), 3);
  numbers(getDigit(int(WiFi.localIP()[2]), 1), 2);
  numbers(getDigit(int(WiFi.localIP()[2]), 0), 1);
  setLED(160, 160, 1);
  setLED(228, 239, 1);
  showStrip();
  delay(ipdelay);

  // // Octet 4:
  ClearDisplay();
  numbers(getDigit(int(WiFi.localIP()[3]), 2), 3);
  numbers(getDigit(int(WiFi.localIP()[3]), 1), 2);
  numbers(getDigit(int(WiFi.localIP()[3]), 0), 1);
  setLED(224, 239, 1);
  showStrip();
  delay(ipdelay);
}


// ###########################################################################################################################################
// # Set the numbers on the display in each single row:
// ###########################################################################################################################################
void numbers(int wert, int segment) {

  // Serial.println(wert);

  switch (segment) {
    case 3:
      {
        switch (wert) {
          case 0:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(79, 79, 1);
              setLED(108, 108, 1);
              setLED(111, 111, 1);
              setLED(140, 140, 1);
              setLED(143, 143, 1);
              setLED(172, 175, 1);
              break;
            }
          case 1:
            {
              setLED(44, 44, 1);
              setLED(76, 76, 1);
              setLED(108, 108, 1);
              setLED(140, 140, 1);
              setLED(172, 172, 1);
              break;
            }
          case 2:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(108, 111, 1);
              setLED(143, 143, 1);
              setLED(172, 175, 1);
              break;
            }
          case 3:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(172, 175, 1);
              break;
            }
          case 4:
            {
              setLED(44, 44, 1);
              setLED(47, 47, 1);
              setLED(76, 76, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(172, 172, 1);
              break;
            }
          case 5:
            {
              setLED(44, 47, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(172, 175, 1);
              break;
            }
          case 6:
            {
              setLED(44, 47, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(143, 143, 1);
              setLED(172, 175, 1);
              break;
            }
          case 7:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(108, 108, 1);
              setLED(140, 140, 1);
              setLED(172, 172, 1);
              break;
            }
          case 8:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(143, 143, 1);
              setLED(172, 175, 1);
              break;
            }
          case 9:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(172, 175, 1);
              break;
            }
        }
        break;
      }

    case 2:
      {
        switch (wert) {
          case 0:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(74, 74, 1);
              setLED(103, 103, 1);
              setLED(106, 106, 1);
              setLED(135, 135, 1);
              setLED(138, 138, 1);
              setLED(167, 170, 1);
              break;
            }
          case 1:
            {
              setLED(39, 39, 1);
              setLED(71, 71, 1);
              setLED(103, 103, 1);
              setLED(135, 135, 1);
              setLED(167, 167, 1);
              break;
            }
          case 2:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(103, 106, 1);
              setLED(138, 138, 1);
              setLED(167, 170, 1);
              break;
            }
          case 3:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(167, 170, 1);
              break;
            }
          case 4:
            {
              setLED(39, 39, 1);
              setLED(42, 42, 1);
              setLED(71, 71, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(167, 167, 1);
              break;
            }
          case 5:
            {
              setLED(39, 42, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(167, 170, 1);
              break;
            }
          case 6:
            {
              setLED(39, 42, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(138, 138, 1);
              setLED(167, 170, 1);
              break;
            }
          case 7:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(103, 103, 1);
              setLED(135, 135, 1);
              setLED(167, 167, 1);
              break;
            }
          case 8:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(138, 138, 1);
              setLED(167, 170, 1);
              break;
            }
          case 9:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(167, 170, 1);
              break;
            }
        }
        break;
      }

    case 1:
      {
        switch (wert) {
          case 0:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(69, 69, 1);
              setLED(98, 98, 1);
              setLED(101, 101, 1);
              setLED(130, 130, 1);
              setLED(133, 133, 1);
              setLED(162, 165, 1);
              break;
            }
          case 1:
            {
              setLED(34, 34, 1);
              setLED(66, 66, 1);
              setLED(98, 98, 1);
              setLED(130, 130, 1);
              setLED(162, 162, 1);
              break;
            }
          case 2:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(98, 101, 1);
              setLED(133, 133, 1);
              setLED(162, 165, 1);
              break;
            }
          case 3:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(162, 165, 1);
              break;
            }
          case 4:
            {
              setLED(34, 34, 1);
              setLED(37, 37, 1);
              setLED(66, 66, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(162, 162, 1);
              break;
            }
          case 5:
            {
              setLED(34, 37, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(162, 165, 1);
              break;
            }
          case 6:
            {
              setLED(34, 37, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(133, 133, 1);
              setLED(162, 165, 1);
              break;
            }
          case 7:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(98, 98, 1);
              setLED(130, 130, 1);
              setLED(162, 162, 1);
              break;
            }
          case 8:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(133, 133, 1);
              setLED(162, 165, 1);
              break;
            }
          case 9:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(162, 165, 1);
              break;
            }
        }
        break;
      }
  }
}


// ###########################################################################################################################################
// # Get a digit from a number at position pos: (Split IP-address octets in single digits)
// ###########################################################################################################################################
int getDigit(int number, int pos) {
  return (pos == 0) ? number % 10 : getDigit(number / 10, --pos);
}



// ###########################################################################################################################################
// # Show a LED output for RESET in the different languages:
// ###########################################################################################################################################
void ResetTextLEDs(uint32_t color) {
  updatedevice = false;
  delay(1000);
  ClearDisplay();

  if (langLEDlayout == 0) {      // DE:
    setLEDcol(137, 138, color);  // RE
    setLEDcol(167, 168, color);  // SE
    setLEDcol(227, 227, color);  // T
  }

  if (langLEDlayout == 1) {      // EN:
    setLEDcol(100, 101, color);  // RE
    setLEDcol(174, 175, color);  // SE
    setLEDcol(227, 227, color);  // T
  }

  if (langLEDlayout == 2) {      // NL:
    setLEDcol(33, 33, color);    // R
    setLEDcol(96, 97, color);    // ES
    setLEDcol(164, 164, color);  // E
    setLEDcol(227, 227, color);  // T
  }

  if (langLEDlayout == 3) {    // SWE:
    setLEDcol(67, 71, color);  // R
  }

  if (langLEDlayout == 4) {    // IT:
    setLEDcol(11, 11, color);  // R
    setLEDcol(9, 9, color);    // E
    setLEDcol(45, 47, color);  // SET
  }

  if (langLEDlayout == 5) {    // FR:
    setLEDcol(11, 13, color);  // RES
    setLEDcol(5, 5, color);    // E
    setLEDcol(36, 36, color);  // T
  }

  if (langLEDlayout == 6) {    // GSW:
    setLEDcol(11, 15, color);  // RESET
  }

  if (langLEDlayout == 7) {    // CN:
    setLEDcol(38, 39, color);  // RESET 重置
  }

  if (langLEDlayout == 8) {      // SWABIAN GERMAN:
    setLEDcol(40, 41, color);    // RE
    setLEDcol(133, 134, color);  // SE
    setLEDcol(204, 204, color);  // T
  }

  if (langLEDlayout == 9) {      // BAVARIAN:
    setLEDcol(106, 106, color);  // R
    setLEDcol(133, 133, color);  // E
    setLEDcol(175, 175, color);  // S
    setLEDcol(170, 170, color);  // E
    setLEDcol(160, 160, color);  // T
  }

  if (langLEDlayout == 10) {     // LUXEMBOURGISH:
    setLEDcol(38, 38, color);    // R
    setLEDcol(73, 73, color);    // E
    setLEDcol(103, 103, color);  // S
    setLEDcol(139, 139, color);  // E
    setLEDcol(171, 171, color);  // T
  }

  if (langLEDlayout == 11) {  // EAST GERMAN:
    setLEDcol(5, 9, color);   // RESET
  }

  showStrip();
}


// ###########################################################################################################################################
// # Actual function, which controls 1/0 of the LED and their sibling with color value:
// ###########################################################################################################################################
void setLEDcol(int ledNrFrom, int ledNrTo, uint32_t color) {
  if (ledNrFrom > ledNrTo) {
    setLEDcol(ledNrTo, ledNrFrom, color);  // Sets LED numbers in correct order
  } else {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS)) {
        strip.setPixelColor(i, color);
        int pairedLED = getPairedLED(i);
        if ((pairedLED >= 0) && (pairedLED < NUMPIXELS))
          strip.setPixelColor(pairedLED, color);
      }
    }
  }
}


// ###########################################################################################################################################
// # Get the sibling led for a two-led lit character (with 32 leds / 16 chars per row):
// ###########################################################################################################################################
int getPairedLED(int ledNumber) {
  const int ledsPerLine = ROWPIXELS * 2;
  int row = ledNumber / ledsPerLine;
  int positionInRow = ledNumber % ledsPerLine;
  return row * ledsPerLine + (ledsPerLine - 1 - positionInRow);
}



// ###########################################################################################################################################
// # GUI: Convert IP-address value to string:
// ###########################################################################################################################################
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]);
}


// ###########################################################################################################################################
// # GUI: Check if Night Mode needs to be set depending on the time:
// ###########################################################################################################################################
void checkforNightMode() {
  // Start time to int:
  String day_time_startH = getValue(day_time_start, ':', 0);
  String day_time_startM = getValue(day_time_start, ':', 1);
  if (debugtexts == 1) Serial.println("day_time_start H part = " + day_time_startH);
  if (debugtexts == 1) Serial.println("day_time_start M part = " + day_time_startM);
  int dt_start_HM = (day_time_startH.toInt() * 100) + day_time_startM.toInt();
  if (debugtexts == 1) Serial.println("dt_start_HM = " + String(dt_start_HM));
  // Stop time to int:
  String day_time_stopH = getValue(day_time_stop, ':', 0);
  String day_time_stopM = getValue(day_time_stop, ':', 1);
  if (debugtexts == 1) Serial.println("day_time_stop H part = " + day_time_stopH);
  if (debugtexts == 1) Serial.println("day_time_stop M part = " + day_time_stopM);
  int dt_stop_HM = (day_time_stopH.toInt() * 100) + day_time_stopM.toInt();
  if (debugtexts == 1) Serial.println("dt_stop_HM = " + String(dt_stop_HM));
  // Current time to int:
  int now_HM = (iHour * 100) + iMinute;
  if (debugtexts == 1) Serial.println("now_HM = " + String(now_HM));

  // Check if start time is before stop time:
  if ((dt_start_HM > dt_stop_HM) || (dt_start_HM == dt_stop_HM)) {
    if (debugtexts == 1) Serial.println("Wrong setting! 'Day time starts' needs to be before 'Day time ends after'. Please correct values.");
    nightModeWarning = true;
  } else {
    nightModeWarning = false;
    if (usenightmode == 1) {
      if ((now_HM >= dt_start_HM) && (now_HM <= dt_stop_HM)) {
        if (debugtexts == 1) Serial.println("Day Time");
        statusNightModeIDtxt = "Day time";
        if ((iHour == 0) && (day_time_startH.toInt() == 23)) {
          statusNightModeIDtxt = "Night time";
        }
      } else {
        if (debugtexts == 1) Serial.println("Night Time");
        statusNightModeIDtxt = "Night time";
      }
    } else {
      statusNightModeIDtxt = "Night mode not used";
    }
  }
}


// ###########################################################################################################################################
// # Update the display / time on it:
// ###########################################################################################################################################
void update_display() {
  // Show the current time or use the time text test function:
  if (testTime == 0) {  // Show the current time:
    show_time(iHour, iMinute);
  } else {  // TEST THE DISPLAY TIME OUTPUT:
    ClearDisplay();
    showStrip();
    Serial.println(" ");
    Serial.println("Show 'TEST' text...");
    strip.setBrightness(25);
    redVal_back = 0;
    greenVal_back = 0;
    blueVal_back = 0;
    usenightmode = 0;
    uint32_t c = strip.Color(redVal_time, greenVal_time, blueVal_time);
    int TextWait = 500;
    showtext('T', TextWait, c);
    showtext('E', TextWait, c);
    showtext('S', TextWait, c);
    showtext('T', TextWait, c);
    for (int i = 1; i <= 12; i++) {  // 12 hours only:
      show_time(i, 0);
      delay(3000);
    }
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    for (int i = 0; i <= 55; i += 5) {  // 5 minutes steps only:
      show_time(9, i);
      delay(3000);
    }
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    for (int i = 9; i <= 12; i++) {  // Hours 9 to 12 with all minute texts:
      for (int y = 0; y < 60; y++) {
        show_time(i, y);
        delay(500);
      }
    }
  }
}


// ###########################################################################################################################################
// # Display hours and minutes text function:
// ###########################################################################################################################################
uint32_t colorRGB;
static int lastHourSet = -1;
static int lastMinutesSet = -1;
void show_time(int hours, int minutes) {

  bool timeChanged = (lastHourSet != hours || lastMinutesSet != minutes);
  if (!timeChanged && updatenow == false) return;

  updatenow = false;
  lastHourSet = hours;
  lastMinutesSet = minutes;

  if (timeChanged && minutes % 5 == 0 && animationMode != ANIM_NONE) {
    animationStart();
    return;
  }

  // Show current time of display update:
  if (debugtexts == 1) Serial.println("Update display now: " + String(hours) + ":" + String(minutes) + ":" + String(iSecond));

  // Set LED intensity + DAY/NIGHT MDOE:
  // ##################
  // Start time to int:
  String day_time_startH = getValue(day_time_start, ':', 0);
  String day_time_startM = getValue(day_time_start, ':', 1);
  int dt_start_HM = (day_time_startH.toInt() * 100) + day_time_startM.toInt();
  // Stop time to int:
  String day_time_stopH = getValue(day_time_stop, ':', 0);
  String day_time_stopM = getValue(day_time_stop, ':', 1);
  int dt_stop_HM = (day_time_stopH.toInt() * 100) + day_time_stopM.toInt();
  // Current time to int:
  int now_HM = (iHour * 100) + iMinute;

  // Set intensity:
  if ((usenightmode == 1) && (set_web_intensity == 0)) {
    if ((now_HM >= dt_start_HM) && (now_HM <= dt_stop_HM)) {
      intensity = intensity_day;
      statusNightModeIDtxt = "Day time";
      if ((iHour == 0) && (day_time_startH.toInt() == 23)) {
        intensity = intensity_night;
        statusNightModeIDtxt = "Night time";
      }
    } else {
      intensity = intensity_night;
      statusNightModeIDtxt = "Night time";
    }
  } else {
    if (set_web_intensity == 0) intensity = intensity_day;
    if (set_web_intensity == 1) intensity = intensity_web;
  }
  strip.setBrightness(intensity);

  // Set background color:
  back_color();

  // Static text color or random color mode:
  if (RandomColor == 0) colorRGB = strip.Color(redVal_time, greenVal_time, blueVal_time);
  if (RandomColor == 1) colorRGB = strip.Color(random(255), random(255), random(255));

  // Display time:
  iHour = hours;
  iMinute = minutes;

  // Test a special time:
  if (testspecialtimeON == 1) {
    Serial.println("Special time test active in Online Mode: " + String(test_hourON) + ":" + String(test_minuteON) + ":" + String(test_secondON));
    iHour = test_hourON;
    iMinute = test_minuteON;
    iSecond = test_secondON;
  }

  // divide minute by 5 to get value for display control
  int minDiv = iMinute / 5;
  if (usesinglemin == 1) showMinutes(iMinute);

  // ########################################################### DE:
  if (langLEDlayout == 0) {  // DE:

    // ES IST:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(10, 12, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES IST ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(76, 79, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }
    // VIERTEL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(69, 75, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTEL ");
    }
    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(32, 35, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }
    // ZWANZIG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(41, 47, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZWANZIG ");
    }
    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(64, 67, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }
    // VOR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(109, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }
    // HALB:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(104, 107, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(169, 171, colorRGB);  // EIN
            if (testPrintTimeTexts == 1) Serial.print("EIN ");
          }
          if ((xHour == 1) && (iMinute > 4)) {
            setLEDcol(168, 171, colorRGB);  // EINS (S in EINS) (just used if not point 1 o'clock)
            if (testPrintTimeTexts == 1) Serial.print("EINS ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(140, 143, colorRGB);  // ZWEI
          if (testPrintTimeTexts == 1) Serial.print("ZWEI ");
          break;
        }
      case 3:
        {
          setLEDcol(136, 139, colorRGB);  // DREI
          if (testPrintTimeTexts == 1) Serial.print("DREI ");
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // VIER
          if (testPrintTimeTexts == 1) Serial.print("VIER ");
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // FUENF
          if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
          break;
        }
      case 6:
        {
          setLEDcol(164, 168, colorRGB);  // SECHS
          if (testPrintTimeTexts == 1) Serial.print("SECHS ");
          break;
        }
      case 7:
        {
          setLEDcol(202, 207, colorRGB);  // SIEBEN
          if (testPrintTimeTexts == 1) Serial.print("SIEBEN ");
          break;
        }
      case 8:
        {
          setLEDcol(172, 175, colorRGB);  // ACHT
          if (testPrintTimeTexts == 1) Serial.print("ACHT ");
          break;
        }
      case 9:
        {
          setLEDcol(132, 135, colorRGB);  // NEUN
          if (testPrintTimeTexts == 1) Serial.print("NEUN ");
          break;
        }
      case 10:
        {
          setLEDcol(99, 102, colorRGB);  // ZEHN (Stunden)
          if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
          break;
        }
      case 11:
        {
          setLEDcol(96, 98, colorRGB);  // ELF
          if (testPrintTimeTexts == 1) Serial.print("ELF ");
          break;
        }
      case 12:
        {
          setLEDcol(197, 201, colorRGB);  // ZWÖLF
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLF ");
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(192, 194, colorRGB);  // UHR
      if (testPrintTimeTexts == 1) Serial.print("UHR ");
    }
  }

  // ########################################################### EN:
  if (langLEDlayout == 1) {  // EN:

    // IT IS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(11, 12, colorRGB);

    // FIVE: (Minutes)                         // x:05 + x:25 + x:35 + x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(38, 41, colorRGB);
    }
    // QUARTER:                                // x:15 + X:45
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(72, 78, colorRGB);
    }
    // A:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(5, 5, colorRGB);
    }
    // TEN: (Minutes)                          // x:10 + x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(0, 2, colorRGB);
    }
    // TWENTY:                                 // x:20 + x:25 + x:35 + x:40
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(42, 47, colorRGB);
    }
    // PAST:                                   // x:05 + x:10 + x:15 + x:20 + x:25 + x:30
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 5) || (minDiv == 6)) {
      setLEDcol(66, 69, colorRGB);
    }
    // TO:                                     // x:35 + x:40 + x:45 + x:50 + x:55
    if ((minDiv == 7) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(65, 66, colorRGB);
    }
    // HALF:                                   // x:30
    if ((minDiv == 6)) {
      setLEDcol(3, 6, colorRGB);
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 35 hour needs to be counted up:
    // Twenty five to two = 13:35
    if (iMinute >= 35) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(201, 203, colorRGB);  // ONE
          break;
        }
      case 2:
        {
          setLEDcol(105, 107, colorRGB);  // TWO
          break;
        }
      case 3:
        {
          setLEDcol(99, 103, colorRGB);  // THREE
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // FOUR
          break;
        }
      case 5:
        {
          setLEDcol(108, 111, colorRGB);  // FIVE
          break;
        }
      case 6:
        {
          setLEDcol(163, 165, colorRGB);  // SIX
          break;
        }
      case 7:
        {
          setLEDcol(171, 175, colorRGB);  // SEVEN
          break;
        }
      case 8:
        {
          setLEDcol(166, 170, colorRGB);  // EIGHT
          break;
        }
      case 9:
        {
          setLEDcol(204, 207, colorRGB);  // NINE
          break;
        }
      case 10:
        {
          setLEDcol(96, 98, colorRGB);  // TEN
          break;
        }
      case 11:
        {
          setLEDcol(138, 143, colorRGB);  // ELEVEN
          break;
        }
      case 12:
        {
          setLEDcol(132, 137, colorRGB);  // TWELVE
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(193, 199, colorRGB);  // O'CLOCK
    }
  }

  // ########################################################### NL:
  if (langLEDlayout == 2) {  // NL:

    // HET IS:
    setLEDcol(13, 15, colorRGB);
    setLEDcol(10, 11, colorRGB);
    // VIJF: (Minuten) x:05, x:25, x:35, x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(0, 3, colorRGB);
    }
    // KWART: x:15, x:45
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(38, 42, colorRGB);
    }
    // TIEN: (Minuten) x:10, x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(44, 47, colorRGB);
    }
    // TIEN: (TIEN VOOR HALF, TIEN OVER HALF) x:20, x:40 (on request not set to TWINTIG OVER)
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(44, 47, colorRGB);
    }
    // OVER: x:05, x:10, x:15, x:35, x:40
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(33, 36, colorRGB);
    }
    // VOOR: x:20, x:25, x:45, x:50, x:55
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(64, 67, colorRGB);
    }
    // HALF:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(107, 110, colorRGB);
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 20 hour needs to be counted up:
    // tien voor half 2 = 13:20
    if (iMinute >= 20) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(99, 101, colorRGB);  // EEN
          break;
        }
      case 2:
        {
          setLEDcol(203, 206, colorRGB);  // TWEE
          break;
        }
      case 3:
        {
          setLEDcol(164, 167, colorRGB);  // DRIE
          break;
        }
      case 4:
        {
          setLEDcol(198, 201, colorRGB);  // VIER
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // VIJF
          break;
        }
      case 6:
        {
          setLEDcol(96, 98, colorRGB);  // ZES
          break;
        }
      case 7:
        {
          setLEDcol(129, 133, colorRGB);  // ZEVEN
          break;
        }
      case 8:
        {
          setLEDcol(102, 105, colorRGB);  // ACHT
          break;
        }
      case 9:
        {
          setLEDcol(171, 175, colorRGB);  // NEGEN
          break;
        }
      case 10:
        {
          setLEDcol(140, 143, colorRGB);  // TIEN (Stunden)
          break;
        }
      case 11:
        {
          setLEDcol(168, 170, colorRGB);  // ELF
          break;
        }
      case 12:
        {
          setLEDcol(134, 139, colorRGB);  // TWAALF
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(193, 195, colorRGB);  // UUR
    }
  }

  // ########################################################### SWE:
  if (langLEDlayout == 3) {  // SWE:

    // KLOCKAN ÄR:
    setLEDcol(9, 15, colorRGB);
    setLEDcol(5, 6, colorRGB);
    // FEM: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(64, 66, colorRGB);
    }
    // KVART:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(36, 40, colorRGB);
    }
    // TIO: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(72, 74, colorRGB);
    }
    // TJUGO:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(75, 79, colorRGB);
    }
    // ÖVER:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(108, 111, colorRGB);
    }
    // I:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(97, 97, colorRGB);
    }
    // HALV:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(140, 143, colorRGB);
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(170, 172, colorRGB);  // ETT
          }
          break;
        }
      case 2:
        {
          setLEDcol(160, 162, colorRGB);  // TVÅ
          break;
        }
      case 3:
        {
          setLEDcol(132, 134, colorRGB);  // TRE
          break;
        }
      case 4:
        {
          setLEDcol(197, 200, colorRGB);  // FYRA
          break;
        }
      case 5:
        {
          setLEDcol(194, 196, colorRGB);  // FEM
          break;
        }
      case 6:
        {
          setLEDcol(201, 203, colorRGB);  // SEX
          break;
        }
      case 7:
        {
          setLEDcol(173, 175, colorRGB);  // SJU
          break;
        }
      case 8:
        {
          setLEDcol(128, 131, colorRGB);  // ÅTTA
          break;
        }
      case 9:
        {
          setLEDcol(135, 137, colorRGB);  // NIO
          break;
        }
      case 10:
        {
          setLEDcol(168, 170, colorRGB);  // TIO (Stunden)
          break;
        }
      case 11:
        {
          setLEDcol(204, 207, colorRGB);  // ELVA
          break;
        }
      case 12:
        {
          setLEDcol(163, 166, colorRGB);  // TOLV
          break;
        }
    }
  }

  // ########################################################### IT:
  if (langLEDlayout == 4) {  // IT:

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 40 hour needs to be counted up:
    if (iMinute >= 40) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }

    // SONO LE:
    if (xHour > 1) {                // NOTE: Displayed only from 2 to 23
      setLEDcol(9, 10, colorRGB);   // LE
      setLEDcol(12, 15, colorRGB);  // SONO
      if (testPrintTimeTexts == 1) {
        Serial.println("");
        Serial.print(hours);
        Serial.print(":");
        Serial.print(minutes);
        Serial.print(" --> SONO LE ");
      }
    }

    switch (xHour) {
      case 1:
        {
          setLEDcol(0, 0, colorRGB);      // È
          setLEDcol(104, 108, colorRGB);  // L’UNA
          if (testPrintTimeTexts == 1) Serial.print("È L’UNA ");
          break;
        }
      case 2:
        {
          setLEDcol(101, 103, colorRGB);  // DUE
          if (testPrintTimeTexts == 1) Serial.print("DUE ");
          break;
        }
      case 3:
        {
          setLEDcol(109, 111, colorRGB);  // TRE
          if (testPrintTimeTexts == 1) Serial.print("TRE ");
          break;
        }
      case 4:
        {
          setLEDcol(73, 79, colorRGB);  // QUATTRO
          if (testPrintTimeTexts == 1) Serial.print("QUATTRO ");
          break;
        }
      case 5:
        {
          setLEDcol(64, 69, colorRGB);  // CINQUE
          if (testPrintTimeTexts == 1) Serial.print("CINQUE ");
          break;
        }
      case 6:
        {
          setLEDcol(40, 42, colorRGB);  // SEI
          if (testPrintTimeTexts == 1) Serial.print("SEI ");
          break;
        }
      case 7:
        {
          setLEDcol(43, 47, colorRGB);  // SETTE
          if (testPrintTimeTexts == 1) Serial.print("SETTE ");
          break;
        }
      case 8:
        {
          setLEDcol(70, 73, colorRGB);  // OTTO
          if (testPrintTimeTexts == 1) Serial.print("OTTO ");
          break;
        }
      case 9:
        {
          setLEDcol(97, 100, colorRGB);  // NOVE
          if (testPrintTimeTexts == 1) Serial.print("NOVE ");
          break;
        }
      case 10:
        {
          setLEDcol(138, 142, colorRGB);  // DIECI
          if (testPrintTimeTexts == 1) Serial.print("DIECI ");
          break;
        }
      case 11:
        {
          setLEDcol(1, 6, colorRGB);  // UNDICI
          if (testPrintTimeTexts == 1) Serial.print("UNDICI ");
          break;
        }
      case 12:
        {
          setLEDcol(34, 39, colorRGB);  // DODICI
          if (testPrintTimeTexts == 1) Serial.print("DODICI ");
          break;
        }
    }

    // E:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(134, 134, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("E ");
    }
    // MENO:
    if ((minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(132, 135, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("MENO ");
    }
    // 5/55: CINQUE
    if ((minDiv == 1) || (minDiv == 11)) {
      setLEDcol(162, 167, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("CINQUE ");
    }
    // 15/45: UN QUARTO
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(128, 129, colorRGB);  // UN
      setLEDcol(234, 239, colorRGB);  // QUARTO
      if (testPrintTimeTexts == 1) Serial.print("UN QUARTO ");
    }
    // 10/50: DIECI
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(192, 196, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("DIECI ");
    }
    // 20/40: VENTI
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(203, 207, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VENTI ");
    }
    // 25: VENTICINQUE
    if (minDiv == 5) {
      setLEDcol(197, 207, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VENTICINQUE ");
    }
    // 30: TRENTA
    if (minDiv == 6) {
      setLEDcol(168, 173, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("TRENTA ");
    }
    // 35: TRENTACINQUE
    if (minDiv == 7) {
      setLEDcol(162, 173, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("TRENTACINQUE ");
    }
  }

  // ########################################################### FR:
  if (langLEDlayout == 5) {  // FR:

    // IL EST:
    setLEDcol(14, 15, colorRGB);  // IL
    setLEDcol(10, 12, colorRGB);  // EST
    // CINQ: (Minutes) x:05, x:25, x:35, x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(197, 200, colorRGB);
    }
    // ET QUART: x:15,
    if ((minDiv == 3)) {
      setLEDcol(171, 172, colorRGB);  // ET
      setLEDcol(193, 197, colorRGB);  // QUART
    }
    // LE QUART: x:45
    if ((minDiv == 9)) {
      setLEDcol(172, 173, colorRGB);  // LE
      setLEDcol(193, 197, colorRGB);  // QUART
    }
    // DIX: (Minutes) x:10, x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(167, 169, colorRGB);
    }
    // VINGT: x:20, x:40
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(202, 206, colorRGB);
    }
    // VINGT-: x:25, x:35
    if ((minDiv == 5) || (minDiv == 7)) {
      setLEDcol(201, 206, colorRGB);
    }
    // MOINS: x:35, x:40 x:45, x:50, x:55
    if ((minDiv == 7) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(128, 132, colorRGB);
    }
    // ET DEMIE: x:30
    if ((minDiv == 6)) {
      setLEDcol(171, 172, colorRGB);  // ET
      setLEDcol(161, 165, colorRGB);  // DEMIE
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 35 hour needs to be counted up:
    if (iMinute >= 35) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(141, 143, colorRGB);  // UNE
          setLEDcol(135, 139, colorRGB);  // HEURE
          break;
        }
      case 2:
        {
          setLEDcol(96, 99, colorRGB);    // DEUX
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 3:
        {
          setLEDcol(107, 111, colorRGB);  // TROIS
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 4:
        {
          setLEDcol(42, 47, colorRGB);    // QUATRE
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 5:
        {
          setLEDcol(1, 4, colorRGB);      // CINQ
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 6:
        {
          setLEDcol(64, 66, colorRGB);    // SIX
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 7:
        {
          setLEDcol(104, 107, colorRGB);  // SEPT
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 8:
        {
          setLEDcol(32, 35, colorRGB);    // HUIT
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 9:
        {
          setLEDcol(100, 103, colorRGB);  // NEUF
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 10:
        {
          setLEDcol(77, 79, colorRGB);    // DIX
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 11:
        {
          setLEDcol(5, 8, colorRGB);      // ONZE
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 12:
        {
          // MINUIT (0) or MIDI (12)
          if (iHour == 0 || (iHour == 23 && iMinute >= 35)) setLEDcol(36, 41, colorRGB);   // MINUIT (0)
          if (iHour == 12 || (iHour == 11 && iMinute >= 35)) setLEDcol(73, 76, colorRGB);  // MIDI (12)
          break;
        }
    }
  }

  // ########################################################### GSW:
  if (langLEDlayout == 6) {  // GSW:

    // ES ISCH:
    setLEDcol(13, 14, colorRGB);  // ES
    setLEDcol(4, 7, colorRGB);    // ISCH
    // FÜÜF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(44, 47, colorRGB);
    }
    // VIERTEL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(72, 78, colorRGB);
    }
    // ZÄH: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(34, 36, colorRGB);
    }
    // ZWÄNZG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(65, 70, colorRGB);
    }
    // AB:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(110, 111, colorRGB);
    }
    // VOR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(107, 109, colorRGB);
    }
    // HALBI:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(101, 105, colorRGB);
    }


    // set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(128, 130, colorRGB);  // EIS
          break;
        }
      case 2:
        {
          setLEDcol(136, 139, colorRGB);  // ZWEI
          break;
        }
      case 3:
        {
          setLEDcol(96, 99, colorRGB);  // DRÜÜ
          break;
        }
      case 4:
        {
          setLEDcol(160, 164, colorRGB);  // VIERI
          break;
        }
      case 5:
        {
          setLEDcol(171, 175, colorRGB);  // FÜÜFI
          break;
        }
      case 6:
        {
          setLEDcol(165, 170, colorRGB);  // SÄCHSI
          break;
        }
      case 7:
        {
          setLEDcol(202, 207, colorRGB);  // SIEBNI
          break;
        }
      case 8:
        {
          setLEDcol(192, 196, colorRGB);  // ACHTI
          break;
        }
      case 9:
        {
          setLEDcol(131, 135, colorRGB);  // NÜÜNI
          break;
        }
      case 10:
        {
          setLEDcol(197, 201, colorRGB);  // ZÄHNI (Stunden)
          break;
        }
      case 11:
        {
          setLEDcol(140, 143, colorRGB);  // ELFI
          break;
        }
      case 12:
        {
          setLEDcol(233, 238, colorRGB);  // ZWÖLFI
          break;
        }
    }
  }

  // ########################################################### CN:
  if (langLEDlayout == 7) {  // CN:

    // IT IS: 现在 时间
    setLEDcol(44, 45, colorRGB);
    setLEDcol(40, 41, colorRGB);
    // 零五分                         // x:05
    if ((minDiv == 1)) {
      setLEDcol(101, 103, colorRGB);
    }
    // 十分                         // x:10
    if ((minDiv == 2)) {
      setLEDcol(98, 99, colorRGB);
    }
    // 十五分                         // x:15
    if ((minDiv == 3)) {
      setLEDcol(138, 140, colorRGB);
    }
    // 二十分                         // x:20
    if ((minDiv == 4)) {
      setLEDcol(98, 100, colorRGB);
    }
    // 二十五分                         // x:25
    if ((minDiv == 5)) {
      setLEDcol(138, 141, colorRGB);
    }
    // 三十分                         // x:30
    if ((minDiv == 6)) {
      setLEDcol(135, 137, colorRGB);
    }
    // 三十五分                         // x:35
    if ((minDiv == 7)) {
      setLEDcol(170, 173, colorRGB);
    }
    // 四十分                         // x:40
    if ((minDiv == 8)) {
      setLEDcol(132, 134, colorRGB);
    }
    // 四十五分                         // x:45
    if ((minDiv == 9)) {
      setLEDcol(166, 169, colorRGB);
    }
    // 五十分                         // x:50
    if ((minDiv == 10)) {
      setLEDcol(163, 165, colorRGB);
    }
    // 五十五分                         // x:55
    if ((minDiv == 11)) {
      setLEDcol(202, 205, colorRGB);
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0) {
      xHour = 12;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(75, 76, colorRGB);  // 一点
          break;
        }
      case 2:
        {
          setLEDcol(72, 73, colorRGB);  // 二点
          break;
        }
      case 3:
        {
          setLEDcol(36, 37, colorRGB);  // 三点
          break;
        }
      case 4:
        {
          setLEDcol(34, 35, colorRGB);  // 四点
          break;
        }
      case 5:
        {
          setLEDcol(70, 71, colorRGB);  // 五点
          break;
        }
      case 6:
        {
          setLEDcol(68, 69, colorRGB);  // 六点
          break;
        }
      case 7:
        {
          setLEDcol(66, 67, colorRGB);  // 七点
          break;
        }
      case 8:
        {
          setLEDcol(108, 109, colorRGB);  // 八点
          break;
        }
      case 9:
        {
          setLEDcol(106, 107, colorRGB);  // 九点
          break;
        }
      case 10:
        {
          setLEDcol(104, 105, colorRGB);  // 十点
          break;
        }
      case 11:
        {
          setLEDcol(75, 77, colorRGB);  // 十一点
          break;
        }
      case 12:
        {
          setLEDcol(72, 74, colorRGB);  // 十二点
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(162, 162, colorRGB);  // 整
    }
  }

  // ########################################################### SWABIAN GERMAN:
  if (langLEDlayout == 8) {  // SWABIAN GERMAN:

    // ES ISCH:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(9, 12, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES ISCH ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(76, 79, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }

    // VIERTL:
    if (minDiv == 3) {
      setLEDcol(0, 5, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTL ");
    }

    // DREIVIERTL:
    if (minDiv == 9) {
      setLEDcol(33, 42, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("DREIVIERTL ");
    }

    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 4) || (minDiv == 8) || (minDiv == 10)) {
      setLEDcol(71, 74, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }

    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(65, 68, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }

    // VOR:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(109, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }

    // HALB:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(102, 105, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:15
    if (iMinute >= 15) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }

    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(165, 168, colorRGB);  // OISE
            if (testPrintTimeTexts == 1) Serial.print("OISE ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(160, 164, colorRGB);  // ZWOIE
          if (testPrintTimeTexts == 1) Serial.print("ZWOIE ");
          break;
        }
      case 3:
        {
          setLEDcol(235, 239, colorRGB);  // DREIE
          if (testPrintTimeTexts == 1) Serial.print("DREIE ");
          break;
        }
      case 4:
        {
          setLEDcol(128, 132, colorRGB);  // VIERE
          if (testPrintTimeTexts == 1) Serial.print("VIERE ");
          break;
        }
      case 5:
        {
          setLEDcol(139, 143, colorRGB);  // FÜNFE
          if (testPrintTimeTexts == 1) Serial.print("FÜNFE ");
          break;
        }
      case 6:
        {
          setLEDcol(133, 138, colorRGB);  // SECHSE
          if (testPrintTimeTexts == 1) Serial.print("SECHSE ");
          break;
        }
      case 7:
        {
          setLEDcol(169, 175, colorRGB);  // SIEBENE
          if (testPrintTimeTexts == 1) Serial.print("SIEBENE ");
          break;
        }
      case 8:
        {
          setLEDcol(203, 207, colorRGB);  // ACHTE
          if (testPrintTimeTexts == 1) Serial.print("ACHTE ");
          break;
        }
      case 9:
        {
          setLEDcol(192, 196, colorRGB);  // NEUNE
          if (testPrintTimeTexts == 1) Serial.print("NEUNE ");
          break;
        }
      case 10:
        {
          setLEDcol(96, 100, colorRGB);  // ZEHNE (Stunden)
          if (testPrintTimeTexts == 1) Serial.print("ZEHNE ");
          break;
        }
      case 11:
        {
          setLEDcol(232, 235, colorRGB);  // ELFE
          if (testPrintTimeTexts == 1) Serial.print("ELFE ");
          break;
        }
      case 12:
        {
          setLEDcol(197, 202, colorRGB);  // ZWÖLFE
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLFE ");
          break;
        }
    }
  }

  // ########################################################### BAVARIAN:
  if (langLEDlayout == 9) {  // BAVARIAN:

    // ES IS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(9, 10, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES IS ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(33, 36, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }

    // VIERTL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(37, 42, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTL ");
    }

    // ZWANZIG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(72, 78, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZWANZIG ");
    }

    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(44, 47, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }

    // NOCH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(64, 67, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NOCH ");
    }

    // VOA:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(68, 70, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOA ");
    }

    // HOIBE:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(107, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HOIBE ");
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }

    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(102, 105, colorRGB);  // OANS
            if (testPrintTimeTexts == 1) Serial.print("OANS ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(161, 164, colorRGB);  // ZWOA
          if (testPrintTimeTexts == 1) Serial.print("ZWOA ");
          break;
        }
      case 3:
        {
          setLEDcol(165, 168, colorRGB);  // DREI
          if (testPrintTimeTexts == 1) Serial.print("DREI ");
          break;
        }
      case 4:
        {
          if (iMinute < 5) {
            setLEDcol(194, 196, colorRGB);  // VIA
            if (testPrintTimeTexts == 1) Serial.print("VIA ");
          } else {
            setLEDcol(192, 196, colorRGB);  // VIARE
            if (testPrintTimeTexts == 1) Serial.print("VIARE ");
          }
          break;
        }
      case 5:
        {
          if (iMinute < 5) {
            setLEDcol(98, 101, colorRGB);  // FÜNF
            if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
          } else {
            setLEDcol(97, 101, colorRGB);  // FÜNFE
            if (testPrintTimeTexts == 1) Serial.print("FÜNFE ");
          }
          break;
        }
      case 6:
        {
          if (iMinute < 5) {
            setLEDcol(203, 207, colorRGB);  // SECKS
            if (testPrintTimeTexts == 1) Serial.print("SECKS ");
          } else {
            setLEDcol(202, 207, colorRGB);  // SECKSE
            if (testPrintTimeTexts == 1) Serial.print("SECKSE ");
          }
          break;
        }
      case 7:
        {
          if (iMinute < 5) {
            setLEDcol(171, 175, colorRGB);  // SIEBN
            if (testPrintTimeTexts == 1) Serial.print("SIEBN ");
          } else {
            setLEDcol(170, 175, colorRGB);  // SIEBNE
            if (testPrintTimeTexts == 1) Serial.print("SIEBNE ");
          }
          break;
        }
      case 8:
        {
          if (iMinute < 5) {
            setLEDcol(129, 132, colorRGB);  // ACHT
            if (testPrintTimeTexts == 1) Serial.print("ACHT ");
          } else {
            setLEDcol(128, 132, colorRGB);  // ACHTE
            if (testPrintTimeTexts == 1) Serial.print("ACHTE ");
          }
          break;
        }
      case 9:
        {
          if (iMinute < 5) {
            setLEDcol(198, 201, colorRGB);  // NEIN
            if (testPrintTimeTexts == 1) Serial.print("NEIN ");
          } else {
            setLEDcol(197, 201, colorRGB);  // NEINE
            if (testPrintTimeTexts == 1) Serial.print("NEINE ");
          }
          break;
        }
      case 10:
        {
          if (iMinute < 5) {
            setLEDcol(134, 137, colorRGB);  // ZEHN (Stunden)
            if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
          } else {
            setLEDcol(133, 137, colorRGB);  // ZEHNE (Stunden)
            if (testPrintTimeTexts == 1) Serial.print("ZEHNE ");
          }
          break;
        }
      case 11:
        {
          if (iMinute < 5) {
            setLEDcol(237, 239, colorRGB);  // EJF
            if (testPrintTimeTexts == 1) Serial.print("EJF ");
          } else {
            setLEDcol(236, 239, colorRGB);  // EJFE
            if (testPrintTimeTexts == 1) Serial.print("EJFE ");
          }
          break;
        }
      case 12:
        {
          if (iMinute < 5) {
            setLEDcol(139, 143, colorRGB);  // ZWÄIF
            if (testPrintTimeTexts == 1) Serial.print("ZWÄIF ");
          } else {
            setLEDcol(138, 143, colorRGB);  // ZWÄIFE
            if (testPrintTimeTexts == 1) Serial.print("ZWÄIFE ");
          }
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(232, 234, colorRGB);  // UAH
      if (testPrintTimeTexts == 1) Serial.print("UAH ");
    }
  }

  // ########################################################### LU:
  if (langLEDlayout == 10) {  // LUXEMBOURGISH:

    // ET ASS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(10, 12, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ET ASS ");
    }

    // FËNNEF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(42, 47, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FËNNEF ");
    }
    // VÉIREL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(36, 41, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VÉIREL ");
    }
    // ZÉNG: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(32, 35, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZÉNG ");
    }
    // ZWANZEG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(72, 78, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZWANZEG ");
    }
    // OP:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(65, 66, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("OP ");
    }
    // VIR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(68, 70, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIR ");
    }
    // HALLWER:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(105, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HALLWER ");
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(137, 139, colorRGB);  // ENG
            if (testPrintTimeTexts == 1) Serial.print("ENG ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(199, 202, colorRGB);  // ZWOU
          if (testPrintTimeTexts == 1) Serial.print("ZWOU ");
          break;
        }
      case 3:
        {
          setLEDcol(140, 143, colorRGB);  // DRÄI
          if (testPrintTimeTexts == 1) Serial.print("DRÄI ");
          break;
        }
      case 4:
        {
          setLEDcol(203, 207, colorRGB);  // VÉIER
          if (testPrintTimeTexts == 1) Serial.print("VÉIER ");
          break;
        }
      case 5:
        {
          setLEDcol(160, 165, colorRGB);  // FËNNEF
          if (testPrintTimeTexts == 1) Serial.print("FËNNEF ");
          break;
        }
      case 6:
        {
          setLEDcol(166, 170, colorRGB);  // SECHS
          if (testPrintTimeTexts == 1) Serial.print("SECHS ");
          break;
        }
      case 7:
        {
          setLEDcol(99, 103, colorRGB);  // SIWEN
          if (testPrintTimeTexts == 1) Serial.print("SIWEN ");
          break;
        }
      case 8:
        {
          setLEDcol(171, 175, colorRGB);  // AACHT
          if (testPrintTimeTexts == 1) Serial.print("AACHT ");
          break;
        }
      case 9:
        {
          setLEDcol(96, 99, colorRGB);  // NÉNG
          if (testPrintTimeTexts == 1) Serial.print("NÉNG ");
          break;
        }
      case 10:
        {
          setLEDcol(133, 136, colorRGB);  // ZÉNG (Stonnen)
          if (testPrintTimeTexts == 1) Serial.print("ZÉNG ");
          break;
        }
      case 11:
        {
          setLEDcol(128, 132, colorRGB);  // ELLEF
          if (testPrintTimeTexts == 1) Serial.print("ELLEF ");
          break;
        }
      case 12:
        {
          setLEDcol(192, 198, colorRGB);  // ZWIELEF
          if (testPrintTimeTexts == 1) Serial.print("ZWIELEF ");
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(236, 239, colorRGB);  // AUER
      if (testPrintTimeTexts == 1) Serial.print("AUER ");
    }
  }

  // ########################################################### EAST GERMAN:
  if (langLEDlayout == 11) {  // EAST GERMAN:

    // ES IST:
    setLEDcol(7, 8, colorRGB);
    setLEDcol(45, 47, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES IST ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(32, 35, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }
    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 4) || (minDiv == 8) || (minDiv == 10)) {
      setLEDcol(40, 43, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }
    // VIERTEL:
    if (minDiv == 3) {
      setLEDcol(64, 70, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTEL ");
    }
    // DREIVIERTEL:
    if (minDiv == 9) {
      setLEDcol(64, 74, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("DREIVIERTEL ");
    }
    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(108, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }
    // VOR:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(76, 78, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }
    // HALB:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(103, 106, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
    }
    // UM:
    if ((minDiv == 0) || (minDiv == 1) || (minDiv == 2) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(101, 102, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("UM ");
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 15 hour needs to be counted up:
    // viertel 2 = 13:15
    if (iMinute >= 15) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }

    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(168, 171, colorRGB);  // EINS
            if (testPrintTimeTexts == 1) Serial.print("EINS ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(172, 175, colorRGB);  // ZWEI
          if (testPrintTimeTexts == 1) Serial.print("ZWEI ");
          break;
        }
      case 3:
        {
          setLEDcol(192, 195, colorRGB);  // DREI
          if (testPrintTimeTexts == 1) Serial.print("DREI ");
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // VIER
          if (testPrintTimeTexts == 1) Serial.print("VIER ");
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // FUENF
          if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
          break;
        }
      case 6:
        {
          setLEDcol(164, 168, colorRGB);  // SECHS
          if (testPrintTimeTexts == 1) Serial.print("SECHS ");
          break;
        }
      case 7:
        {
          setLEDcol(138, 143, colorRGB);  // SIEBEN
          if (testPrintTimeTexts == 1) Serial.print("SIEBEN ");
          break;
        }
      case 8:
        {
          setLEDcol(199, 202, colorRGB);  // ACHT
          if (testPrintTimeTexts == 1) Serial.print("ACHT ");
          break;
        }
      case 9:
        {
          setLEDcol(96, 99, colorRGB);  // NEUN
          if (testPrintTimeTexts == 1) Serial.print("NEUN ");
          break;
        }
      case 10:
        {
          setLEDcol(133, 136, colorRGB);  // ZEHN (Stunden)
          if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
          break;
        }
      case 11:
        {
          setLEDcol(196, 198, colorRGB);  // ELF
          if (testPrintTimeTexts == 1) Serial.print("ELF ");
          break;
        }
      case 12:
        {
          setLEDcol(203, 207, colorRGB);  // ZWÖLF
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLF ");
          break;
        }
    }
  }

  if (ntpSyncing) showNtpSyncLED(strip.Color(255, 0, 0));
  showStripSafe();
}


// ###########################################################################################################################################
// # Display extra minutes function:
// ###########################################################################################################################################
void showMinutes(int minutes) {
  int minMod = (minutes % 5);
  // Serial.println(minMod);

  // ##################################################### DE:
  if (langLEDlayout == 0) {  // DE:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(226, 231, colorRGB);  // MINUTE
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
    }
  }

  // ##################################################### EN:
  if (langLEDlayout == 1) {  // EN:
    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(226, 231, colorRGB);  // MINUTE
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
    }
  }

  // ##################################################### NL:
  if (langLEDlayout == 2) {  // NL:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(225, 231, colorRGB);  // MINUTEN (set to this on request, because there was no space for the extra word "minuut")
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
    }
  }

  // ##################################################### SWE:
  if (langLEDlayout == 3) {  // SWE:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(227, 231, colorRGB);  // MINUT
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTER
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTER
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTER
          break;
        }
    }
  }

  // ##################################################### IT:
  if (langLEDlayout == 4) {  // IT:

    switch (minMod) {
      case 1:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(230, 230, colorRGB);  // 1
          setLEDcol(225, 225, colorRGB);  // M
          break;
        }
      case 2:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 2
          setLEDcol(225, 225, colorRGB);  // M
          break;
        }
      case 3:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(228, 228, colorRGB);  // 3
          setLEDcol(225, 225, colorRGB);  // M
          break;
        }
      case 4:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(227, 227, colorRGB);  // 4
          setLEDcol(225, 225, colorRGB);  // M
          break;
        }
    }
  }

  // ##################################################### FR:
  if (langLEDlayout == 5) {  // FR:
    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(226, 231, colorRGB);  // MINUTE
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
    }
  }

  // ##################################################### GSW:
  if (langLEDlayout == 6) {  // GSW:

    switch (minMod) {
      case 1:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 1
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 2:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(228, 228, colorRGB);  // 2
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 3:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(227, 227, colorRGB);  // 3
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 4:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(226, 226, colorRGB);  // 4
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
    }
  }

  // ########################################################### CN:
  if (langLEDlayout == 7) {  // CN:
    switch (minMod) {
      case 1:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(199, 199, colorRGB);  // 一
          setLEDcol(194, 195, colorRGB);  // 分钟
          break;
        }
      case 2:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(198, 198, colorRGB);  // 二
          setLEDcol(194, 195, colorRGB);  // 分钟
          break;
        }
      case 3:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(197, 197, colorRGB);  // 三
          setLEDcol(194, 195, colorRGB);  // 分钟
          break;
        }
      case 4:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(196, 196, colorRGB);  // 四
          setLEDcol(194, 195, colorRGB);  // 分钟
          break;
        }
    }
  }

  // ##################################################### SWABIAN:
  if (langLEDlayout == 8) {  // SWABIAN:

    switch (minMod) {
      case 1:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 1
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 2:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(228, 228, colorRGB);  // 2
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 3:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(227, 227, colorRGB);  // 3
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 4:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(226, 226, colorRGB);  // 4
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
    }
  }

  // ##################################################### BAVARIAN:
  if (langLEDlayout == 9) {  // BAVARIAN:

    switch (minMod) {
      case 1:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 1
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 2:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(228, 228, colorRGB);  // 2
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 3:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(227, 227, colorRGB);  // 3
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 4:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(226, 226, colorRGB);  // 4
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
    }
  }

  // ##################################################### LUXEMBOURGISH:
  if (langLEDlayout == 10) {  // LUXEMBOURGISH:

    switch (minMod) {
      case 1:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(232, 232, colorRGB);  // 1
          setLEDcol(227, 225, colorRGB);  // MIN
          break;
        }
      case 2:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(231, 231, colorRGB);  // 2
          setLEDcol(227, 225, colorRGB);  // MIN
          break;
        }
      case 3:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(230, 230, colorRGB);  // 3
          setLEDcol(227, 225, colorRGB);  // MIN
          break;
        }
      case 4:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 4
          setLEDcol(227, 225, colorRGB);  // MIN
          break;
        }
    }
  }

  // ##################################################### EAST GERMAN:
  if (langLEDlayout == 11) {  // EAST GERMAN:

    switch (minMod) {
      case 1:
        {
          setLEDcol(239, 239, colorRGB);  // +
          setLEDcol(237, 237, colorRGB);  // 1
          setLEDcol(227, 232, colorRGB);  // MINUTE
          break;
        }
      case 2:
        {
          setLEDcol(239, 239, colorRGB);  // +
          setLEDcol(236, 236, colorRGB);  // 2
          setLEDcol(226, 232, colorRGB);  // MINUTEN
          break;
        }
      case 3:
        {
          setLEDcol(239, 239, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 3
          setLEDcol(226, 232, colorRGB);  // MINUTEN
          break;
        }
      case 4:
        {
          setLEDcol(239, 239, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 4
          setLEDcol(226, 232, colorRGB);  // MINUTEN
          break;
        }
    }
  }
}


// ###########################################################################################################################################
// # Background color function: SET ALL LEDs OFF
// ###########################################################################################################################################
void back_color() {
  uint32_t c0 = strip.Color(redVal_back, greenVal_back, blueVal_back);  // Background color
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, c0);
  }
}


// ###########################################################################################################################################
// # Actual function, which controls 1/0 of the LED:
// ###########################################################################################################################################
void setLED(int ledNrFrom, int ledNrTo, int switchOn) {
  if (ledNrFrom > ledNrTo) {
    setLED(ledNrTo, ledNrFrom, switchOn);  // Sets LED numbers in correct order
  } else {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS)) {
        strip.setPixelColor(i, strip.Color(redVal_time, greenVal_time, blueVal_time));
        int pairedLED = getPairedLED(i);
        if ((pairedLED >= 0) && (pairedLED < NUMPIXELS))
          strip.setPixelColor(pairedLED, strip.Color(redVal_time, greenVal_time, blueVal_time));
      }
    }
  }
  if (switchOn == 0) {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS))
        strip.setPixelColor(i, strip.Color(0, 0, 0));  // Switch LEDs off
    }
  }
}


// ###########################################################################################################################################
// # NTP time functions:
// ###########################################################################################################################################
void configNTPTime() {
  initTime(Timezone);
  printLocalTime();
}
// ###########################################################################################################################################
void setTimezone(String timezone) {
  Serial.printf("Setting timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
  delay(1000);
}
// ###########################################################################################################################################
void initTime(String timezone) {
  Serial.println("Setting up time from: " + NTPserver);

  // Flash TIME LEDs 3x blue:
  for (int i = 0; i < 3; i++) {
    ClearDisplay();
    showStrip();
    delay(500);

    if (langLEDlayout == 0) {  // DE:
      setLEDcol(1, 4, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 1) {  // EN:
      setLEDcol(33, 36, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 2) {  // NL:
      setLEDcol(69, 72, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 3) {  // SWE:
      setLEDcol(96, 98, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 4) {                       // IT:
      setLEDcol(111, 111, strip.Color(0, 0, 255));  // T
      setLEDcol(96, 97, strip.Color(0, 0, 255));    // EM
      setLEDcol(131, 131, strip.Color(0, 0, 255));  // P
      setLEDcol(234, 234, strip.Color(0, 0, 255));  // O
    }

    if (langLEDlayout == 5) {                       // FR:
      setLEDcol(10, 10, strip.Color(0, 0, 255));    // T
      setLEDcol(41, 42, strip.Color(0, 0, 255));    // EM
      setLEDcol(105, 105, strip.Color(0, 0, 255));  // p
      setLEDcol(128, 128, strip.Color(0, 0, 255));  // S
    }

    if (langLEDlayout == 6) {                   // GSW:
      setLEDcol(0, 3, strip.Color(0, 0, 255));  // ZIIT
    }

    if (langLEDlayout == 7) {  // CN:
      setLEDcol(40, 41, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 8) {                       // SWABIAN:
      setLEDcol(73, 74, strip.Color(0, 0, 255));    // ZE
      setLEDcol(131, 131, strip.Color(0, 0, 255));  // I
      setLEDcol(204, 204, strip.Color(0, 0, 255));  // T
    }

    if (langLEDlayout == 9) {                   // BAVARIAN:
      setLEDcol(5, 8, strip.Color(0, 0, 255));  // ZEID
    }

    if (langLEDlayout == 10) {                  // LUXEMBOURGISH:
      setLEDcol(1, 4, strip.Color(0, 0, 255));  // ZÄIT
    }

    if (langLEDlayout == 11) {                  // EAST GERMAN:
      setLEDcol(1, 4, strip.Color(0, 0, 255));  // ZEIT
    }

    showStrip();
    delay(500);
  }


  struct tm timeinfo;
  configTime(0, 0, NTPserver.c_str());
  delay(500);

  while (!getLocalTime(&timeinfo)) {
    delay(1000);

    if (langLEDlayout == 0) {  // DE:
      setLEDcol(1, 4, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 1) {  // EN:
      setLEDcol(33, 36, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 2) {  // NL:
      setLEDcol(69, 72, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 3) {  // SWE:
      setLEDcol(96, 98, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 4) {                       // IT:
      setLEDcol(111, 111, strip.Color(255, 0, 0));  // T
      setLEDcol(96, 97, strip.Color(255, 0, 0));    // EM
      setLEDcol(131, 131, strip.Color(255, 0, 0));  // P
      setLEDcol(234, 234, strip.Color(255, 0, 0));  // O
    }

    if (langLEDlayout == 5) {                       // FR:
      setLEDcol(10, 10, strip.Color(255, 0, 0));    // T
      setLEDcol(41, 42, strip.Color(255, 0, 0));    // EM
      setLEDcol(105, 105, strip.Color(255, 0, 0));  // p
      setLEDcol(128, 128, strip.Color(255, 0, 0));  // S
    }

    if (langLEDlayout == 6) {                   // GSW:
      setLEDcol(0, 3, strip.Color(255, 0, 0));  // ZIIT
    }

    if (langLEDlayout == 7) {  // CN:
      setLEDcol(40, 41, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 8) {                       // SWABIAN:
      setLEDcol(73, 74, strip.Color(255, 0, 0));    // ZE
      setLEDcol(131, 131, strip.Color(255, 0, 0));  // I
      setLEDcol(204, 204, strip.Color(255, 0, 0));  // T
    }

    if (langLEDlayout == 9) {                   // BAVARIAN:
      setLEDcol(5, 8, strip.Color(255, 0, 0));  // ZEID
    }

    if (langLEDlayout == 10) {                  // LUXEMBOURGISH:
      setLEDcol(1, 4, strip.Color(255, 0, 0));  // ZÄIT
    }

    if (langLEDlayout == 11) {                  // EAST GERMAN:
      setLEDcol(1, 4, strip.Color(255, 0, 0));  // ZEIT
    }

    showStrip();
    delay(250);
    ClearDisplay();
    delay(250);
    ResetTextLEDs(strip.Color(0, 255, 0));
    Serial.println("! Failed to obtain time - Time server could not be reached ! --> RESTART THE DEVICE NOW...");
    delay(250);
    ESP.restart();
  }

  // Time successfully received:
  ClearDisplay();
  if (langLEDlayout == 0) {  // DE:
    setLEDcol(1, 4, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 1) {  // EN:
    setLEDcol(33, 36, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 2) {  // NL:
    setLEDcol(69, 72, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 3) {  // SWE:
    setLEDcol(96, 98, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 4) {                       // IT:
    setLEDcol(111, 111, strip.Color(0, 255, 0));  // T
    setLEDcol(96, 97, strip.Color(0, 255, 0));    // EM
    setLEDcol(131, 131, strip.Color(0, 255, 0));  // P
    setLEDcol(234, 234, strip.Color(0, 255, 0));  // O
  }

  if (langLEDlayout == 5) {                       // FR:
    setLEDcol(10, 10, strip.Color(0, 255, 0));    // T
    setLEDcol(41, 42, strip.Color(0, 255, 0));    // EM
    setLEDcol(105, 105, strip.Color(0, 255, 0));  // p
    setLEDcol(128, 128, strip.Color(0, 255, 0));  // S
  }

  if (langLEDlayout == 6) {                   // GSW:
    setLEDcol(0, 3, strip.Color(0, 255, 0));  // ZIIT
  }

  if (langLEDlayout == 7) {  // CN:
    setLEDcol(40, 41, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 8) {                       // SWABIAN:
    setLEDcol(73, 74, strip.Color(0, 255, 0));    // ZE
    setLEDcol(131, 131, strip.Color(0, 255, 0));  // I
    setLEDcol(204, 204, strip.Color(0, 255, 0));  // T
  }

  if (langLEDlayout == 9) {                   // BAVARIAN:
    setLEDcol(5, 8, strip.Color(0, 255, 0));  // ZEID
  }

  if (langLEDlayout == 10) {                  // LUXEMBOURGISH:
    setLEDcol(1, 4, strip.Color(0, 255, 0));  // ZÄIT
  }

  if (langLEDlayout == 11) {                  // EAST GERMAN:
    setLEDcol(1, 4, strip.Color(0, 255, 0));  // ZEIT
  }


  showStrip();
  delay(1000);
  Serial.println("Got the time from NTP server: " + NTPserver);
  setTimezone(timezone);
}
// ###########################################################################################################################################
// # Show the language-specific "ZEIT" word with a given color (used during NTP resync)
// ###########################################################################################################################################
void showNtpSyncLED(uint32_t color) {
  if (langLEDlayout == 0)  setLEDcol(1, 4, color);    // DE:   ZEIT
  if (langLEDlayout == 1)  setLEDcol(33, 36, color);  // EN:   TIME
  if (langLEDlayout == 2)  setLEDcol(69, 72, color);  // NL:   TIJD
  if (langLEDlayout == 3)  setLEDcol(96, 98, color);  // SWE:  TID
  if (langLEDlayout == 4) { setLEDcol(111,111,color); setLEDcol(96,97,color); setLEDcol(131,131,color); setLEDcol(234,234,color); }  // IT: TEMP
  if (langLEDlayout == 5) { setLEDcol(10,10,color);   setLEDcol(41,42,color); setLEDcol(105,105,color); setLEDcol(128,128,color); }  // FR: TEMPS
  if (langLEDlayout == 6)  setLEDcol(0, 3, color);    // GSW:  ZIIT
  if (langLEDlayout == 7)  setLEDcol(40, 41, color);  // CN
  if (langLEDlayout == 8) { setLEDcol(73,74,color); setLEDcol(131,131,color); setLEDcol(204,204,color); }  // SCHWB: ZEIT
  if (langLEDlayout == 9)  setLEDcol(5, 8, color);    // BAY:  ZEID
  if (langLEDlayout == 10) setLEDcol(1, 4, color);    // LTZ:  ZÄIT
  if (langLEDlayout == 11) setLEDcol(1, 4, color);    // EAST: ZEIT
}
// ###########################################################################################################################################
void printLocalTime() {
  static unsigned long lastCall = 0;
  if (!animationRunning && millis() - lastCall < 1000) return;
  lastCall = millis();
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  iStartTime = String(timeStringBuff);
  iHour = timeinfo.tm_hour;
  iMinute = timeinfo.tm_min;
  iSecond = timeinfo.tm_sec;
  if (ShowTimeEachSecond == 1) {
    Serial.print("Current time: ");
    Serial.print(iHour);
    Serial.print(":");
    Serial.print(iMinute);
    Serial.print(":");
    Serial.println(iSecond);
  }

  // Daily NTP resync at configured hour/minute
  static int lastNtpDay = -1;
  static unsigned long lastNtpAttempt = 0;

  bool triggerSync = (iHour == ntpSyncHour && iMinute == ntpSyncMinute && iSecond < 5 && timeinfo.tm_yday != lastNtpDay);
  bool retrySync   = (!ntpSyncing && lastNtpDay != timeinfo.tm_yday
                      && lastNtpAttempt > 0 && millis() - lastNtpAttempt >= 30UL * 60 * 1000);

  if (triggerSync || retrySync) {
    lastNtpAttempt = millis();
    ntpSyncing = true;
    updatenow = true;
    configTime(0, 0, NTPserver.c_str());
    setenv("TZ", Timezone.c_str(), 1); tzset();
    Serial.println(triggerSync ? "# Daily NTP resync triggered" : "# NTP resync retry");
  }

  if (ntpSyncing) {
    if (millis() - lastNtpAttempt >= 2000) {
      sntp_sync_status_t syncStatus = sntp_get_sync_status();
      if (syncStatus == SNTP_SYNC_STATUS_COMPLETED) {
        ntpSyncing = false;
        lastNtpDay = timeinfo.tm_yday;
        updatenow = true;
        Serial.println("# NTP resync successful");
      }
    }
    if (millis() - lastNtpAttempt >= 2UL * 60 * 1000) {
      ntpSyncing = false;
      Serial.println("# NTP resync timed out");
    }
  }
}
// ###########################################################################################################################################
void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst) {
  struct tm tm;
  tm.tm_year = yr - 1900;  // Set date
  tm.tm_mon = month - 1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;  // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}
// ###########################################################################################################################################


// ###########################################################################################################################################
// # HTML command web server:
// ###########################################################################################################################################
int ew = 0;  // Current extra word
String ledstatus = "ON";
void handleLEDupdate() {  // LED server pages urls:

  ledserver.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {  // Show a manual how to use these links:
    String message = "WordClock web configuration and querry options examples:\n\n";
    message = message + "General:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023 --> Shows this text\n\n";
    message = message + "Get the status of the WordClock LEDs:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/status --> Show the status of the LEDs (0 = OFF and 1 = ON).\n\n";
    message = message + "Turn the LEDs OFF or ON:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=0 --> LED intensity is set to OFF which will turn the display off.\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=1 --> LED intensity is set to ON which will turn the display on again...\n";
    request->send(200, "text/plain", message);
  });

  ledserver.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {  // Configure background and time texts color and intensity:
    int paramsNr = request->params();
    // Serial.println(paramsNr);
    for (int i = 0; i < paramsNr; i++) {
      AsyncWebParameter* p = request->getParam(i);
      // Serial.print("Param name: ");
      // Serial.println(p->name());
      // Serial.print("Param value: ");
      // Serial.println(p->value());
      // Serial.println("------------------");
      if ((p->value().toInt() >= 0) && (p->value().toInt() <= 1)) {
        if ((String(p->name()) == "LEDs") && (p->value().toInt() == 0)) {
          set_web_intensity = 1;
          ledstatus = "OFF";
        }
        if ((String(p->name()) == "LEDs") && (p->value().toInt() == 1)) {
          set_web_intensity = 0;
          ledstatus = "ON";
        }
        changedvalues = true;
        updatenow = true;
      } else {
        request->send(200, "text/plain", "INVALID VALUES - MUST BE BETWEEN 0 and 1");
      }
    }
    request->send(200, "text/plain", "WordClock LEDs set to: " + ledstatus);
  });

  ledserver.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {  // Show the status of all extra words and the color for the background and time texts:
    String message = ledstatus;
    request->send(200, "text/plain", message);
  });

  ledserver.begin();
}


// ###########################################################################################################################################
// # Startup LED test function
// ###########################################################################################################################################
void callStartText() {
  Serial.println("Show 'WordClock' startup text...");
  uint32_t c = strip.Color(redVal_time, greenVal_time, blueVal_time);
  int TextWait = 500;
  showtext('W', TextWait, c);
  showtext('O', TextWait, c);
  showtext('R', TextWait, c);
  showtext('D', TextWait, c);
  showtext('C', TextWait, c);
  showtext('L', TextWait, c);
  showtext('O', TextWait, c);
  showtext('C', TextWait, c);
  showtext('K', TextWait, c);
}


// ###########################################################################################################################################
// # Startup WiFi text function:
// ###########################################################################################################################################
void SetWLAN(uint32_t color) {
  // if (debugtexts == 1) Serial.println("Show text WLAN/WIFI...");
  ClearDisplay();

  if (langLEDlayout == 0) {  // DE:
    setLEDcol(5, 8, color);  // WIFI
  }

  if (langLEDlayout == 1) {   // EN:
    setLEDcol(7, 10, color);  // WIFI
  }

  if (langLEDlayout == 2) {    // NL:
    setLEDcol(75, 78, color);  // WIFI
  }

  if (langLEDlayout == 3) {  // SWE:
    setLEDcol(0, 3, color);  // WIFI
  }

  if (langLEDlayout == 4) {      // IT:
    setLEDcol(233, 233, color);  // W
    setLEDcol(231, 231, color);  // I
    setLEDcol(226, 226, color);  // F
    setLEDcol(224, 224, color);  // I
  }

  if (langLEDlayout == 5) {      // FR:
    setLEDcol(239, 239, color);  // W
    setLEDcol(237, 237, color);  // I
    setLEDcol(232, 232, color);  // F
    setLEDcol(224, 224, color);  // I
  }

  if (langLEDlayout == 6) {   // GSW:
    setLEDcol(7, 10, color);  // WIFI
  }

  if (langLEDlayout == 7) {    // CN:
    setLEDcol(42, 43, color);  // WIFI
  }

  if (langLEDlayout == 8) {    // SWABIAN:
    setLEDcol(12, 13, color);  // WI
    setLEDcol(7, 8, color);    // FI
  }

  if (langLEDlayout == 9) {    // BAVARIAN:
    setLEDcol(10, 13, color);  // WIFI
  }

  if (langLEDlayout == 10) {  // LUXEMBOURGISH:
    setLEDcol(5, 8, color);   // WIFI
  }

  if (langLEDlayout == 11) {   // EAST GERMAN:
    setLEDcol(36, 39, color);  // WLAN
  }

  showStrip();
}


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>WordClock &middot; Setup</title>
<style>
:root{--bg:#f7f7f4;--s:#fff;--l:#e9e9e4;--ink:#14140f;--ink2:#555550;--ink3:#8a8a82;--a:#65d435;--r:10px}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,'Segoe UI',sans-serif;background:var(--bg);color:var(--ink);min-height:100vh;display:flex;align-items:center;justify-content:center;padding:24px;-webkit-font-smoothing:antialiased}
.card{background:var(--s);border:1px solid var(--l);border-radius:var(--r);padding:40px 32px;width:100%;max-width:420px;text-align:center}
.logo{font-size:21px;font-weight:700;letter-spacing:-.03em;margin-bottom:4px;display:flex;align-items:center;justify-content:center;gap:8px}
.dot{width:7px;height:7px;border-radius:50%;background:var(--a);flex-shrink:0}
.tag{font-size:11px;color:var(--ink3);letter-spacing:.08em;text-transform:uppercase;margin-bottom:36px}
h1{font-size:20px;font-weight:600;letter-spacing:-.02em;margin-bottom:10px}
p{font-size:13.5px;color:var(--ink2);line-height:1.65;margin-bottom:32px}
.btn{display:inline-block;padding:11px 28px;background:var(--a);color:#0e2a05;border:none;border-radius:7px;font-size:13.5px;font-weight:600;cursor:pointer;transition:opacity .15s;font-family:inherit;letter-spacing:.01em}
.btn:hover{opacity:.85}
.btn:disabled{opacity:.45;cursor:not-allowed}
</style>
</head>
<body>
<div class="card">
  <div class="logo"><span class="dot"></span>WordClock</div>
  <div class="tag">Initial Setup</div>
  <h1>Welcome</h1>
  <p>Connect your WordClock to your local WiFi network for online mode (NTP time sync, weather, etc.).</p>
  <form action="/start" name="myForm" style="margin-bottom:16px">
    <button class="btn" id="submitButton" type="submit" onclick="this.disabled=true;this.textContent='Loading...';setTimeout(function(){document.forms['myForm'].submit();},100)">Configure WiFi &rarr;</button>
  </form>
  <div style="height:1px;background:#e9e9e4;margin:20px 0"></div>
  <h1 style="margin-bottom:10px">Offline Mode</h1>
  <p style="margin-bottom:8px">No internet required. Time is set manually via the web interface. After switching, connect to:</p>
  <p style="margin-bottom:4px"><strong>WiFi:</strong> %OFFLINE_SSID%</p>
  <p style="margin-bottom:20px"><strong>Password:</strong> %OFFLINE_PW%</p>
  <form action="/offline">
    <button class="btn" style="background:#e9e9e4;color:#14140f">Use Offline Mode &rarr;</button>
  </form>
</div>
</body>
</html>
)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>WordClock &middot; Setup</title>
<style>
:root{--bg:#f7f7f4;--s:#fff;--l:#e9e9e4;--ink:#14140f;--ink2:#555550;--ink3:#8a8a82;--a:#65d435;--danger:#d24a3c;--r:10px}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,'Segoe UI',sans-serif;background:var(--bg);color:var(--ink);min-height:100vh;display:flex;flex-direction:column;align-items:center;padding:32px 16px 60px;-webkit-font-smoothing:antialiased}
.logo{font-size:20px;font-weight:700;letter-spacing:-.03em;margin-bottom:4px;display:flex;align-items:center;gap:8px}
.dot{width:7px;height:7px;border-radius:50%;background:var(--a);flex-shrink:0}
.tag{font-size:11px;color:var(--ink3);letter-spacing:.08em;text-transform:uppercase;margin-bottom:24px}
.card{background:var(--s);border:1px solid var(--l);border-radius:var(--r);padding:28px 28px 32px;width:100%;max-width:460px}
h1{font-size:17px;font-weight:600;letter-spacing:-.01em;margin-bottom:22px;padding-bottom:16px;border-bottom:1px solid var(--l)}
.field{margin-bottom:16px}
.lbl{display:block;font-size:11.5px;font-weight:500;color:var(--ink3);letter-spacing:.04em;text-transform:uppercase;margin-bottom:6px}
input,select,#mySSIDSelect{width:100%;padding:9px 11px;border:1px solid var(--l);border-radius:7px;font-size:13.5px;font-family:inherit;color:var(--ink);background:var(--bg);outline:none;transition:border-color .15s,box-shadow .15s;-webkit-appearance:none;appearance:none}
input:focus,select:focus,#mySSIDSelect:focus{border-color:var(--a);background:#fff;box-shadow:0 0 0 3px rgba(101,212,53,.15)}
.hr{height:1px;background:var(--l);margin:20px 0}
.error{display:none;font-size:13px;color:var(--danger);background:#fdf2f1;border:1px solid rgba(210,74,60,.25);border-radius:7px;padding:9px 13px;margin-bottom:14px}
.btn{width:100%;padding:12px;background:var(--a);color:#0e2a05;border:none;border-radius:7px;font-size:13.5px;font-weight:600;cursor:pointer;font-family:inherit;transition:opacity .15s;margin-top:6px;letter-spacing:.01em}
.btn:hover{opacity:.85}
.btn:disabled{opacity:.45;cursor:not-allowed}
p{font-size:13px;margin-bottom:12px}
p label{font-size:11.5px;font-weight:500;color:var(--ink3);letter-spacing:.04em;text-transform:uppercase;display:block;margin-bottom:6px}
</style>
<script>
function updateSSIDInput(){var s=document.getElementById("mySSIDSelect");if(s&&s.options.length>0)document.getElementById("mySSID").value=s.options[s.selectedIndex].value;}
function validateForm(){var e=document.querySelector('.error');e.style.display='none';e.innerHTML='';if(!document.forms["myForm"]["mySSID"].value){e.innerHTML="WiFi SSID must be set.";e.style.display='block';return false;}if(!document.forms["myForm"]["myPW"].value){e.innerHTML="WiFi password must be set.";e.style.display='block';return false;}return true;}
function disableButtonAndSubmit(){if(validateForm()){var b=document.getElementById("submitButton");b.textContent='Saving...';b.disabled=true;setTimeout(function(){document.forms["myForm"].submit();},1000);}}
window.onload=function(){var s=document.getElementById("mySSIDSelect");if(s)s.addEventListener('change',updateSSIDInput);};
</script>
</head>
<body>
<div class="logo"><span class="dot"></span>WordClock</div>
<div class="tag">Initial Setup</div>
<div class="card">
  <h1>WiFi &amp; Language</h1>
  <form action="/get" name="myForm" onsubmit="return validateForm()">
    <!-- Select element will be dynamically added here -->
    <div class="field">
      <span class="lbl">WiFi Network (SSID)</span>
      <input id="mySSID" name="mySSID" placeholder="Network name" autocomplete="off" autocorrect="off" autocapitalize="none" spellcheck="false">
    </div>
    <div class="field">
      <span class="lbl">WiFi Password</span>
      <input type="text" id="myPW" name="myPW" placeholder="Password" autocorrect="off" spellcheck="false">
    </div>
    <div class="hr"></div>
    <div class="field">
      <span class="lbl">Firmware</span>
      <div style="font-size:13px;color:var(--ink2)">Version: <b>FW_VERSION</b> &nbsp;|&nbsp; Language: <b>LANG_NAME</b></div>
    </div>
    <div class="error"></div>
    <button class="btn" id="submitButton" type="submit" onclick="disableButtonAndSubmit()">Save &amp; Connect &rarr;</button>
  </form>
</div>
</body>
</html>
)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char saved_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>WordClock &middot; Setup</title>
<style>
:root{--bg:#f7f7f4;--s:#fff;--l:#e9e9e4;--ink:#14140f;--ink2:#555550;--ink3:#8a8a82;--a:#65d435;--r:10px}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,'Segoe UI',sans-serif;background:var(--bg);color:var(--ink);min-height:100vh;display:flex;align-items:center;justify-content:center;padding:24px;-webkit-font-smoothing:antialiased}
.card{background:var(--s);border:1px solid var(--l);border-radius:var(--r);padding:40px 32px;width:100%;max-width:420px;text-align:center}
.logo{font-size:20px;font-weight:700;letter-spacing:-.03em;margin-bottom:4px;display:flex;align-items:center;justify-content:center;gap:8px}
.dot{width:7px;height:7px;border-radius:50%;background:var(--a);flex-shrink:0}
.tag{font-size:11px;color:var(--ink3);letter-spacing:.08em;text-transform:uppercase;margin-bottom:28px}
.icon{width:52px;height:52px;border-radius:50%;display:flex;align-items:center;justify-content:center;margin:0 auto 18px;font-size:22px}
.icon-wait{background:rgba(200,200,200,.2);border:1px solid var(--l);color:var(--ink3);animation:spin 1.5s linear infinite}
.icon-ok{background:rgba(101,212,53,.15);border:1px solid rgba(101,212,53,.3);color:var(--a)}
.icon-err{background:rgba(210,74,60,.1);border:1px solid rgba(210,74,60,.3);color:#d24a3c}
@keyframes spin{to{transform:rotate(360deg)}}
h1{font-size:19px;font-weight:600;letter-spacing:-.02em;margin-bottom:12px}
p{font-size:13.5px;color:var(--ink2);line-height:1.65;margin-bottom:10px}
.ip{font-size:22px;font-weight:700;font-family:monospace;color:var(--ink);background:var(--l);border-radius:8px;padding:10px 16px;margin:14px 0;display:inline-block}
.link{color:var(--a);font-size:13px}
.note{margin-top:16px;font-size:12px;color:var(--ink3)}
.err{color:#d24a3c;font-size:13px}
</style>
</head>
<body>
<div class="card">
  <div class="logo"><span class="dot"></span>WordClock</div>
  <div class="tag" id="tag">Verbinde...</div>
  <div class="icon icon-wait" id="icon">&#8987;</div>
  <h1 id="title">Verbinde mit WiFi</h1>
  <div id="msg">
    <p>Bitte warten &mdash; WordClock verbindet sich mit deinem Netzwerk.</p>
    <p class="note">Timeout in <span id="cd">60</span>s &mdash; Diese Seite aktualisiert sich automatisch.</p>
  </div>
</div>
<script>
var cd=60;var cdt=setInterval(function(){cd--;var e=document.getElementById('cd');if(e)e.textContent=cd;if(cd<=0)clearInterval(cdt);},1000);
var t=setInterval(function(){
  fetch('/api/newip').then(function(r){return r.text();}).then(function(ip){
    if(ip&&ip.length>3){
      clearInterval(t);
      if (window.AndroidApp && window.AndroidApp.saveNewIP) {
        window.AndroidApp.saveNewIP(ip);
      }
      var i=document.getElementById('icon');
      i.className='icon icon-ok';i.innerHTML='&#10003;';
      document.getElementById('tag').textContent='Verbunden!';
      document.getElementById('title').textContent='Verbindung erfolgreich!';
      document.getElementById('msg').innerHTML=
        '<p>WordClock ist jetzt erreichbar unter:</p>'+
        '<div class="ip">'+ip+'</div>'+
        '<p class="note">Nach dem Neustart erreichbar unter:</p>'+
        '<p><a class="link" href="http://'+ip+'">http://'+ip+'</a></p>'+
        '<p class="note">WordClock startet gleich neu...</p>';
    }
  }).catch(function(){});
},2000);
// Timeout nach 60s
setTimeout(function(){
  clearInterval(t);
  clearInterval(cdt);
  var i=document.getElementById('icon');
  i.className='icon icon-err';i.innerHTML='&#10007;';
  document.getElementById('tag').textContent='Fehler';
  document.getElementById('title').textContent='Verbindung fehlgeschlagen';
  document.getElementById('msg').innerHTML=
    '<p class="err">Konnte nicht verbinden. Bitte SSID und Passwort prüfen.</p>'+
    '<p class="note">Verbinde dich erneut mit dem <b>WordClock</b> Hotspot und versuche es nochmal.</p>'+
    '<br><a href="http://192.168.4.1" style="display:inline-block;margin-top:8px;padding:10px 24px;background:#65d435;color:#0e2a05;border-radius:7px;font-weight:600;font-size:13px;text-decoration:none;">&#8594; Erneut versuchen</a>';
},60000);
</script>
</body>
</html>
)rawliteral";


// ###########################################################################################################################################
// # Wifi scan function to help you to setup your WiFi connection
// ###########################################################################################################################################
String ScanWiFi() {
  String html = config_html;
  Serial.println("Scan WiFi networks - START");
  int n = WiFi.scanNetworks();
  Serial.println("WiFi scan done");
  Serial.println("Scan WiFi networks - END");
  Serial.println(" ");
  if (n > 0) {
    Serial.print(n);
    Serial.println(" WiFi networks found:");
    Serial.println(" ");
    String ssidList = "<p><label for=\"mySSISelect\">Found these networks:</label><br /><select id=\"mySSIDSelect\" name=\"mySSIDSelect\"><option value=\"\" disabled selected>Choose...</option>";
    String seenSSIDs = ",";  // Komma-separierte Liste bereits hinzugefügter SSIDs
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(ssid);
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : "*");
      // Duplikate überspringen
      if (ssid.length() == 0) continue;
      if (seenSSIDs.indexOf("," + ssid + ",") >= 0) continue;
      seenSSIDs += ssid + ",";
      ssidList += "<option value=\"" + ssid + "\">" + ssid + "</option>";
    }
    ssidList += "</select></p>";
    html.replace("<!-- Select element will be dynamically added here -->", ssidList);
  } else {
    Serial.println("No WiFi networks found");
  }
  WiFi.scanDelete();  // free scan memory and reset WiFi scan state before starting AP
  const char* langNames[] = {"Deutsch","Englisch","Niederlaendisch","Schwedisch","Italienisch","Franzoesisch","Schweizerdeutsch","Chinesisch","Schwaebisch","Bayrisch","Luxemburgisch","Ostdeutsch"};
  html.replace("FW_VERSION", String(WORD_CLOCK_VERSION));
  html.replace("LANG_NAME", String(langNames[LANGUAGE]));
  return html;
}


// ###########################################################################################################################################
// # Captive Portal by AWSW to avoid the usage of the WiFi Manager library to have more control
// ###########################################################################################################################################
const char* PARAM_INPUT_1 = "mySSID";
const char* PARAM_INPUT_2 = "myPW";
const char* PARAM_INPUT_3 = "setlanguage";
const String captiveportalURL = "http://192.168.4.1";
void CaptivePortalSetup() {
  String htmlConfigContent = ScanWiFi();
  const char* temp_ssid = "WordClock";
  const char* temp_password = "";
  WiFi.softAP(temp_ssid, temp_password);
  WiFi.setTxPower(WIFI_POWER_15dBm);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("#################################################################################################################################################################################");
  Serial.print("# Temporary WiFi access point initialized. Please connect to the WiFi access point now and set your local WiFi credentials and WordClock language. Access point name: ");
  Serial.println(temp_ssid);
  Serial.print("# In case your browser does not open the WordClock setup page automatically after connecting to the access point, please navigate to this URL manually to http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("#################################################################################################################################################################################");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    String html = String(index_html);
    html.replace("%OFFLINE_SSID%", String(Offline_SSID));
    html.replace("%OFFLINE_PW%", String(Offline_PW));
    request->send(200, "text/html", html);
  });

  server.on("/offline", HTTP_GET, [](AsyncWebServerRequest* request) {
    preferences.putUInt("UseOnlineMode", 0);
    request->send(200, "text/html", "<html><body style='font-family:system-ui;text-align:center;padding:40px'>"
      "<h2>Switching to Offline Mode...</h2>"
      "<p>Connect to WiFi: <strong>" + String(Offline_SSID) + "</strong><br>"
      "Password: <strong>" + String(Offline_PW) + "</strong></p>"
      "<p>The WordClock will restart now.</p></body></html>");
    pendingRestartAt = millis() + 1500;
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam(PARAM_INPUT_1)) {
      String newSSID = request->getParam(PARAM_INPUT_1)->value();
      String newPASS = request->getParam(PARAM_INPUT_2)->value();
      preferences.begin("wordclock", false);  // reopen in case it was closed by a previous attempt
      preferences.putString("WIFIssid", newSSID);
      preferences.putString("WIFIpass", newPASS);
      preferences.putUInt("showIPonce", 1);
      preferences.end();
      // AP bleibt aktiv, gleichzeitig ins Heimnetz verbinden
      pendingRestartAt = 0;        // cancel any pending restart from previous attempt
      wifiConnectDeadline = millis() + 60000;  // 60 second connection timeout
      WiFi.disconnect(true);       // stop previous connection attempt
      delay(200);
      WiFi.mode(WIFI_AP_STA);
      WiFi.persistent(false);
      WiFi.setTxPower(WIFI_POWER_15dBm);
      WiFi.begin(newSSID.c_str(), newPASS.c_str());
      Serial.println("Connecting to: " + newSSID);
    }
    request->send_P(200, "text/html", saved_html);
    ResetTextLEDs(strip.Color(0, 255, 0));
  });

  // Gibt die neue IP zurück sobald verbunden (wird von saved_html gepollt)
  // Plant gleichzeitig einen Neustart nach 18 Sekunden
  server.on("/api/newip", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (WiFi.status() == WL_CONNECTED) {
      if (pendingRestartAt == 0) pendingRestartAt = millis() + 4000;
      request->send(200, "text/plain", WiFi.localIP().toString());
    } else {
      request->send(200, "text/plain", "");
    }
  });

  server.on("/start", HTTP_GET, [htmlConfigContent](AsyncWebServerRequest* request) {
    request->send(200, "text/html", htmlConfigContent);
  });

  server.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("msftconnecttest.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("microsoft.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/fwlink", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/wpad.dat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/generate_204", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/redirect", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/canonical.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/success.txt", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/service/update2/json", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/chat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/startpage", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/favicon.ico", [](AsyncWebServerRequest* request) {
    request->send(404);
  });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_html);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
    Serial.println("Served Basic HTML Page");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.print("URL not found: ");
    Serial.print(request->host());
    Serial.print(" ");
    Serial.print(request->url());
    Serial.print(" sent redirect to " + captiveportalURL + "\n");
  });

  server.begin();
  Serial.println("WordClock Captive Portal web server started");
  ShowOfflineIPaddress();  // AP-IP auf den LEDs anzeigen (nutzt WiFi.softAPIP() = apIP)
}


// ###########################################################################################################################################
// # Wifi setup and reconnect function that runs once at startup and during the loop function of the ESP:
// ###########################################################################################################################################
void WIFI_SETUP() {
  Serial.println(" ");
  wifiTimerStart = millis();
  esp_log_level_set("wifi", ESP_LOG_WARN);  // Disable WiFi debug warnings
  if (testTime == 0) {                      // If time text test mode is NOT used:
    String WIFIssid = preferences.getString("WIFIssid");
    bool WiFiConfigEmpty = false;
    if (WIFIssid == "") {
      WiFiConfigEmpty = true;
    }
    String WIFIpass = preferences.getString("WIFIpass");
    if (WIFIpass == "") {
      WiFiConfigEmpty = true;
    }
    if (WiFiConfigEmpty == true) {
      Serial.println("Show SET WIFI...");
      uint32_t c = strip.Color(0, 255, 255);
      int TextWait = 500;
      showtext('S', TextWait, c);
      showtext('E', TextWait, c);
      showtext('T', TextWait, c);
      showtext(' ', TextWait, c);
      showtext('W', TextWait, c);
      showtext('I', TextWait, c);
      showtext('F', TextWait, c);
      showtext('I', TextWait, c);
      showtext(' ', TextWait, c);
      CaptivePortalSetup();
      SetWLAN(strip.Color(0, 255, 255));
    } else {
      Serial.println("Try to connect to found WiFi configuration: ");
      WiFi.persistent(false);
      WiFi.setHostname(DEFAULT_AP_NAME);  // Vor WiFi.mode() setzen!
      WiFi.mode(WIFI_STA);
      WiFi.setSleep(false);
      int tryCount = 0;
      int wifiRebootCount = preferences.getUInt("wifiRebootCnt", 0);
      Serial.println("WiFi reboot attempt: " + String(wifiRebootCount + 1) + "/10");
      WiFi.setTxPower(WIFI_POWER_15dBm);
      delay(500);
      WiFi.begin((const char*)WIFIssid.c_str(), (const char*)WIFIpass.c_str());
      Serial.println("Connecting to WiFi: " + String(WIFIssid));
      while (WiFi.status() != WL_CONNECTED) {
        SetWLAN(strip.Color(0, 0, 255));
        tryCount = tryCount + 1;
        Serial.print("Connection try #: ");
        Serial.print(tryCount);
        Serial.print(" Status: ");
        Serial.println(WiFi.status());
        if (tryCount >= 8) {
          SetWLAN(strip.Color(255, 0, 0));
        }
        if (tryCount == 10) {
          wifiRebootCount++;
          if (wifiRebootCount >= 10) {
            preferences.putUInt("wifiRebootCnt", 0);
            preferences.putString("WIFIssid", "");  // delete credentials → triggers CaptivePortalSetup on next boot
            preferences.putString("WIFIpass", "");
            Serial.println("WiFi failed after 10 restarts — resetting WiFi credentials. Restarting...");
            delay(500);
            ESP.restart();
          }
          preferences.putUInt("wifiRebootCnt", wifiRebootCount);
          Serial.println("WiFi not reachable — restarting (attempt " + String(wifiRebootCount) + "/10)...");
          delay(500);
          ESP.restart();
        }
        delay(1000);
        SetWLAN(strip.Color(0, 0, 0));
        delay(500);
      }
      Serial.println(" ");
      preferences.putUInt("wifiRebootCnt", 0);  // reset counter on success
      WiFIsetup = true;
      Serial.print("Successfully connected now to WiFi SSID: ");
      Serial.println(WiFi.SSID());
      Serial.println("IP: " + WiFi.localIP().toString());
      Serial.println("DNS: " + WiFi.dnsIP().toString());
      Serial.println("GW: " + WiFi.gatewayIP().toString());
      Serial.println("ESP32 hostname: " + String(WiFi.getHostname()));
      SetWLAN(strip.Color(0, 255, 0));
      delay(1000);
      configNTPTime();      // NTP time setup
      updatenow = true;     // Night Mode beim Start sofort prüfen
      setupWebInterface();  // Generate the configuration page
      handleLEDupdate();    // LED update via web
      setupOTAupate();      // ESP32 OTA update
      Serial.println("######################################################################");
      Serial.println("# Web interface online at: http://" + IpAddress2String(WiFi.localIP()));
      Serial.println("# Web interface online at: http://" + String(WiFi.getHostname()));
      Serial.println("# HTTP controls online at: http://" + IpAddress2String(WiFi.localIP()) + ":2023");
      Serial.println("# HTTP controls online at: http://" + String(WiFi.getHostname()) + ":2023");
      Serial.println("######################################################################");
      Serial.println("# WordClock startup finished...");
      Serial.println("######################################################################");
      if (useStartupText == 1) callStartText();  // Show "WordClock" startup text
      if (useshowip == 1) ShowIPaddress();       // Display the current IP-address
      Serial.println(" ");
      Serial.println(" ");
      Serial.println(" ");
      lastHourSet = iHour;      // prevent animation trigger on first display update
      lastMinutesSet = iMinute;
      updatenow = true;         // Update the display 1x after startup
      update_display();         // Update LED display
    }
  }
}

// ###########################################################################################################################################
// # ESP32 OTA update:
// ###########################################################################################################################################
const char otaserverIndex[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>WordClock</title></head>
      <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <body>
    <form method='POST' action='/update' enctype='multipart/form-data'>
      <center><b><h1>WordClock software update</h1></b>
      <h2>Please select the in the Arduino IDE > "Sketch" ><br/>"Export Compiled Binary (Alt+Ctrl+S)"<br/>to generate the required "Code.ino.bin" file.<br/><br/>
      Select the "Code.ino.bin" file with the "Search" button.<br/><br/>
      Use the "Update" button to start the update.<br/><br/>WordClock will restart automatically.</h2><br/>
      <input type='file' name='update'>       <input type='submit' value='Update'>
     </center></form></body>
  </html>
 )=====";


const char otaNOK[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>WordClock</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
    <body>
      <center><b><h1>WordClock software update</h1></b>
      <h2>ERROR: Software update FAILED !!!<br/><br/>WordClock will restart automatically.</h2><br/>
      </center></body>
  </html>
 )=====";


const char otaOK[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>WordClock</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
    <body>
      <center><b><h1>WordClock software update</h1></b>
      <h2>Software update done =)<br/><br/>WordClock will restart automatically.</h2><br/>
      </center></body>
  </html>
 )=====";


void setupOTAupate() {
  otaserver.on("/", HTTP_GET, []() {
    otaserver.sendHeader("Connection", "close");
    otaserver.send(200, "text/html", otaserverIndex);
  });

  otaserver.on(
    "/update", HTTP_POST, []() {
      otaserver.sendHeader("Connection", "close");
      if (Update.hasError()) {
        otaserver.send(200, "text/html", otaNOK);
        ResetTextLEDs(strip.Color(255, 0, 0));
      } else {
        otaserver.send(200, "text/html", otaOK);
        ResetTextLEDs(strip.Color(0, 255, 0));
      }
      delay(3000);
      ESP.restart();
    },
    []() {
      HTTPUpload& upload = otaserver.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin()) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Update success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      } else {
        Serial.printf("Update failed unexpectedly (likely broken connection): status=%d\n", upload.status);
      }
    });
  otaserver.begin();
}


// ###########################################################################################################################################
// # OFFLINE MODE Captive Portal by AWSW
// ###########################################################################################################################################
void OfflinePotalSetup() {
  wifiTimerStart = millis();
  if (debugtexts == 1) Serial.println("\nCreating WordClock Offline Mode access point...");
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_15dBm);
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (UseOfflineModeWithPassword == 1) {  // Access point with password or not
    WiFi.softAP(Offline_SSID, Offline_PW);
  } else {
    WiFi.softAP(Offline_SSID);
  }
  Serial.println("##############################################################################################################################################################################################################");
  Serial.print("# Offline Mode WiFi access point initialized. Please connect to the WiFi access point and set the current time now. Access point name: '");
  Serial.print(Offline_SSID);
  if (UseOfflineModeWithPassword == 1) {
    Serial.print("' using the password: '");
    Serial.print(Offline_PW);
  }
  Serial.println("'");
  Serial.print("# In case your browser does not open the WordClock configuration page automatically after connecting to the access point, please navigate to this URL manually: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("##############################################################################################################################################################################################################");

  setupWebInterface();  // Generate the configuration page
  setupOTAupate();      // OTA auch im Offline-Modus verfügbar

  server.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /connecttest.txt");
  });
  server.on("msftconnecttest.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served msftconnecttest.com");
  });
  server.on("/fwlink", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /fwlink");
  });
  server.on("/wpad.dat", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served wpad.dat");
  });
  server.on("/generate_204", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /gernerate_204");
  });
  server.on("/redirect", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /redirect");
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /hotspot-detect.html");
  });
  server.on("/canonical.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /cannonical.html");
  });
  server.on("/success.txt", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /success.txt");
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /ncsi.txt");
  });
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /chrome-variations/seed");
  });
  server.on("/service/update2/json", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /service/update2/json");
  });
  server.on("/chat", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served /chat");
  });
  server.on("/startpage", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /startpage");
  });
  server.on("/favicon.ico", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served /favicon.ico");
  });


  server.on("/", HTTP_ANY, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", WEBUI_HTML);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
    Serial.println("Served WEBUI HTML Page");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.print("Web page not found: ");
    Serial.print(request->host());
    Serial.print(" ");
    Serial.print(request->url());
    Serial.print(" sent redirect to " + captiveportalURL + "\n");
  });

  server.begin();
  if (debugtexts == 1) Serial.println("WordClock OFFLINE MODE captive portal web server started");
  ShowOfflineIPaddress();  // Display the current Offline Mode IP-address every time on startup
}


// ###########################################################################################################################################
// # Show the Offline Mode IP-address on the display:
// ###########################################################################################################################################
void ShowOfflineIPaddress() {
  if (useshowip == 1) {
    // Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.softAPIP()));
    int ipdelay = 2000;

    // Testing the digits:
    // for (int i = 0; i < 10; i++) {
    //   back_color();
    //   // numbers(i, 3);
    //   // numbers(i, 2);
    //   numbers(i, 1);
    //   showStrip();
    //   delay(ipdelay);
    // }

    // Octet 1:
    ClearDisplay();
    numbers(getDigit(int(WiFi.softAPIP()[0]), 2), 3);
    numbers(getDigit(int(WiFi.softAPIP()[0]), 1), 2);
    numbers(getDigit(int(WiFi.softAPIP()[0]), 0), 1);
    setLED(191, 191, 1);
    setLED(0, 3, 1);
    setLED(240, 243, 1);
    showStrip();
    delay(ipdelay);

    // // Octet 2:
    ClearDisplay();
    numbers(getDigit(int(WiFi.softAPIP()[1]), 2), 3);
    numbers(getDigit(int(WiFi.softAPIP()[1]), 1), 2);
    numbers(getDigit(int(WiFi.softAPIP()[1]), 0), 1);
    setLED(191, 191, 1);
    setLED(0, 7, 1);
    setLED(240, 247, 1);
    showStrip();
    delay(ipdelay);

    // // Octet 3:
    ClearDisplay();
    numbers(getDigit(int(WiFi.softAPIP()[2]), 2), 3);
    numbers(getDigit(int(WiFi.softAPIP()[2]), 1), 2);
    numbers(getDigit(int(WiFi.softAPIP()[2]), 0), 1);
    setLED(191, 191, 1);
    setLED(0, 11, 1);
    setLED(240, 251, 1);
    showStrip();
    delay(ipdelay);

    // // Octet 4:
    ClearDisplay();
    numbers(getDigit(int(WiFi.softAPIP()[3]), 2), 3);
    numbers(getDigit(int(WiFi.softAPIP()[3]), 1), 2);
    numbers(getDigit(int(WiFi.softAPIP()[3]), 0), 1);
    setLED(0, 15, 1);
    setLED(240, 255, 1);
    showStrip();
    delay(ipdelay);
  }
}



// ###########################################################################################################################################
// # Split String into an Array (DayNightModeFunction)
// ###########################################################################################################################################
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// ###########################################################################################################################################
// # Text output function: (modified and original by Dreso)
// ###########################################################################################################################################
// # Matrix treated with coordinates from 1,1 (top,left) to 16,16 (bottom, right)
// # each element with two Leds aligned above each other  (16x16 Matrix treated as 8x16 Matrix)
// #  __________________       ______________
// # | 1,1 | 1,2 | 1,3 | .... | 1,15 | 1,16 |
// # |-----|-----|-----|       -------------|
// # | 2,1 | 2,2 |...................| 2,16 |
// # |-----|-----|-----|       -------------|
// # | 3,1 |-----|
// # |-----|-----|-----|       -------------|
// # |...................
// # |-----|-----|-----|       -------------|
// # | 8,1 | 8,2 | 8,3 | ...  | 8,15 | 8,16 |
// # |_____|_____|______     _|______|______|
#define MAXROWS 8
#define MAXCOLUMS 16


// ##############################################################################################
// #Duocolor Pixel
// ##############################################################################################
void DrawPixelX(int row, int colum, uint32_t c1, uint32_t c2) {
  if (row > 0) row--;                                                      // internal index begins with 0 and external with 1
  if (colum > 0) colum--;                                                  // internal index begins with 0
  strip.setPixelColor(row * 2 * MAXCOLUMS + (MAXCOLUMS - 1 - colum), c1);  // upper led
  strip.setPixelColor(row * 2 * MAXCOLUMS + (MAXCOLUMS + colum), c2);      // lower led // (2023 WordClock)
}


// ##############################################################################################
// # Monocolor Line Drawing
// ##############################################################################################
void DrawLine(int startrow, int startcolum, int endrow, int endcolum, uint32_t c) {
  DrawLineX(startrow, startcolum, endrow, endcolum, c, c);  //same color for both Leds
}


// ##############################################################################################
// # Monocolor Pixel
// ##############################################################################################
void DrawPixel(int row, int colum, uint32_t c) {
  DrawPixelX(row, colum, c, c);  //same color for both Leds
}


// ##############################################################################################
// # Draw Line with possible different colours on the two leds per element
// #
// # Diagonal Lines are also possible
// # Boundaries are not checked in this routine
// # Endpoints can also be smaller in number than the start
// ##############################################################################################
void DrawLineX(int startrow, int startcolum, int endrow, int endcolum, uint32_t c1, uint32_t c2) {
  int dx, dy;
  if (endcolum > startcolum) {
    dx = 1;
  } else {
    if (endcolum < startcolum) {
      dx = -1;
    } else {
      dx = 0;
    }
  }
  if (endrow > startrow) {
    dy = 1;
  } else {
    if (endrow < startrow) {
      dy = -1;
    } else {
      dy = 0;
    }
  }

  int actrow = startrow, actcolum = startcolum;
  bool bDone = false;
  while (!bDone) {
    DrawPixelX(actrow, actcolum, c1, c2);
    if (dx == 0 && dy == 0) {
      bDone = true;
    }
    actrow += dy;
    actcolum += dx;
    if (actrow == endrow) {
      dy = 0;
    }
    if (actcolum == endcolum) {
      dx = 0;
    }
  }
}


// ###########################################################################################################################################
// # Text output function:
// ###########################################################################################################################################
void showtext(char letter, int wait, uint32_t c) {
  ClearDisplay();
  uint32_t c2;
  letter = toupper(letter);  // only uppercase possible at the moment

  if (letter == 'A') {
    DrawLine(3, 6, 6, 6, c);
    DrawLine(3, 11, 6, 11, c);
    DrawLine(2, 7, 2, 10, c);
    DrawLine(4, 6, 4, 11, c);
  }
  if (letter == 'B') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 7, 2, 10, c);
    DrawPixel(3, 11, c);
    DrawLine(4, 7, 4, 10, c);
    DrawPixel(5, 11, c);
    DrawLine(6, 7, 6, 10, c);
  }
  if (letter == 'C') {
    DrawLine(2, 7, 2, 11, c);
    DrawLine(3, 6, 5, 6, c);
    DrawLine(6, 7, 6, 11, c);
  }
  if (letter == 'D') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(3, 11, 5, 11, c);
    DrawLine(2, 7, 2, 10, c);
    DrawLine(6, 7, 6, 10, c);
  }
  if (letter == 'E') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 6, 2, 11, c);
    DrawLine(4, 6, 4, 10, c);
    DrawLine(6, 6, 6, 11, c);
  }
  if (letter == 'F') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 7, 2, 11, c);
    DrawLine(4, 7, 4, 10, c);
  }
  if (letter == 'G') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(4, 11, 6, 11, c);
    DrawLine(2, 6, 2, 11, c);
    DrawLine(4, 9, 4, 11, c);
    DrawLine(6, 6, 6, 11, c);
  }
  if (letter == 'H') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 11, 6, 11, c);
    DrawLine(4, 6, 4, 11, c);
  }
  if (letter == 'I') {
    DrawLine(2, 7, 2, 9, c);
    DrawLine(3, 8, 5, 8, c);
    DrawLine(6, 7, 6, 9, c);
  }
  if (letter == 'J') {
    DrawLine(2, 6, 2, 10, c);
    DrawLine(3, 10, 5, 10, c);
    DrawLine(6, 7, 6, 9, c);
    DrawPixel(5, 6, c);
  }
  if (letter == 'K') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(4, 7, 4, 9, c);
    DrawLine(4, 9, 2, 11, c);
    DrawLine(4, 9, 6, 11, c);
  }
  if (letter == 'L') {
    //"L"
    DrawLine(2, 6, 6, 6, c);
    DrawLine(6, 6, 6, 11, c);
  }
  if (letter == 'M') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 11, 6, 11, c);
    DrawPixel(3, 7, c);
    DrawLine(4, 8, 4, 9, c);
    DrawLine(3, 10, 3, 10, c);
  }
  if (letter == 'N') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 11, 6, 11, c);
    DrawLine(3, 7, 6, 10, c);
  }
  if (letter == 'O') {
    DrawLine(3, 6, 5, 6, c);
    DrawLine(3, 11, 5, 11, c);
    DrawLine(2, 7, 2, 10, c);
    DrawLine(6, 7, 6, 10, c);
  }
  if (letter == 'P') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(3, 11, 3, 11, c);
    DrawLine(2, 6, 2, 10, c);
    DrawLine(4, 6, 4, 10, c);
  }
  if (letter == 'Q') {
    DrawLine(3, 6, 5, 6, c);
    DrawLine(3, 11, 5, 11, c);
    DrawLine(2, 7, 2, 10, c);
    DrawLine(6, 7, 6, 10, c);
    DrawLine(5, 10, 6, 11, c);
  }
  if (letter == 'R') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(3, 11, 4, 11, c);
    DrawLine(2, 6, 2, 10, c);
    DrawLine(4, 6, 4, 11, c);
    DrawLine(5, 10, 6, 11, c);
  }
  if (letter == 'S') {
    DrawLine(3, 6, 3, 6, c);
    DrawLine(5, 11, 5, 11, c);
    DrawLine(2, 7, 2, 11, c);
    DrawLine(4, 7, 4, 10, c);
    DrawLine(6, 6, 6, 10, c);
  }
  if (letter == 'T') {
    DrawLine(2, 6, 2, 10, c);
    DrawLine(3, 8, 6, 8, c);
  }
  if (letter == 'U') {
    DrawLine(2, 6, 5, 6, c);
    DrawLine(2, 11, 5, 11, c);
    DrawLine(6, 7, 6, 10, c);
  }
  if (letter == 'V') {
    DrawLine(2, 6, 3, 6, c);
    DrawLine(2, 11, 3, 11, c);
    DrawLine(4, 6, 6, 8, c);
    DrawLine(6, 9, 4, 11, c);
  }
  if (letter == 'W') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 11, 6, 11, c);
    DrawLine(5, 7, 4, 8, c);
    DrawLine(4, 9, 5, 10, c);
  }
  if (letter == 'X') {
    DrawLine(2, 6, 4, 8, c);
    DrawLine(4, 9, 2, 11, c);
    DrawLine(6, 6, 4, 8, c);
    DrawLine(4, 9, 6, 11, c);
  }
  if (letter == 'Y') {
    DrawLine(2, 6, 4, 8, c);
    DrawLine(4, 8, 2, 10, c);
    DrawLine(5, 8, 6, 8, c);
  }
  if (letter == 'Z') {
    DrawLine(2, 6, 2, 10, c);
    DrawLine(3, 9, 5, 7, c);
    DrawLine(6, 6, 6, 10, c);
  }

  if (letter == ' ') {
    // already cleareed at top
  }
  showStrip();
  delay(wait);
}


// ###########################################################################################################################################
// # EOF - You have successfully reached the end of the code - well done ;-)
// ###########################################################################################################################################
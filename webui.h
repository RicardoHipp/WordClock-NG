// ###########################################################################################################################################
// # WordClock custom web UI - replaces ESPUI
// ###########################################################################################################################################
const char WEBUI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>WordClock &middot; Config</title>
<style>
:root{--bg:#f7f7f4;--s:#fff;--l:#e9e9e4;--l2:#f0efeb;--ink:#14140f;--ink2:#555550;--ink3:#8a8a82;--a:#65d435;--ai:#0e2a05;--danger:#d24a3c;--r:10px;--gap:14px}
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,'Segoe UI',sans-serif;background:var(--bg);color:var(--ink);min-height:100vh;-webkit-font-smoothing:antialiased}
.page{max-width:1100px;margin:0 auto;padding:40px 20px 80px}
.hdr{display:flex;align-items:center;justify-content:space-between;padding-bottom:22px;border-bottom:1px solid var(--l);margin-bottom:32px;flex-wrap:wrap;gap:12px}
.brand{display:flex;align-items:baseline;gap:12px}
.bmark{font-size:21px;font-weight:700;letter-spacing:-.03em;display:flex;align-items:center;gap:8px}
.bdot{width:7px;height:7px;border-radius:50%;background:var(--a);flex-shrink:0}
.bsub{font-size:11px;color:var(--ink3);letter-spacing:.08em;text-transform:uppercase}
.hdr-r{display:flex;align-items:center;gap:16px}
.pill{display:inline-flex;align-items:center;gap:6px;padding:5px 10px 5px 8px;border-radius:999px;font-size:12px;font-weight:500;border:1px solid rgba(101,212,53,.3);background:rgba(101,212,53,.1);color:#2a6010}
.pill::before{content:'';width:6px;height:6px;border-radius:50%;background:var(--a)}
.mono{font-family:monospace;font-size:12px;color:var(--ink3)}
.sec{margin-bottom:36px}
.sec-hd{display:flex;align-items:center;gap:10px;margin-bottom:14px}
.sec-n{font-size:11px;color:var(--ink3);font-family:monospace}
.sec-t{font-size:11.5px;font-weight:600;text-transform:uppercase;letter-spacing:.12em}
.sec-l{flex:1;height:1px;background:var(--l)}
.grid{display:grid;grid-template-columns:repeat(3,1fr);gap:var(--gap)}
.g2{grid-template-columns:repeat(2,1fr)}
.g1{grid-template-columns:1fr}
@media(max-width:860px){.grid{grid-template-columns:repeat(2,1fr)}}
@media(max-width:540px){.grid{grid-template-columns:1fr}}
.full{grid-column:1/-1}
.card{background:var(--s);border:1px solid var(--l);border-radius:var(--r);padding:14px 16px;display:flex;flex-direction:column;gap:10px}
.clbl{font-size:11px;font-weight:500;color:var(--ink3);letter-spacing:.04em;text-transform:uppercase}
.cval{font-size:13px;color:var(--ink2);line-height:1.5}
.cmono{font-family:monospace;font-size:12.5px;color:var(--ink);background:var(--l2);border:1px solid var(--l);border-radius:5px;padding:3px 8px;display:inline-block}
.tog-row{display:flex;align-items:center;justify-content:space-between;gap:8px}
.tog{position:relative;width:38px;height:22px;flex-shrink:0;cursor:pointer}
.tog input{opacity:0;width:0;height:0;position:absolute}
.tog-sl{position:absolute;inset:0;background:#dcdcd6;border-radius:999px;transition:background .18s;border:1px solid var(--l)}
.tog-sl::after{content:'';position:absolute;width:16px;height:16px;top:2px;left:2px;background:#fff;border-radius:50%;transition:transform .18s cubic-bezier(.5,1.6,.5,1);box-shadow:0 1px 3px rgba(0,0,0,.2)}
.tog input:checked+.tog-sl,.tog.on .tog-sl{background:var(--a);border-color:var(--a)}
.tog input:checked+.tog-sl::after,.tog.on .tog-sl::after{transform:translateX(16px)}
input[type=range]{width:100%;-webkit-appearance:none;height:4px;border-radius:2px;background:var(--l);outline:none;cursor:pointer;pointer-events:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;border-radius:50%;background:var(--a);cursor:pointer;box-shadow:0 1px 3px rgba(0,0,0,.2);pointer-events:auto}
input[type=range]::-moz-range-thumb{width:22px;height:22px;border-radius:50%;background:var(--a);cursor:pointer;border:none;pointer-events:auto}
.rv{font-size:12px;color:var(--ink2);font-family:monospace;min-width:28px;text-align:right}
input[type=color]{width:100%;height:34px;border:1px solid var(--l);border-radius:7px;padding:2px;background:var(--bg);cursor:pointer;-webkit-appearance:none}
input[type=color]::-webkit-color-swatch-wrapper{padding:2px}
input[type=color]::-webkit-color-swatch{border:none;border-radius:4px}
select,input[type=time]{width:100%;padding:8px 10px;border:1px solid var(--l);border-radius:7px;font-size:13px;font-family:inherit;color:var(--ink);background:var(--bg);outline:none;-webkit-appearance:none}
select:focus,input[type=time]:focus{border-color:var(--a);box-shadow:0 0 0 3px rgba(101,212,53,.12)}
.btn{padding:8px 16px;border:none;border-radius:7px;font-size:13px;font-weight:600;cursor:pointer;font-family:inherit;transition:opacity .15s}
.btn:hover{opacity:.85}
.btn:disabled{opacity:.4;cursor:not-allowed}
.btn-a{background:var(--a);color:var(--ai)}
.btn-g{background:var(--l2);color:var(--ink2);border:1px solid var(--l)}
.btn-d{background:var(--danger);color:#fff}
.warn{padding:9px 13px;border-radius:7px;font-size:12.5px;background:#fdf2f1;border:1px solid rgba(210,74,60,.25);color:var(--danger)}
.info{padding:9px 13px;border-radius:7px;font-size:12.5px;background:rgba(101,212,53,.08);border:1px solid rgba(101,212,53,.3);color:#2a6010}
.toast{position:fixed;bottom:20px;right:20px;padding:9px 14px;border-radius:8px;font-size:13px;opacity:0;transition:opacity .2s;pointer-events:none;z-index:99;color:#fff}
.toast.show{opacity:1}
</style>
</head>
<body>
<div class="page">

<header class="hdr">
  <div class="brand">
    <div class="bmark"><span class="bdot"></span>WordClock</div>
    <span class="bsub">Configuration</span>
  </div>
  <div class="hdr-r">
    <div class="mono" id="chip_temp" style="color:var(--ink3)">-&deg;C</div>
    <div class="pill">Connected</div>
    <div class="mono" style="color:var(--ink3)">&#127760; <span id="lang_name">-</span></div>
    <div class="mono">v<span id="fw">-</span></div>
  </div>
</header>

<!-- 01 LED -->
<div class="sec">
  <div class="sec-hd"><span class="sec-n">01</span><span class="sec-t">LED Settings</span><span class="sec-l"></span></div>
  <div id="web-ctrl-warn" style="display:none;margin-bottom:12px">
    <div class="warn">Brightness is currently controlled via web URL. <a id="web-ctrl-link" href="#" style="color:var(--danger)">Deactivate</a></div>
  </div>
  <div class="grid">
    <div class="card">
      <div class="clbl">Time Color</div>
      <input type="color" id="color_time" oninput="deb('color_time',this.value)">
    </div>
    <div class="card">
      <div class="clbl">Background Color</div>
      <input type="color" id="color_back" oninput="deb('color_back',this.value)">
    </div>
    <div class="card">
      <div class="clbl">Random Color / minute</div>
      <div class="tog-row">
        <span class="cval" id="l-rc">-</span>
        <label class="tog"><input type="checkbox" id="random_color" onchange="tog('random_color','l-rc',this.checked)"><span class="tog-sl"></span></label>
      </div>
    </div>
    <div class="card">
      <div class="clbl">Show Single Minutes</div>
      <div class="tog-row">
        <span class="cval" id="l-sm">-</span>
        <label class="tog"><input type="checkbox" id="single_min" onchange="tog('single_min','l-sm',this.checked)"><span class="tog-sl"></span></label>
      </div>
    </div>
  </div>
</div>

<!-- 02 DAY/NIGHT -->
<div class="sec">
  <div class="sec-hd"><span class="sec-n">02</span><span class="sec-t">Day / Night Brightness</span><span class="sec-l"></span></div>
  <div id="night-warn" style="display:none;margin-bottom:12px">
    <div class="warn">Wrong setting: 'Day starts at' must be before 'Day ends after'.</div>
  </div>
  <div class="grid">
    <div class="card">
      <div class="clbl">Night Mode</div>
      <div class="tog-row">
        <span class="cval" id="l-nm">-</span>
        <label class="tog"><input type="checkbox" id="night_mode" onchange="tog('night_mode','l-nm',this.checked)"><span class="tog-sl"></span></label>
      </div>
    </div>
    <div class="card">
      <div class="clbl">Night Mode Status</div>
      <div class="cval" id="night_status">-</div>
    </div>
    <div class="card">
      <div class="clbl">Brightness Day &mdash; <span class="rv" id="v-bd">-</span></div>
      <input type="range" id="brightness_day" min="0" max="255" oninput="document.getElementById('v-bd').textContent=this.value;deb('brightness_day',this.value)">
    </div>
    <div class="card">
      <div class="clbl">Brightness Night &mdash; <span class="rv" id="v-bn">-</span></div>
      <input type="range" id="brightness_night" min="0" max="255" oninput="document.getElementById('v-bn').textContent=this.value;deb('brightness_night',this.value)">
    </div>
    <div class="card">
      <div class="clbl">Day starts at</div>
      <input type="time" id="day_start" onchange="s('day_start',this.value)">
    </div>
    <div class="card">
      <div class="clbl">Day ends after</div>
      <input type="time" id="day_stop" onchange="s('day_stop',this.value)">
    </div>
  </div>
</div>

<!-- 03 OPERATION MODE -->
<div class="sec">
  <div class="sec-hd"><span class="sec-n">03</span><span class="sec-t">Operation Mode</span><span class="sec-l"></span></div>
  <div class="grid g2">
    <div class="card">
      <div class="clbl">Use Online Mode</div>
      <div class="tog-row">
        <span class="cval" id="l-om">-</span>
        <label class="tog"><input type="checkbox" id="online_mode" onchange="sr('online_mode',this.checked?1:0)"><span class="tog-sl"></span></label>
      </div>
    </div>
    <div class="card">
      <div class="clbl">Status</div>
      <div class="cval" id="mode_status">-</div>
    </div>
  </div>
  <!-- Offline-only -->
  <div id="sec-offline" style="display:none;margin-top:var(--gap)">
    <div class="grid">
      <div class="card">
        <div class="clbl">Get Time from Device</div>
        <button class="btn btn-a" onclick="getDevTime()">Sync Time</button>
        <div class="cval" id="dev_time_status" style="font-size:12px;color:var(--ink3)"></div>
      </div>
      <div class="card full">
        <div class="clbl">WLAN-Timeout (Offline-Hotspot) &mdash; <span class="rv" id="v-wt">-</span> Min.</div>
        <input type="range" id="wifi_timeout" min="0" max="120" step="1" oninput="document.getElementById('v-wt').textContent=this.value==='0'?'Aus':(this.value+' Min.');deb('wifi_timeout',this.value)">
        <div class="cval" style="font-size:11px;color:var(--ink3)">Schaltet den WLAN-Hotspot im Offline-Modus nach der gewählten Zeit nach dem Systemstart ab, um Strahlung zu vermeiden. 0 = Deaktiviert.</div>
      </div>
    </div>
  </div>
</div>

<!-- 04 STARTUP (online only) -->
<div class="sec" id="sec-startup">
  <div class="sec-hd"><span class="sec-n">04</span><span class="sec-t">Startup</span><span class="sec-l"></span></div>
  <div class="grid">
    <div class="card">
      <div class="clbl">Show 'WordClock' text</div>
      <div class="tog-row">
        <span class="cval" id="l-st">-</span>
        <label class="tog"><input type="checkbox" id="startup_text" onchange="tog('startup_text','l-st',this.checked)"><span class="tog-sl"></span></label>
      </div>
    </div>
    <div class="card">
      <div class="clbl">Show IP on startup</div>
      <div class="tog-row">
        <span class="cval" id="l-si">-</span>
        <label class="tog"><input type="checkbox" id="show_ip" onchange="tog('show_ip','l-si',this.checked)"><span class="tog-sl"></span></label>
      </div>
    </div>
    <div class="card">
      <div class="clbl">Startup Time</div>
      <div class="cmono" id="startup_time">-</div>
    </div>
  </div>
</div>

<!-- 05 WIFI (online only) -->
<div class="sec" id="sec-wifi">
  <div class="sec-hd"><span class="sec-n">05</span><span class="sec-t">WiFi</span><span class="sec-l"></span></div>
  <div class="grid">
    <div class="card"><div class="clbl">SSID</div><div class="cmono" id="ssid">-</div></div>
    <div class="card"><div class="clbl">Signal</div><div class="cmono" id="rssi">-</div></div>
    <div class="card"><div class="clbl">IP Address</div><div class="cmono" id="ip">-</div></div>
    <div class="card"><div class="clbl">MAC Address</div><div class="cmono" id="mac">-</div></div>
    <div class="card"><div class="clbl">Hostname</div><div class="cmono" id="hostname">-</div></div>
    <div class="card"><div class="clbl">Gateway</div><div class="cmono" id="gateway">-</div></div>
  </div>
</div>

<!-- 06 RTC & TIME ZONE -->
<div class="sec">
  <div class="sec-hd"><span class="sec-n">06</span><span class="sec-t">RTC &amp; Time Zone</span><span class="sec-l"></span></div>
  <div class="grid">
    <div class="card">
      <div class="clbl">Browser Zeit (Lokal)</div>
      <div class="cmono" style="font-size:13px" id="browser_time">--:--:--</div>
    </div>
    <div class="card">
      <div class="clbl">ESP32 Clock (Lokal)</div>
      <div class="cmono" style="font-size:13px" id="esp_time">--:--:--</div>
    </div>
    <div class="card">
      <div class="clbl">DS3231 RTC (Lokal)</div>
      <div class="cmono" style="font-size:13px" id="rtc_local">--:--:--</div>
    </div>
    <div class="card">
      <div class="clbl">UTC (Nullzeit)</div>
      <div class="cmono" style="font-size:13px" id="utc_time">--:--:--</div>
    </div>
    <div class="card">
      <div class="clbl">UTC-Offset (DST)</div>
      <div class="cmono" style="font-size:13px" id="utc_offset">--</div>
    </div>
    <div class="card">
      <div class="clbl">DS3231 Status</div>
      <div class="cval" id="ds3231_status">-</div>
    </div>
    <div class="card">
      <div class="clbl">DS3231 Chip-Temperatur</div>
      <div class="cmono" style="font-size:13px" id="ds3231_temp">--</div>
    </div>
    <div class="card">
      <div class="clbl">RTC-Abgleich (Drift-Test)</div>
      <button class="btn btn-g" id="btn-rtcsync" onclick="toggleRtcSync()">Abgleich pausieren</button>
      <div class="cval" style="font-size:11px;color:var(--ink3)">Pausiert den 5-Min-Abgleich der ESP-Uhr mit der DS3231, um die Drift beider Uhren getrennt zu beobachten</div>
    </div>
    <div class="card">
      <div class="clbl">ESP-Uhr verstellen (Sync-Test)</div>
      <div style="display:flex;align-items:center;gap:8px;flex-wrap:wrap">
        <input type="number" id="esp_shift_sec" value="60" step="1" style="width:80px">
        <button class="btn btn-a" onclick="espShift()">Verstellen</button>
      </div>
      <div class="cval" style="font-size:11px;color:var(--ink3)">Verschiebt nur die ESP-Uhr um n Sekunden (negativ erlaubt) &mdash; die DS3231 bleibt unberührt. Der nächste 5-Min-Abgleich sollte die Abweichung wieder korrigieren</div>
    </div>
    <div class="card full">
      <div class="clbl">N&auml;chste Sommer/Winterzeit</div>
      <div class="cval" id="dst_next">-</div>
    </div>
    <div class="card full">
      <div class="clbl">Time Zone</div>
      <select id="timezone" onchange="sr('timezone',this.value)">
        <option value="CET-1CEST,M3.5.0,M10.5.0/3">Central Europe (DE/AT/CH/FR/IT/NL/PL)</option>
        <option value="MET-2METDST,M3.5.0/01,M10.5.0/02">Most of Europe</option>
        <option value="GMT0BST,M3.5.0/01,M10.5.0/02">UK (London)</option>
        <option value="EET-2EEST,M3.5.5/0,M10.5.5/0">Eastern Europe / Asia</option>
        <option value="EST5EDT,M3.2.0,M11.1.0">USA EST</option>
        <option value="CST6CDT,M3.2.0,M11.1.0">USA CST</option>
        <option value="MST7MDT,M4.1.0,M10.5.0">USA MST</option>
        <option value="ACST-9:30ACDT,M10.1.0,M4.1.0/3">Australia</option>
        <option value="NZST-12NZDT,M9.5.0,M4.1.0/3">New Zealand</option>
        <option value="ICT-7">Vietnam (ICT-7)</option>
      </select>
    </div>
  </div>
</div>

<!-- 07 TIME SETTINGS (online only) -->
<div class="sec" id="sec-time">
  <div class="sec-hd"><span class="sec-n">07</span><span class="sec-t">Time Settings</span><span class="sec-l"></span></div>
  <div class="grid g1">
    <div class="card">
      <div class="clbl">NTP Time Server</div>
      <select id="ntp_server" onchange="sr('ntp_server',this.value)">
        <option value="Your local router">Best choice: Your local router (gateway)</option>
        <option value="pool.ntp.org">pool.ntp.org</option>
        <option value="europe.pool.ntp.org">europe.pool.ntp.org</option>
        <option value="fritz.box">fritz.box</option>
        <option value="speedport.ip">speedport.ip</option>
        <option value="time.fu-berlin.de">DE: time.fu-berlin.de</option>
        <option value="ntp.neel.ch">CH: ntp.neel.ch</option>
        <option value="asynchronos.iiss.at">AT: asynchronos.iiss.at</option>
        <option value="ntp1.oma.be">BE: ntp1.oma.be</option>
        <option value="uk.pool.ntp.org">UK: uk.pool.ntp.org</option>
        <option value="north-america.pool.ntp.org">US: north-america.pool.ntp.org</option>
        <option value="asia.pool.ntp.org">AS: asia.pool.ntp.org</option>
        <option value="ntp.nict.jp">JP: ntp.nict.jp</option>
      </select>
    </div>
  </div>
  <div class="grid g2" style="margin-top:var(--gap)">
    <div class="card">
      <div class="clbl">Daily NTP Resync Time</div>
      <input type="time" id="ntp_sync_time" onchange="setNtpSyncTime(this.value)">
      <div class="cval" style="font-size:11px;color:var(--ink3)">Uhrzeit für den täglichen NTP-Sync (Standard: 03:00)</div>
    </div>
  </div>
</div>


<!-- 08 ANIMATION -->
<div class="sec">
  <div class="sec-hd"><span class="sec-n">08</span><span class="sec-t">Animation</span><span class="sec-l"></span></div>
  <div class="grid g1">
    <div class="card">
      <div class="clbl">Transition Effect (every 5 minutes)</div>
      <select id="animation_mode" onchange="s('animation_mode',this.value)">
        <option value="0">No effect</option>
        <option value="1">Matrix</option>
        <option value="2">Teletype</option>
        <option value="3">Fade</option>
        <option value="4">Squeeze</option>
        <option value="5">Snake</option>
        <option value="6">Flicker</option>
      </select>
    </div>
    <div class="card">
      <div class="clbl">Test Animation</div>
      <button class="btn btn-a" onclick="testAnimation()">Test</button>
    </div>
  </div>
</div>

<!-- 09 WEATHER (online only) -->
<div class="sec" id="sec-weather">
  <div class="sec-hd"><span class="sec-n">09</span><span class="sec-t">Weather</span><span class="sec-l"></span></div>
  <div class="grid">
    <div class="card full">
      <div class="clbl">OpenWeatherMap API Key</div>
      <input type="text" id="weather_key" placeholder="e.g. a1b2c3d4e5f6..." style="width:100%;padding:8px 10px;border:1px solid var(--l);border-radius:7px;font-size:13px;font-family:monospace;color:var(--ink);background:var(--bg);outline:none" oninput="debStr('weather_key',this.value)">
      <div class="cval" style="font-size:11px;color:var(--ink3)">Free key at openweathermap.org &rarr; API keys</div>
    </div>
    <div class="card">
      <div class="clbl">Latitude</div>
      <input type="text" id="weather_lat" placeholder="e.g. 48.137" style="width:100%;padding:8px 10px;border:1px solid var(--l);border-radius:7px;font-size:13px;font-family:monospace;color:var(--ink);background:var(--bg);outline:none" oninput="debStr('weather_lat',this.value)">
    </div>
    <div class="card">
      <div class="clbl">Longitude</div>
      <input type="text" id="weather_lon" placeholder="e.g. 11.576" style="width:100%;padding:8px 10px;border:1px solid var(--l);border-radius:7px;font-size:13px;font-family:monospace;color:var(--ink);background:var(--bg);outline:none" oninput="debStr('weather_lon',this.value)">
    </div>
    <div class="card full" id="weather-info" style="display:none">
      <div class="clbl">Last Fetch</div>
      <div class="cval" style="display:flex;gap:16px;flex-wrap:wrap;align-items:center">
        <span>&#127757; <span id="w-city">-</span></span>
        <span class="cmono" id="w-desc">-</span>
        <span class="cmono"><span id="w-temp">-</span> &deg;C</span>
        <span class="cmono">Icon: <span id="w-icon">-</span></span>
      </div>
    </div>
    <div class="card full">
      <div class="clbl">Test Icon on Display</div>
      <div style="display:flex;align-items:center;gap:10px;flex-wrap:wrap">
        <select id="weather_test_icon" style="flex:1;padding:8px 10px;border:1px solid var(--l);border-radius:7px;font-size:13px;font-family:inherit;color:var(--ink);background:var(--bg);outline:none">
          <option value="01">01 &mdash; Clear sky (day)</option>
          <option value="01n">01n &mdash; Clear sky (night)</option>
          <option value="02">02 &mdash; Few clouds (day)</option>
          <option value="02n">02n &mdash; Few clouds (night)</option>
          <option value="03">03 &mdash; Scattered clouds</option>
          <option value="09">09 &mdash; Shower rain</option>
          <option value="10">10 &mdash; Rain (day)</option>
          <option value="10n">10n &mdash; Rain (night)</option>
          <option value="11">11 &mdash; Thunderstorm</option>
          <option value="13">13 &mdash; Snow</option>
          <option value="50">50 &mdash; Mist / Fog</option>
        </select>
        <button class="btn btn-a" onclick="testWeatherIcon()">Test (5s)</button>
      </div>
    </div>
    <div class="card">
      <div class="clbl">Show every &mdash; <span class="rv" id="v-wi">-</span> min</div>
      <input type="range" id="weather_interval" min="1" max="120" step="1" oninput="document.getElementById('v-wi').textContent=this.value;deb('weather_interval',this.value)">
    </div>
    <div class="card">
      <div class="clbl">Show for &mdash; <span class="rv" id="v-wd">-</span> sec</div>
      <input type="range" id="weather_duration" min="3" max="30" step="1" oninput="document.getElementById('v-wd').textContent=this.value;deb('weather_duration',this.value)">
    </div>
  </div>
</div>

<!-- 09 SMART HOME (online only) -->
<div class="sec" id="sec-smart">
  <div class="sec-hd"><span class="sec-n">10</span><span class="sec-t">Smart Home</span><span class="sec-l"></span></div>
  <div class="grid g1">
    <div class="card">
      <div class="clbl">Control via Web URL</div>
      <div class="cval">Control the LEDs from your smart home via: <span class="cmono" id="smart_url">-</span></div>
    </div>
  </div>
</div>

<!-- 10 UPDATE (online only) -->
<div class="sec" id="sec-update">
  <div class="sec-hd"><span class="sec-n">11</span><span class="sec-t">Update</span><span class="sec-l"></span></div>
  <div class="grid g1">
    <div class="card">
      <div class="clbl">OTA Firmware Update</div>
      <div style="display:flex;align-items:center;gap:14px;flex-wrap:wrap">
        <button class="btn btn-g" id="btn-ota" onclick="toggleOTA()">Activate Update Mode</button>
        <div class="cval" style="font-size:12.5px">URL: <span class="cmono" id="ota_url">-</span></div>
      </div>
    </div>
  </div>
</div>

<!-- 11 MAINTENANCE -->
<div class="sec">
  <div class="sec-hd"><span class="sec-n">12</span><span class="sec-t">Maintenance</span><span class="sec-l"></span></div>
  <div class="grid">
    <div class="card">
      <div class="clbl">Restart WordClock</div>
      <button class="btn btn-a" onclick="doRestart()">Restart</button>
    </div>
    <div class="card" id="card-wifireset">
      <div class="clbl">Reset WiFi Settings</div>
      <button class="btn btn-d" onclick="doWifiReset()">Reset WiFi</button>
    </div>
    <div class="card">
      <div class="clbl">Reset All Settings</div>
      <button class="btn btn-g" onclick="doSettingsReset()">Reset Settings</button>
    </div>
  </div>
</div>

<div style="text-align:center;font-size:11px;color:var(--ink3);padding-top:8px">
  NonCommercial &mdash; Not for commercial use &mdash; AWSW WordClock
</div>

</div>
<div class="toast" id="toast"></div>

<script>
var dbt={};
function deb(k,v){clearTimeout(dbt[k]);dbt[k]=setTimeout(function(){s(k,v);},500);}
function debStr(k,v){clearTimeout(dbt[k]);dbt[k]=setTimeout(function(){s(k,v);},1200);}

function s(key,val){
  fetch('/api/set?'+key+'='+encodeURIComponent(val))
    .then(function(r){return r.json();})
    .then(function(d){if(d.ok)toast('Saved');else toast('Error','e');})
    .catch(function(){toast('Error','e');});
}

function sr(key,val){
  if(!confirm('This change requires a restart. Continue?'))return;
  fetch('/api/set?'+key+'='+encodeURIComponent(val))
    .then(function(){toast('Restarting...');setTimeout(function(){location.reload();},4500);})
    .catch(function(){toast('Error','e');});
}

function setNtpSyncTime(v){
  var p=v.split(':');if(p.length<2)return;
  s('ntp_sync_hour',parseInt(p[0]));
  setTimeout(function(){s('ntp_sync_minute',parseInt(p[1]));},300);
}

function doRestart(){
  if(!confirm('Restart WordClock?'))return;
  fetch('/api/restart').then(function(){toast('Restarting...');setTimeout(function(){location.reload();},4500);});
}
function doWifiReset(){
  if(!confirm('Reset WiFi? WordClock will restart and open the setup portal.'))return;
  fetch('/api/wifi_reset').then(function(){toast('WiFi reset. Reconnect to WordClock AP.');});
}
function doSettingsReset(){
  if(!confirm('Reset all settings? WiFi and language are kept.'))return;
  fetch('/api/settings_reset').then(function(){toast('Done');setTimeout(function(){location.reload();},2000);});
}
var otaActive=false;
function toggleOTA(){
  var url=otaActive?'/api/ota_off':'/api/ota';
  fetch(url).then(function(){
    otaActive=!otaActive;
    var b=el('btn-ota');
    if(otaActive){b.textContent='Deactivate Update Mode';b.style.background='#f1a000';b.style.color='#fff';}
    else{b.textContent='Activate Update Mode';b.style.background='';b.style.color='';}
    toast(otaActive?'Update mode active — use the OTA URL':'Update mode deactivated');
  });
}
function getDevTime(){
  var now=new Date();
  var ts=now.getFullYear()+'-'+
    String(now.getMonth()+1).padStart(2,'0')+'-'+
    String(now.getDate()).padStart(2,'0')+'T'+
    String(now.getHours()).padStart(2,'0')+':'+
    String(now.getMinutes()).padStart(2,'0')+':'+
    String(now.getSeconds()).padStart(2,'0')+'.000';
  fetch('/api/set?device_time='+encodeURIComponent(ts))
    .then(function(r){return r.json();})
    .then(function(d){
      if(d.ok){
        var n=new Date();
        document.getElementById('dev_time_status').textContent=
          'Set: '+String(n.getHours()).padStart(2,'0')+':'+String(n.getMinutes()).padStart(2,'0')+':'+String(n.getSeconds()).padStart(2,'0');
        toast('Time synced');
      }
    });
}

function testAnimation(){
  var mode=document.getElementById('animation_mode').value;
  if(mode=='0'){toast('No effect selected','e');return;}
  fetch('/api/animation_test?mode='+mode)
    .then(function(r){return r.json();})
    .then(function(d){if(d.ok)toast('Animation started');else toast('Error','e');})
    .catch(function(){toast('Error','e');});
}
function testWeatherIcon(){
  var code=document.getElementById('weather_test_icon').value;
  fetch('/api/weather_test?icon='+code)
    .then(function(r){return r.json();})
    .then(function(d){if(d.ok)toast('Showing icon for 5s');else toast('Error','e');})
    .catch(function(){toast('Error','e');});
}
function toast(msg,type){
  var t=document.getElementById('toast');
  t.textContent=msg;
  t.style.background=type==='e'?'#d24a3c':'#14140f';
  t.className='toast show';
  clearTimeout(t._t);
  t._t=setTimeout(function(){t.className='toast';},2500);
}

function el(id){return document.getElementById(id);}
function sv(id,v){var e=el(id);if(e)e.value=String(v);}
function st(id,v){var e=el(id);if(e)e.textContent=v;}
function sc(id,v){
  var e=el(id);if(!e)return;
  e.checked=!!v;
  var t=e.parentElement;if(t)t.classList.toggle('on',!!v);
}
function sd(id,v){var e=el(id);if(e)e.style.display=v?'':'none';}
// Toggle handler: immediate visual + text feedback, then API call
function tog(key,lblId,checked){
  var e=el(key);if(e){var t=e.parentElement;if(t)t.classList.toggle('on',checked);}
  if(lblId)st(lblId,checked?'On':'Off');
  s(key,checked?1:0);
}

function apply(d){
  st('fw',d.version);
  if(d.chip_temp!==undefined)st('chip_temp',d.chip_temp.toFixed(1)+'°C');
  if(d.lang_name)st('lang_name',d.lang_name);
  sv('color_time',d.color_time);sv('color_back',d.color_back);
  sc('random_color',d.random_color);st('l-rc',d.random_color?'On':'Off');
  sc('single_min',d.single_min);st('l-sm',d.single_min?'On':'Off');
  sc('night_mode',d.night_mode);st('l-nm',d.night_mode?'On':'Off');
  sv('brightness_day',d.brightness_day);st('v-bd',d.brightness_day);
  sv('brightness_night',d.brightness_night);st('v-bn',d.brightness_night);
  sv('day_start',d.day_start);sv('day_stop',d.day_stop);
  sc('online_mode',d.online_mode);st('l-om',d.online_mode?'Online':'Offline');
  st('mode_status',d.online_mode?'Online Mode — all functions active':'Offline Mode');
  sc('startup_text',d.startup_text);st('l-st',d.startup_text?'On':'Off');
  sc('show_ip',d.show_ip);st('l-si',d.show_ip?'On':'Off');
  sv('ntp_server',d.ntp_server);sv('timezone',d.timezone);
  st('night_status',d.night_status||'Not used');
  st('startup_time',d.startup_time||'-');
  st('ds3231_status',d.ds3231_found?'Connected ✓':'Not detected');
  if(d.dst_next)st('dst_next',d.dst_next);
  st('ssid',d.ssid||'-');st('rssi',(d.rssi||'-')+'dBm');
  st('ip',d.ip||'-');st('mac',d.mac||'-');
  st('hostname',d.hostname||'-');st('gateway',d.gateway||'-');
  var base='http://'+d.ip;
  st('smart_url',base+':2023');st('ota_url',base+':8080');
  if(el('web-ctrl-link'))el('web-ctrl-link').href=base+':2023/config?INTENSITYviaWEB=0';
  sd('web-ctrl-warn',d.web_control);
  sd('night-warn',d.night_warn);
  // Weather
  sv('weather_key',d.weather_key||'');
  sv('weather_lat',d.weather_lat||'');
  sv('weather_lon',d.weather_lon||'');
  sv('weather_interval',d.weather_interval||30);st('v-wi',d.weather_interval||30);
  sv('weather_duration',d.weather_duration||5);st('v-wd',d.weather_duration||5);
  if(d.weather_city){st('w-city',d.weather_city);st('w-desc',d.weather_desc||'-');st('w-temp',d.weather_temp||'-');st('w-icon',d.weather_icon||'-');sd('weather-info',true);};
  sv('animation_mode',d.animation_mode||0);
  sv('wifi_timeout',d.wifi_timeout||0);st('v-wt',(d.wifi_timeout||0)==0?'Aus':(d.wifi_timeout+' Min.'));
  if(d.ntp_sync_hour!==undefined){var h=d.ntp_sync_hour,m=d.ntp_sync_minute||0;var e=el('ntp_sync_time');if(e)e.value=(h<10?'0'+h:h)+':'+(m<10?'0'+m:m);}
  var online=!!d.online_mode;
  ['sec-startup','sec-wifi','sec-time','sec-weather','sec-smart','card-wifireset'].forEach(function(id){sd(id,online);});
  sd('sec-offline',!online);
}

function poll(){fetch('/api/get').then(function(r){return r.json();}).then(apply).catch(function(){});}
function pollTime(){fetch('/api/time').then(function(r){return r.json();}).then(function(d){st('esp_time',d.esp_time||'--:--:--');st('rtc_local',d.rtc_local||'--:--:--');st('utc_time',d.utc_time||'--:--:--');st('utc_offset',d.utc_offset||'--');st('ds3231_status',d.ds3231_found?'Connected ✓':'Not detected');st('ds3231_temp',d.ds3231_temp||'--');setRtcSyncBtn(!!d.ds3231_sync);}).catch(function(){});}
var rtcSyncActive=true;
function setRtcSyncBtn(active){
  rtcSyncActive=active;
  var b=el('btn-rtcsync');
  if(!b)return;
  if(active){b.textContent='Abgleich pausieren';b.style.background='';b.style.color='';}
  else{b.textContent='Abgleich fortsetzen';b.style.background='#f1a000';b.style.color='#fff';}
}
function espShift(){
  var v=parseInt(el('esp_shift_sec').value,10)||0;
  if(!v){toast('Bitte Sekundenwert eingeben');return;}
  fetch('/api/esp_shift?sec='+v).then(function(){
    toast('ESP-Uhr um '+(v>0?'+':'')+v+' s verstellt — DS3231 unberührt');
  });
}
function toggleRtcSync(){
  var wasActive=rtcSyncActive;
  var url=wasActive?'/api/rtc_sync_off':'/api/rtc_sync_on';
  fetch(url).then(function(){
    setRtcSyncBtn(!wasActive);
    toast(wasActive?'RTC-Abgleich pausiert — ESP-Uhr läuft jetzt frei':'RTC-Abgleich aktiviert');
  });
}
poll();
pollTime();
setInterval(poll,10000);
setInterval(pollTime,1000);
function updateBrowserTime(){
  var n=new Date();
  var p=function(v){return(v<10?'0':'')+v;};
  st('browser_time',p(n.getDate())+'.'+p(n.getMonth()+1)+'.'+n.getFullYear()+' '+p(n.getHours())+':'+p(n.getMinutes())+':'+p(n.getSeconds()));
}
updateBrowserTime();
setInterval(updateBrowserTime,1000);
</script>
</body>
</html>
)rawliteral";

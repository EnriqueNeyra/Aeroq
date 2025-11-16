#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/core/defines.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/sensor/sensor.h"

#include <string>
#include <cmath>

namespace aeroq {

using esphome::sensor::Sensor;

static const char *const TAG = "aeroq_ui";

// Forward declarations for functions implemented in aeroq_display.h
namespace aeroq_display {
  bool get_temp_unit_f();
  void set_temp_unit_f(bool use_fahrenheit);
}

// Full HTML/JS UI. Right-hand panel fetches the latest version
// from your GitHub manifest.json and compares it to ESPHOME_PROJECT_VERSION.
static const char INDEX_HTML[] = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>Aeroq Air Quality Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <style>
    :root {
      --bg: #05060a;
      --card-bg: #111319;
      --accent: #00e0b8;
      --accent-soft: rgba(0, 224, 184, 0.12);
      --text-main: #f5f7ff;
      --text-muted: #9aa0b7;
      --border: #222633;
      --radius-lg: 18px;
      --radius-md: 10px;
      --shadow-soft: 0 18px 45px rgba(0, 0, 0, 0.45);
    }
    * {
      box-sizing: border-box;
      -webkit-font-smoothing: antialiased;
    }
    body {
      margin: 0;
      padding: 0;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      background: radial-gradient(circle at top, #171b2c 0, #05060a 55%);
      color: var(--text-main);
      font-family: system-ui, -apple-system, BlinkMacSystemFont, "SF Pro Text",
        "Segoe UI", sans-serif;
    }
    .shell {
      width: 100%;
      max-width: 960px;
      padding: 24px;
    }
    .frame {
      background: radial-gradient(circle at top left, #252b3d 0, #111319 50%);
      border-radius: 24px;
      padding: 22px 22px 20px;
      box-shadow: var(--shadow-soft);
      border: 1px solid rgba(255,255,255,0.04);
      backdrop-filter: blur(22px);
    }
    .header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      margin-bottom: 20px;
    }
    .title-block {
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .pill {
      width: 11px;
      height: 32px;
      border-radius: 50px;
      background: linear-gradient(180deg, var(--accent), #0c8b74);
      box-shadow: 0 0 12px rgba(0, 224, 184, 0.65);
    }
    h1 {
      margin: 0;
      font-size: 20px;
      letter-spacing: 0.04em;
      text-transform: uppercase;
      font-weight: 650;
    }
    .subtitle {
      margin: 0;
      margin-top: 2px;
      font-size: 12px;
      color: var(--text-muted);
    }
    .status-chip {
      padding: 6px 12px;
      border-radius: 999px;
      font-size: 11px;
      font-weight: 500;
      display: inline-flex;
      align-items: center;
      gap: 6px;
      background: rgba(0, 224, 184, 0.09);
      border: 1px solid rgba(0, 224, 184, 0.5);
      color: var(--accent);
    }
    .status-dot {
      width: 7px;
      height: 7px;
      border-radius: 999px;
      background: var(--accent);
      box-shadow: 0 0 8px rgba(0, 224, 184, 0.9);
      animation: breathe 1.6s ease-in-out infinite;
    }
    @keyframes breathe {
      0%, 100% {
        transform: scale(1);
        opacity: 0.75;
        box-shadow: 0 0 6px rgba(0, 224, 184, 0.7);
      }
      50% {
        transform: scale(1.6);
        opacity: 1;
        box-shadow: 0 0 18px rgba(0, 224, 184, 1);
      }
    }

    .grid {
      display: grid;
      grid-template-columns: minmax(0, 2fr) minmax(0, 1.15fr);
      gap: 16px;
    }

    .card {
      background: var(--card-bg);
      border-radius: var(--radius-lg);
      border: 1px solid var(--border);
      padding: 16px 16px 14px;
      position: relative;
      overflow: hidden;
    }

    .card-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      margin-bottom: 10px;
    }
    .card-title {
      font-size: 14px;
      font-weight: 600;
      letter-spacing: 0.04em;
      text-transform: uppercase;
      color: #d8ddff;
    }
    .card-subtitle {
      font-size: 11px;
      color: var(--text-muted);
    }

    /* Temp unit toggle */
    .unit-toggle {
      display: inline-flex;
      background: rgba(15,23,42,0.8);
      border-radius: 999px;
      border: 1px solid rgba(148,163,184,0.4);
      padding: 2px;
    }
    .unit-btn {
      border: none;
      background: transparent;
      color: var(--text-muted);
      font-size: 11px;
      padding: 3px 8px;
      border-radius: 999px;
      cursor: pointer;
    }
    .unit-btn--active {
      background: rgba(0,224,184,0.18);
      color: var(--accent);
    }

    .metrics-grid {
      display: grid;
      grid-template-columns: repeat(3, minmax(0, 1fr));
      gap: 10px;
      margin-top: 4px;
    }

    .metric {
      background: radial-gradient(circle at top, #232636 0, #151722 45%);
      border-radius: var(--radius-md);
      border: 1px solid rgba(255,255,255,0.04);
      padding: 8px 10px;
    }
    .metric-label {
      font-size: 11px;
      color: var(--text-muted);
      margin-bottom: 4px;
    }
    .metric-main {
      display: flex;
      align-items: baseline;
      gap: 4px;
    }
    .metric-value {
      font-size: 18px;
      font-weight: 620;
    }
    .metric-unit {
      font-size: 11px;
      color: var(--text-muted);
    }

    .chip-row {
      display: flex;
      flex-wrap: wrap;
      gap: 6px;
      margin-top: 6px;
    }
    .chip {
      font-size: 11px;
      padding: 4px 9px;
      border-radius: 999px;
      border: 1px solid rgba(255,255,255,0.06);
      background: rgba(0,0,0,0.18);
      color: var(--text-muted);
    }

    /* Firmware / OTA panel */
    .firmware-card {
      background: radial-gradient(circle at top, #1d1523 0, #151220 45%);
      border-radius: var(--radius-lg);
      border: 1px solid rgba(255,255,255,0.04);
      padding: 16px 16px 14px;
      position: relative;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
    }
    .firmware-card::before {
      content: "";
      position: absolute;
      inset: -80px;
      background: radial-gradient(circle at top right, rgba(0,224,184,0.16) 0, transparent 55%);
      opacity: 0.9;
      pointer-events: none;
    }
    .firmware-inner {
      position: relative;
      z-index: 1;
      display: flex;
      flex-direction: column;
      gap: 10px;
    }

    .fw-pill {
      display: inline-flex;
      align-items: center;
      gap: 4px;
      padding: 4px 8px;
      border-radius: 999px;
      border: 1px solid rgba(255,255,255,0.06);
      font-size: 10px;
      color: var(--text-muted);
      align-self: flex-start;
    }
    .fw-pill-dot {
      width: 6px;
      height: 6px;
      border-radius: 999px;
      background: var(--accent);
    }

    .fw-title-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
    }

    .fw-title {
      font-size: 14px;
      font-weight: 600;
      letter-spacing: 0.04em;
      text-transform: uppercase;
      margin: 0;
    }

    .fw-badge {
      font-size: 10px;
      padding: 4px 10px;
      border-radius: 999px;
      border: 1px solid rgba(255,255,255,0.16);
      background: rgba(0,0,0,0.25);
      color: var(--text-muted);
      white-space: nowrap;
    }
    .fw-badge--ok {
      border-color: rgba(0,224,184,0.6);
      color: var(--accent);
      background: rgba(0,224,184,0.08);
    }
    .fw-badge--warn {
      border-color: #ffb347;
      color: #ffcf7f;
      background: rgba(255,179,71,0.1);
    }
    .fw-badge--unknown {
      border-color: rgba(148,163,184,0.7);
      color: var(--text-muted);
      background: rgba(15,23,42,0.5);
    }

    .fw-sub {
      font-size: 11px;
      color: var(--text-muted);
      max-width: 260px;
      line-height: 1.4;
    }

    .fw-meta {
      display: grid;
      grid-template-columns: 1fr;
      gap: 4px;
      font-size: 11px;
      color: var(--text-muted);
      margin-top: 4px;
    }
    .fw-meta span strong {
      color: var(--text-main);
      font-weight: 500;
    }

    .fw-form {
      margin-top: 8px;
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      align-items: center;
    }
    .fw-file {
      font-size: 11px;
      color: var(--text-muted);
    }
    .fw-file input[type="file"] {
      font-size: 11px;
      max-width: 170px;
    }
    .fw-btn {
      border: none;
      outline: none;
      font-size: 11px;
      font-weight: 500;
      padding: 6px 14px;
      border-radius: 999px;
      cursor: pointer;
      background: var(--accent);
      color: #041716;
      box-shadow: 0 10px 24px rgba(0,224,184,0.4);
      transition: transform 0.12s ease, box-shadow 0.12s ease, filter 0.12s ease;
    }
    .fw-btn:hover {
      transform: translateY(-1px);
      filter: brightness(1.05);
      box-shadow: 0 12px 30px rgba(0,224,184,0.55);
    }
    .fw-btn:active {
      transform: translateY(0);
      box-shadow: 0 6px 18px rgba(0,224,184,0.35);
    }
    .fw-btn[disabled] {
      opacity: 0.4;
      cursor: default;
      box-shadow: none;
      transform: none;
    }

    .fw-status-text {
      font-size: 10px;
      color: var(--text-muted);
      margin-top: 4px;
    }

    .footer {
      margin-top: 14px;
      display: flex;
      justify-content: flex-end;
      font-size: 10px;
      color: var(--text-muted);
      opacity: 0.7;
    }

    @media (max-width: 720px) {
      .frame {
        padding: 18px 16px 16px;
      }
      .grid {
        grid-template-columns: 1fr;
      }
      .firmware-card {
        order: -1;
      }
    }
  </style>
</head>
<body>
  <div class="shell">
    <div class="frame">
      <div class="header">
        <div class="title-block">
          <div class="pill"></div>
          <div>
            <h1>Aeroq</h1>
            <p class="subtitle">Local live view · Air quality telemetry</p>
          </div>
        </div>
        <div class="status-chip">
          <span class="status-dot"></span>
          <span id="status-text">Live</span>
        </div>
      </div>

      <div class="grid">
        <div class="card">
          <div class="card-header">
            <div>
              <div class="card-title">Environment</div>
              <div class="card-subtitle">Live sensor snapshot</div>
            </div>
            <div class="unit-toggle">
              <button class="unit-btn unit-btn--active" data-unit="F">°F</button>
              <button class="unit-btn" data-unit="C">°C</button>
            </div>
          </div>
          <div class="metrics-grid" id="metrics"></div>
          <div class="chip-row">
            <div class="chip" id="chip-last-updated">Last updated: —</div>
          </div>
        </div>

        <div class="firmware-card">
          <div class="firmware-inner">
            <div class="fw-pill">
              <div class="fw-pill-dot"></div>
              <span>Firmware</span>
            </div>

            <div class="fw-title-row">
              <h2 class="fw-title">Firmware & Updates</h2>
              <div id="fw-badge" class="fw-badge fw-badge--unknown">Checking…</div>
            </div>

            <p class="fw-sub">
              Manage firmware here when your Aeroq is on your home network.
              Upload a new <code>.bin</code> file to update over Wi-Fi, or check
              for updates from the Aeroq firmware channel.
            </p>

            <div class="fw-meta">
              <span>Current firmware: <strong id="fw-current">—</strong></span>
              <span>Latest available: <strong id="fw-latest">—</strong></span>
            </div>

            <form class="fw-form" id="ota-form" action="/update" method="POST" enctype="multipart/form-data">
              <label class="fw-file">
                <input type="file" name="firmware" id="fw-file-input" />
              </label>
              <button type="submit" class="fw-btn" id="fw-update-btn" disabled>
                Upload &amp; install
              </button>
            </form>

            <div class="fw-status-text" id="fw-status-text">
              Select a firmware file to enable the update button.
            </div>
          </div>
        </div>
      </div>

      <div class="footer">
        <span id="footer-ip">Device: aeroq.local</span>
      </div>
    </div>
  </div>

  <script>
    // --- Live metrics + firmware status + temp unit ---

    const metricsEl = document.getElementById('metrics');
    const lastUpdatedChip = document.getElementById('chip-last-updated');
    const statusText = document.getElementById('status-text');

    const fwCurrentEl = document.getElementById('fw-current');
    const fwLatestEl = document.getElementById('fw-latest');
    const fwBadgeEl = document.getElementById('fw-badge');
    const fwStatusTextEl = document.getElementById('fw-status-text');
    const fwUpdateBtn = document.getElementById('fw-update-btn');
    const fwFileInput = document.getElementById('fw-file-input');

    const unitButtons = document.querySelectorAll('.unit-btn');

    const MANIFEST_URL = 'https://enriqueneyra.github.io/Aeroq/manifest.json';

    let currentFwVersion = null;
    let latestFwVersion = null;
    let manifestError = false;
    let currentTempUnit = 'F';

    function formatNumber(v, digits = 1) {
      if (v === null || v === undefined || Number.isNaN(v)) return '—';
      return Number(v).toFixed(digits);
    }

    function applyUnitToUI(unit) {
      currentTempUnit = unit;
      unitButtons.forEach(btn => {
        btn.classList.toggle('unit-btn--active', btn.dataset.unit === unit);
      });
    }

    unitButtons.forEach(btn => {
      btn.addEventListener('click', async () => {
        const unit = btn.dataset.unit;
        applyUnitToUI(unit);
        try {
          await fetch('/api/set_temp_unit?u=' + encodeURIComponent(unit), {
            method: 'POST'
          });
        } catch (e) {
          // ignore; device state will be reflected on next poll
        }
      });
    });

    function renderMetrics(data) {
      const unit = (data.temp_unit === 'C' || data.temp_unit === 'F') ? data.temp_unit : 'F';
      applyUnitToUI(unit);

      const tScdRaw = data.t_scd;
      const tSenRaw = data.t_sen;

      const tScd = unit === 'F' ? (tScdRaw * 9/5 + 32) : tScdRaw;
      const tSen = unit === 'F' ? (tSenRaw * 9/5 + 32) : tSenRaw;

      const items = [
        { label: 'CO\u2082',        value: data.co2,   unit: 'ppm',           decimals: 1 },
        { label: 'Temp (SCD41)',   value: tScd,       unit: '°' + unit,      decimals: 1 },
        { label: 'RH (SCD41)',     value: data.rh_scd, unit: '%',            decimals: 1 },
        { label: 'Temp (SEN54)',   value: tSen,       unit: '°' + unit,      decimals: 1 },
        { label: 'RH (SEN54)',     value: data.rh_sen, unit: '%',            decimals: 1 },
        { label: 'PM1.0',          value: data.pm1,   unit: 'µg/m³',         decimals: 1 },
        { label: 'PM2.5',          value: data.pm25,  unit: 'µg/m³',         decimals: 1 },
        { label: 'PM4.0',          value: data.pm4,   unit: 'µg/m³',         decimals: 1 },
        { label: 'PM10',           value: data.pm10,  unit: 'µg/m³',         decimals: 1 },
        { label: 'VOC Index',      value: data.voc,   unit: '',              decimals: 0 }
      ];

      metricsEl.innerHTML = items.map(m => {
        const val = formatNumber(m.value, m.decimals);
        return `
          <div class="metric">
            <div class="metric-label">${m.label}</div>
            <div class="metric-main">
              <div class="metric-value">${val}</div>
              <div class="metric-unit">${m.unit}</div>
            </div>
          </div>
        `;
      }).join('');

      const now = new Date();
      lastUpdatedChip.textContent =
        'Last updated: ' + now.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
    }

    function compareVersions(a, b) {
      // Returns 1 if a>b, -1 if a<b, 0 if equal / invalid.
      if (!a || !b) return 0;
      const pa = String(a).split('.').map(x => parseInt(x, 10) || 0);
      const pb = String(b).split('.').map(x => parseInt(x, 10) || 0);
      const len = Math.max(pa.length, pb.length);
      for (let i = 0; i < len; i++) {
        const va = pa[i] || 0;
        const vb = pb[i] || 0;
        if (va > vb) return 1;
        if (va < vb) return -1;
      }
      return 0;
    }

    function renderFirmwareStatus() {
      if (!currentFwVersion) {
        fwCurrentEl.textContent = '—';
      } else {
        fwCurrentEl.textContent = currentFwVersion;
      }

      if (manifestError) {
        fwLatestEl.textContent = 'Unavailable';
        fwBadgeEl.textContent = 'Status unknown';
        fwBadgeEl.className = 'fw-badge fw-badge--unknown';
        fwStatusTextEl.textContent = 'Could not reach the Aeroq update channel from this browser.';
        return;
      }

      if (!latestFwVersion) {
        fwLatestEl.textContent = 'Checking…';
        fwBadgeEl.textContent = 'Checking…';
        fwBadgeEl.className = 'fw-badge fw-badge--unknown';
        fwStatusTextEl.textContent = 'Checking for the latest firmware version…';
        return;
      }

      fwLatestEl.textContent = latestFwVersion;

      const cmp = compareVersions(latestFwVersion, currentFwVersion);
      if (cmp > 0) {
        fwBadgeEl.textContent = 'Update available';
        fwBadgeEl.className = 'fw-badge fw-badge--warn';
        fwStatusTextEl.textContent = 'A newer firmware is available. Upload the new .bin file to update over Wi-Fi.';
      } else {
        fwBadgeEl.textContent = 'Up to date';
        fwBadgeEl.className = 'fw-badge fw-badge--ok';
        fwStatusTextEl.textContent = 'This device is running the latest known firmware.';
      }
    }

    function renderFirmwareFromState(data) {
      currentFwVersion = data.fw_version || null;
      renderFirmwareStatus();
    }

    fwFileInput.addEventListener('change', () => {
      const hasFile = fwFileInput.files && fwFileInput.files.length > 0;
      fwUpdateBtn.disabled = !hasFile;
      if (hasFile) {
        fwStatusTextEl.textContent = 'Click "Upload & install" to start the OTA update over Wi-Fi.';
      } else {
        renderFirmwareStatus();
      }
    });

    async function pollState() {
      try {
        const res = await fetch('/api/state', { cache: 'no-store' });
        if (!res.ok) throw new Error('bad status');
        const json = await res.json();
        renderMetrics(json);
        renderFirmwareFromState(json);
        statusText.textContent = 'Live';
      } catch (e) {
        statusText.textContent = 'Offline?';
      }
    }

    async function fetchManifestVersion() {
      try {
        const res = await fetch(MANIFEST_URL, { cache: 'no-store' });
        if (!res.ok) throw new Error('bad status');
        const man = await res.json();

        // Try to be flexible about manifest shape.
        let v = null;
        if (man.version) {
          v = man.version;
        } else if (man.firmware && man.firmware.version) {
          v = man.firmware.version;
        } else if (Array.isArray(man.builds) && man.builds[0] && man.builds[0].version) {
          v = man.builds[0].version;
        }
        latestFwVersion = v;
        manifestError = false;
      } catch (e) {
        manifestError = true;
        latestFwVersion = null;
      }
      renderFirmwareStatus();
    }

    // Poll local state every 5s, check manifest once on load and then every 6h.
    setInterval(pollState, 5000);
    pollState();

    fetchManifestVersion();
    setInterval(fetchManifestVersion, 6 * 60 * 60 * 1000);
  </script>
</body>
</html>)HTML";

class AeroqUI : public esphome::Component, public AsyncWebHandler {
 public:
  void set_co2(Sensor *s)    { co2_ = s; }
  void set_pm25(Sensor *s)   { pm25_ = s; }
  void set_t_scd(Sensor *s)  { t_scd_ = s; }
  void set_rh_scd(Sensor *s) { rh_scd_ = s; }
  void set_t_sen(Sensor *s)  { t_sen_ = s; }
  void set_rh_sen(Sensor *s) { rh_sen_ = s; }
  void set_pm1(Sensor *s)    { pm1_ = s; }
  void set_pm4(Sensor *s)    { pm4_ = s; }
  void set_pm10(Sensor *s)   { pm10_ = s; }
  void set_voc(Sensor *s)    { voc_ = s; }

  void setup() override {
    auto *ws = esphome::web_server_base::global_web_server_base;
    if (ws == nullptr) {
      ESP_LOGW(TAG, "web_server_base not initialized; UI will not be available");
      return;
    }
    ws->add_handler(this);
    ESP_LOGI(TAG, "Aeroq UI handler registered");
  }

  void dump_config() override {
    ESP_LOGCONFIG(TAG, "Aeroq custom web UI");
#ifdef ESPHOME_PROJECT_VERSION
    ESP_LOGCONFIG(TAG, "  Firmware version: %s", ESPHOME_PROJECT_VERSION);
#else
    ESP_LOGCONFIG(TAG, "  Firmware version: unknown");
#endif
  }

  // AsyncWebHandler interface
  bool canHandle(AsyncWebServerRequest *request) const override {
    auto url = request->url();
    if (url == "/" || url == "/index.html") return true;
    if (url.rfind("/api/", 0) == 0) return true;
    return false;
  }

  void handleRequest(AsyncWebServerRequest *request) override {
    auto url = request->url();
    if (url == "/" || url == "/index.html") {
      handle_root_(request);
    } else if (url == "/api/state") {
      handle_state_(request);
    } else if (url == "/api/set_temp_unit") {
      handle_set_temp_unit_(request);
    } else {
      request->send(404, "text/plain", "Not found");
    }
  }

  bool isRequestHandlerTrivial() const override { return false; }

 private:
  Sensor *co2_{nullptr};
  Sensor *pm25_{nullptr};
  Sensor *t_scd_{nullptr};
  Sensor *rh_scd_{nullptr};
  Sensor *t_sen_{nullptr};
  Sensor *rh_sen_{nullptr};
  Sensor *pm1_{nullptr};
  Sensor *pm4_{nullptr};
  Sensor *pm10_{nullptr};
  Sensor *voc_{nullptr};

  static float safe_state_(Sensor *s) {
    if (s == nullptr) return NAN;
    return s->state;
  }

  void handle_root_(AsyncWebServerRequest *request) {
    request->send(200, "text/html", INDEX_HTML);
  }

  void handle_state_(AsyncWebServerRequest *request) {
    char json[640];
    const char *ver =
#ifdef ESPHOME_PROJECT_VERSION
        ESPHOME_PROJECT_VERSION;
#else
        "unknown";
#endif

    const bool use_f = aeroq_display::get_temp_unit_f();

    snprintf(json, sizeof(json),
             "{\"co2\":%.1f,"
             "\"t_scd\":%.2f,\"rh_scd\":%.2f,"
             "\"t_sen\":%.2f,\"rh_sen\":%.2f,"
             "\"pm1\":%.1f,\"pm25\":%.1f,\"pm4\":%.1f,\"pm10\":%.1f,"
             "\"voc\":%.1f,"
             "\"fw_version\":\"%s\","
             "\"temp_unit\":\"%s\"}",
             safe_state_(co2_), safe_state_(t_scd_), safe_state_(rh_scd_),
             safe_state_(t_sen_), safe_state_(rh_sen_),
             safe_state_(pm1_), safe_state_(pm25_), safe_state_(pm4_),
             safe_state_(pm10_), safe_state_(voc_),
             ver,
             use_f ? "F" : "C");

    request->send(200, "application/json", json);
  }

  void handle_set_temp_unit_(AsyncWebServerRequest *request) {
    if (request->hasParam("u")) {
      auto *p = request->getParam("u");
      auto v = p->value();
      bool use_f = (v == "F" || v == "f");
      aeroq_display::set_temp_unit_f(use_f);
      request->send(200, "application/json", "{\"ok\":true}");
    } else {
      request->send(400, "application/json", "{\"ok\":false,\"error\":\"missing u\"}");
    }
  }
};

}  // namespace aeroq

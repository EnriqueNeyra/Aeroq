#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/sensor/sensor.h"

#include <string>
#include <cmath>

namespace aeroq {

using esphome::sensor::Sensor;

static const char *const TAG = "aeroq_ui";

// Minimal-but-modern HTML UI; you can tweak styling later.
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
    }

    .grid {
      display: grid;
      grid-template-columns: minmax(0, 2fr) minmax(0, 1.1fr);
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

    .firmware-card {
      background: radial-gradient(circle at top, #1d1523 0, #151220 45%);
      border-radius: var(--radius-lg);
      border: 1px solid rgba(255,255,255,0.04);
      padding: 16px 16px 14px;
      position: relative;
      overflow: hidden;
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
      margin-bottom: 6px;
    }
    .fw-pill-dot {
      width: 6px;
      height: 6px;
      border-radius: 999px;
      background: var(--accent);
    }

    .fw-title {
      font-size: 14px;
      font-weight: 600;
      letter-spacing: 0.04em;
      text-transform: uppercase;
      margin-bottom: 4px;
    }
    .fw-sub {
      font-size: 11px;
      color: var(--text-muted);
      margin-bottom: 10px;
      max-width: 260px;
    }

    .fw-status {
      font-size: 11px;
      margin-top: 6px;
      color: var(--text-muted);
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
            <div class="fw-title">Browser view only</div>
            <div class="fw-sub">
              OTA uploads are handled by your main tooling for now. This panel is just a local status view.
            </div>
            <div class="fw-status" id="fw-status">
              Ready on <strong>aeroq.local</strong>
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
    const metricsEl = document.getElementById('metrics');
    const lastUpdatedChip = document.getElementById('chip-last-updated');
    const statusText = document.getElementById('status-text');

    function formatNumber(v, digits = 1) {
      if (v === null || v === undefined || Number.isNaN(v)) return '—';
      return Number(v).toFixed(digits);
    }

    function renderMetrics(data) {
      const items = [
        { key: 'co2', label: 'CO\u2082', unit: 'ppm' },
        { key: 't_scd', label: 'Temp (SCD41)', unit: '°C' },
        { key: 'rh_scd', label: 'RH (SCD41)', unit: '%' },
        { key: 't_sen', label: 'Temp (SEN54)', unit: '°C' },
        { key: 'rh_sen', label: 'RH (SEN54)', unit: '%' },
        { key: 'pm1', label: 'PM1.0', unit: 'µg/m³' },
        { key: 'pm25', label: 'PM2.5', unit: 'µg/m³' },
        { key: 'pm4', label: 'PM4.0', unit: 'µg/m³' },
        { key: 'pm10', label: 'PM10', unit: 'µg/m³' },
        { key: 'voc', label: 'VOC Index', unit: '' }
      ];

      metricsEl.innerHTML = items.map(m => {
        const raw = data[m.key];
        const val = m.key.startsWith('pm') || m.key === 'voc'
          ? formatNumber(raw, 0)
          : formatNumber(raw, 1);
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

    async function pollState() {
      try {
        const res = await fetch('/api/state', { cache: 'no-store' });
        if (!res.ok) throw new Error('bad status');
        const json = await res.json();
        renderMetrics(json);
        statusText.textContent = 'Live';
      } catch (e) {
        statusText.textContent = 'Offline?';
      }
    }

    setInterval(pollState, 5000);
    pollState();
  </script>
</body>
</html>)HTML";

class AeroqUI : public esphome::Component, public AsyncWebHandler {
 public:
  // matches what main.cpp + Python shim expect
  void set_basic_auth(const std::string &u, const std::string &p) {
    user_ = u;
    pass_ = p;
  }

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
  }

  // AsyncWebHandler interface
  bool canHandle(AsyncWebServerRequest *request) const override {
    auto url = request->url();  // std::string in IDF backend
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
    } else {
      request->send(404, "text/plain", "Not found");
    }
  }

  bool isRequestHandlerTrivial() const override { return false; }

 private:
  std::string user_;
  std::string pass_;

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
    // NOTE: for now we ignore basic auth; credentials are stored but unused
    request->send(200, "text/html", INDEX_HTML);
  }

  void handle_state_(AsyncWebServerRequest *request) {
    char json[512];
    snprintf(json, sizeof(json),
             "{\"co2\":%.1f,"
             "\"t_scd\":%.2f,\"rh_scd\":%.2f,"
             "\"t_sen\":%.2f,\"rh_sen\":%.2f,"
             "\"pm1\":%.1f,\"pm25\":%.1f,\"pm4\":%.1f,\"pm10\":%.1f,"
             "\"voc\":%.1f}",
             safe_state_(co2_), safe_state_(t_scd_), safe_state_(rh_scd_),
             safe_state_(t_sen_), safe_state_(rh_sen_),
             safe_state_(pm1_), safe_state_(pm25_), safe_state_(pm4_),
             safe_state_(pm10_), safe_state_(voc_));

    request->send(200, "application/json", json);
  }
};

}  // namespace aeroq

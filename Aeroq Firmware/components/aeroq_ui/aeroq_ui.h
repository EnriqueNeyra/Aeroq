// src/aeroq_ui.h
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/update/update.h"

#ifndef APP_VERSION
#define APP_VERSION ESPHOME_PROJECT_VERSION
#endif

namespace aeroq {

using esphome::Component;
using esphome::web_server_base::Request;
using WebServer = esphome::web_server_base::WebServer;

class AeroqUI final : public Component {
 public:
  AeroqUI(esphome::sensor::Sensor *co2,
          esphome::sensor::Sensor *pm25,
          esphome::sensor::Sensor *temp_pref,
          esphome::sensor::Sensor *hum_pref,
          esphome::update::UpdateComponent *up)
      : co2_(co2), pm25_(pm25), temp_(temp_pref), hum_(hum_pref), update_(up) {}

  void set_basic_auth(const std::string &user, const std::string &pass) {
    user_ = user; pass_ = pass;
  }

  void setup() override {
    auto *ws = esphome::web_server_base::global_web_server;
    if (!ws) return;

    // Root UI
    ws->on("/", HTTP_GET, [this](Request *req) {
      if (!this->check_auth_(req)) return;
      req->send_P(200, "text/html", INDEX_HTML, strlen_P(INDEX_HTML));
    });

    // JSON state
    ws->on("/api/state", HTTP_GET, [this](Request *req) {
      if (!this->check_auth_(req)) return;
      char json[512];
      const float co2 = safe_(co2_), pm  = safe_(pm25_), t = safe_(temp_), h = safe_(hum_);
      snprintf(json, sizeof(json),
        "{\"co2\":%.1f,\"pm25\":%.1f,\"temp\":%.2f,\"hum\":%.1f,"
        "\"fw\":\"%s\",\"build\":\"%s\"}",
        co2, pm, t, h, ESPHOME_PROJECT_VERSION, APP_VERSION);
      req->send(200, "application/json", json);
    });

    // Update check/install
    ws->on("/api/update/check", HTTP_POST, [this](Request *req) {
      if (!this->check_auth_(req)) return;
      if (update_) update_->check();
      req->send(200, "text/plain", "Update check requested");
    });

    ws->on("/api/update/install", HTTP_POST, [this](Request *req) {
      if (!this->check_auth_(req)) return;
      if (update_) update_->install();
      req->send(200, "text/plain", "Installing… device will reboot");
    });
  }

  void loop() override {}

 private:
  esphome::sensor::Sensor *co2_{nullptr};
  esphome::sensor::Sensor *pm25_{nullptr};
  esphome::sensor::Sensor *temp_{nullptr};
  esphome::sensor::Sensor *hum_{nullptr};
  esphome::update::UpdateComponent *update_{nullptr};

  std::string user_, pass_;

  static float safe_(esphome::sensor::Sensor *s) {
    if (!s) return 0.0f;
    const float v = s->state;
    return std::isnan(v) ? 0.0f : v;
  }

  bool check_auth_(Request *req) {
    if (user_.empty()) return true;  // no auth configured
    if (req->authenticate(user_.c_str(), pass_.c_str())) return true;
    req->requestAuthentication("Aeroq");
    return false;
  }

  // Keep the UI small and embedded (no filesystem needed)
  static const char INDEX_HTML[] PROGMEM;
};

// ================= HTML (inline) =================
const char AeroqUI::INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Aeroq • Local</title>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&display=swap">
<style>
:root{--bg:#050810;--panel:#0f1729;--soft:#0b0f1b;--accent:#46d6b7;--accent-soft:rgba(70,214,183,.12);
--text:#f9fafb;--muted:#9ca3af;--border:rgba(148,163,184,.35);--good:#22c55e;--warn:#f59e0b;--bad:#ef4444;--shadow:0 22px 60px rgba(15,23,42,.9)}
*{box-sizing:border-box}
body{margin:0;min-height:100vh;background:
 radial-gradient(circle at top left, rgba(70,214,183,.10), transparent 55%),
 radial-gradient(circle at bottom right, rgba(56,189,248,.08), transparent 55%),var(--bg);
 color:var(--text);font-family:Inter,system-ui,-apple-system,"Segoe UI",Roboto;display:flex;align-items:center;justify-content:center;padding:24px}
.shell{width:100%;max-width:980px;background:linear-gradient(135deg,rgba(15,23,42,.95),rgba(15,23,42,.98));border:1px solid var(--border);border-radius:24px;box-shadow:var(--shadow);padding:26px}
.header{display:flex;justify-content:space-between;gap:12px;align-items:center}
.eyebrow{display:inline-flex;align-items:center;gap:8px;padding:4px 10px;border-radius:999px;background:rgba(15,23,42,.9);border:1px solid var(--border);font-size:11px;letter-spacing:.14em;text-transform:uppercase;color:var(--muted)}
.dot{width:8px;height:8px;border-radius:999px;background:var(--accent);box-shadow:0 0 0 4px var(--accent-soft)}
.grid{margin-top:18px;display:grid;grid-template-columns:2fr 1fr;gap:18px}
.card{background:rgba(15,23,42,.92);border:1px solid var(--border);border-radius:18px;padding:16px}
.kv{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}
.tile{background:rgba(2,6,23,.9);border:1px solid rgba(51,65,85,.8);border-radius:14px;padding:12px}
.label{font-size:10px;color:#6b7280;text-transform:uppercase;letter-spacing:.14em}
.value{margin-top:4px;font-size:26px;font-weight:700}
.unit{font-size:12px;color:#9ca3af;margin-left:6px}
.row{display:flex;align-items:center;gap:10px;margin-top:8px}
.btn{appearance:none;border:none;cursor:pointer;font-weight:600;padding:10px 14px;border-radius:999px;background:var(--accent);color:#020617}
.btn:disabled{opacity:.6;cursor:not-allowed}
.small{font-size:12px;color:var(--muted)}
.footer{margin-top:16px;display:flex;justify-content:space-between;color:var(--muted);font-size:12px}
.badge{padding:2px 8px;border-radius:999px;border:1px solid rgba(148,163,184,.4);font-size:11px}
.status{font-size:12px;border-radius:999px;padding:4px 10px;border:1px solid rgba(148,163,184,.35);color:#e5e7eb}
.ok{background:rgba(5,46,22,.7);border-color:rgba(22,163,74,.8);color:#bbf7d0}
.warn{background:rgba(113,63,18,.6);border-color:rgba(245,158,11,.7);color:#fde68a}
.bad{background:rgba(127,29,29,.6);border-color:rgba(239,68,68,.7);color:#fecaca}
@media (max-width:768px){.grid{grid-template-columns:1fr}}
</style>
</head>
<body>
  <div class="shell">
    <div class="header">
      <div class="eyebrow"><span class="dot"></span><span>Aeroq · Local Web</span></div>
      <div id="air-status" class="status ok">Healthy</div>
    </div>

    <div class="grid">
      <div class="card">
        <div class="kv">
          <div class="tile"><div class="label">CO₂</div><div class="value"><span id="co2">—</span><span class="unit">ppm</span></div></div>
          <div class="tile"><div class="label">PM2.5</div><div class="value"><span id="pm25">—</span><span class="unit">µg/m³</span></div></div>
          <div class="tile"><div class="label">Temperature</div><div class="value"><span id="temp">—</span><span class="unit">°C</span></div></div>
          <div class="tile"><div class="label">Humidity</div><div class="value"><span id="hum">—</span><span class="unit">%</span></div></div>
        </div>
      </div>

      <div class="card">
        <div class="label">Firmware</div>
        <div class="row"><span id="fw" class="badge">—</span><button id="btnCheck" class="btn">Check for Update</button></div>
        <div class="row"><button id="btnInstall" class="btn" disabled>Install Update</button><span id="msg" class="small"></span></div>
      </div>
    </div>

    <div class="footer">
      <div>Visit on your LAN: <strong>aeroq.local</strong></div>
      <div>Build: <span id="build">—</span></div>
    </div>
  </div>

<script>
async function fetchState(){
  try{
    const r = await fetch('/api/state'); if(!r.ok) return;
    const j = await r.json();
    set('co2',  j.co2?.toFixed?.(0) ?? '—');
    set('pm25', j.pm25?.toFixed?.(0) ?? '—');
    set('temp', j.temp?.toFixed?.(1) ?? '—');
    set('hum',  j.hum?.toFixed?.(0) ?? '—');
    set('fw',   j.fw || '—'); set('build', j.build || '—');

    const status = document.getElementById('air-status');
    const co2 = j.co2 ?? 0, pm = j.pm25 ?? 0;
    let cls='ok', txt='Healthy';
    if(co2>1200 || pm>35){ cls='bad'; txt='Poor'; }
    else if(co2>800 || pm>12){ cls='warn'; txt='Moderate'; }
    status.className = 'status ' + cls; status.textContent = txt;
  }catch(e){}
}
function set(id,v){ const el=document.getElementById(id); if(el) el.textContent=v; }

document.getElementById('btnCheck').addEventListener('click', async ()=>{
  set('msg','Checking...');
  await fetch('/api/update/check', { method: 'POST' });
  set('msg','Update check requested');
  document.getElementById('btnInstall').disabled = false;
});
document.getElementById('btnInstall').addEventListener('click', async ()=>{
  document.getElementById('btnInstall').disabled = true;
  set('msg','Installing... device will reboot');
  await fetch('/api/update/install', { method: 'POST' });
});

fetchState(); setInterval(fetchState, 2000);
</script>
</body></html>)HTML";

}  // class AeroqUI

}  // namespace aeroq

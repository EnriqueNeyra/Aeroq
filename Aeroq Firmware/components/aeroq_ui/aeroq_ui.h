#pragma once

#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/update/update.h"
#include <cmath>

#ifndef APP_VERSION
#define APP_VERSION ESPHOME_PROJECT_VERSION
#endif

namespace aeroq {

using esphome::Component;
using esphome::web_server_base::Request;
using WebServer = esphome::web_server_base::WebServer;

// ============================================================================
//                               AeroqUI Component
// ============================================================================
class AeroqUI : public Component {
 public:
  // --- Setters for each required sensor ID (called from Python shim) --------
  void set_co2(esphome::sensor::Sensor *s)      { co2_ = s; }
  void set_pm25(esphome::sensor::Sensor *s)     { pm25_ = s; }
  void set_t_scd(esphome::sensor::Sensor *s)    { t_scd_ = s; }
  void set_rh_scd(esphome::sensor::Sensor *s)   { rh_scd_ = s; }
  void set_t_sen(esphome::sensor::Sensor *s)    { t_sen_ = s; }
  void set_rh_sen(esphome::sensor::Sensor *s)   { rh_sen_ = s; }
  void set_pm1(esphome::sensor::Sensor *s)      { pm1_ = s; }
  void set_pm4(esphome::sensor::Sensor *s)      { pm4_ = s; }
  void set_pm10(esphome::sensor::Sensor *s)     { pm10_ = s; }
  void set_voc(esphome::sensor::Sensor *s)      { voc_ = s; }

  // Generic setter: ESPHome gives us a Component* → cast to UpdateComponent*
  void set_update(esphome::Component *c) {
    update_ = dynamic_cast<esphome::update::UpdateComponent *>(c);
  }

  void set_basic_auth(const std::string &u, const std::string &p) {
    user_ = u;
    pass_ = p;
  }

  // ============================================================================
  //                                     SETUP
  // ============================================================================
  void setup() override {
    auto *ws = esphome::web_server_base::global_web_server;
    if (!ws) return;

    // --- Serve the custom UI at "/" -----------------------------------------
    ws->on("/", HTTP_GET, [this](Request *req) {
      if (!this->check_auth_(req)) return;
      req->send_P(200, "text/html", INDEX_HTML, strlen_P(INDEX_HTML));
    });

    // --- Live JSON state -----------------------------------------------------
    ws->on("/api/state", HTTP_GET, [this](Request *req) {
      if (!this->check_auth_(req)) return;

      auto f = [](esphome::sensor::Sensor *s) {
        if (!s) return 0.0f;
        float v = s->state;
        return (std::isnan(v) ? 0.0f : v);
      };

      char json[896];
      snprintf(json, sizeof(json),
        "{\"co2\":%.1f,"
         "\"scd_temp\":%.2f,\"scd_rh\":%.1f,"
         "\"sen_temp\":%.2f,\"sen_rh\":%.1f,"
         "\"pm1\":%.1f,\"pm25\":%.1f,\"pm4\":%.1f,\"pm10\":%.1f,"
         "\"voc\":%.1f,"
         "\"fw\":\"%s\",\"build\":\"%s\"}",
        f(co2_), f(t_scd_), f(rh_scd_), f(t_sen_), f(rh_sen_),
        f(pm1_), f(pm25_), f(pm4_), f(pm10_), f(voc_),
        ESPHOME_PROJECT_VERSION, APP_VERSION
      );

      req->send(200, "application/json", json);
    });

    // --- Update check --------------------------------------------------------
    ws->on("/api/update/check", HTTP_POST, [this](Request *req) {
      if (!this->check_auth_(req)) return;

      if (update_) update_->check();  // safe; update_ may be null
      req->send(200, "text/plain", "Update check requested");
    });

    // --- Update install ------------------------------------------------------
    ws->on("/api/update/install", HTTP_POST, [this](Request *req) {
      if (!this->check_auth_(req)) return;

      if (update_) update_->install();
      req->send(200, "text/plain", "Installing… device will reboot");
    });
  }

  void loop() override {}

 private:
  // --- Sensor pointers -------------------------------------------------------
  esphome::sensor::Sensor *co2_{nullptr};
  esphome::sensor::Sensor *pm25_{nullptr};
  esphome::sensor::Sensor *t_scd_{nullptr};
  esphome::sensor::Sensor *rh_scd_{nullptr};
  esphome::sensor::Sensor *t_sen_{nullptr};
  esphome::sensor::Sensor *rh_sen_{nullptr};
  esphome::sensor::Sensor *pm1_{nullptr};
  esphome::sensor::Sensor *pm4_{nullptr};
  esphome::sensor::Sensor *pm10_{nullptr};
  esphome::sensor::Sensor *voc_{nullptr};

  esphome::update::UpdateComponent *update_{nullptr};

  std::string user_;
  std::string pass_;

  // ============================================================================
  //                         BASIC AUTH HANDLING
  // ============================================================================
  bool check_auth_(Request *req) {
    if (user_.empty()) return true;

    if (req->authenticate(user_.c_str(), pass_.c_str()))
      return true;

    req->requestAuthentication("Aeroq");
    return false;
  }

  // ============================================================================
  //                               CUSTOM HTML PAGE
  // ============================================================================
  static const char INDEX_HTML[] PROGMEM;
};

// ============================================================================
//                              INLINE MINIFIED HTML
// ============================================================================
const char AeroqUI::INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head>

<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Aeroq • Local</title>

<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&display=swap">

<style>
:root{
 --bg:#050810;--panel:#0f1729;--accent:#46d6b7;--accent-soft:rgba(70,214,183,.12);
 --text:#f9fafb;--muted:#9ca3af;--border:rgba(148,163,184,.35)
}
*{box-sizing:border-box}
body{
 margin:0;min-height:100vh;
 background:
  radial-gradient(circle at 10% 0, rgba(70,214,183,.10),transparent 55%),
  radial-gradient(circle at 90% 100%, rgba(56,189,248,.08),transparent 55%),
  var(--bg);
 color:var(--text);
 font-family:Inter,system-ui,-apple-system,"Segoe UI";
 display:flex;align-items:center;justify-content:center;padding:24px
}
.shell{
 width:100%;max-width:980px;
 background:linear-gradient(135deg,rgba(15,23,42,.95),rgba(15,23,42,.98));
 border:1px solid var(--border);border-radius:24px;
 padding:26px;box-shadow:0 22px 60px rgba(15,23,42,.9)
}
.header{
 display:flex;justify-content:space-between;align-items:center;gap:12px
}
.eyebrow{
 display:inline-flex;align-items:center;gap:8px;
 padding:4px 10px;border-radius:999px;
 background:rgba(15,23,42,.9);border:1px solid var(--border);
 font-size:11px;letter-spacing:.14em;text-transform:uppercase;color:var(--muted)
}
.dot{
 width:8px;height:8px;border-radius:999px;
 background:var(--accent);box-shadow:0 0 0 4px var(--accent-soft)
}
.grid{
 margin-top:18px;
 display:grid;grid-template-columns:2fr 1fr;gap:18px
}
.card{
 background:rgba(15,23,42,.92);
 border:1px solid var(--border);
 border-radius:18px;
 padding:16px
}
.kv{
 display:grid;
 grid-template-columns:repeat(2,minmax(0,1fr));
 gap:12px
}
.tile{
 background:rgba(2,6,23,.9);
 border:1px solid rgba(51,65,85,.8);
 border-radius:14px;
 padding:12px
}
.label{
 font-size:10px;color:#6b7280;text-transform:uppercase;letter-spacing:.14em
}
.value{
 margin-top:4px;font-size:26px;font-weight:700
}
.unit{font-size:12px;color:#9ca3af;margin-left:6px}
.row{display:flex;align-items:center;gap:10px;margin-top:8px}
.btn{
 appearance:none;border:none;cursor:pointer;
 font-weight:600;padding:10px 14px;border-radius:999px;
 background:var(--accent);color:#020617
}
.btn:disabled{opacity:.6;cursor:not-allowed}
.small{font-size:12px;color:var(--muted)}
.footer{
 margin-top:16px;
 display:flex;justify-content:space-between;
 color:var(--muted);font-size:12px
}
.badge{
 padding:2px 8px;border-radius:999px;
 border:1px solid rgba(148,163,184,.4);
 font-size:11px
}
.status{
 font-size:12px;border-radius:999px;padding:4px 10px;
 border:1px solid rgba(148,163,184,.35);color:#e5e7eb
}
.ok{background:rgba(5,46,22,.7);border-color:rgba(22,163,74,.8);color:#bbf7d0}
.warn{background:rgba(113,63,18,.6);border-color:rgba(245,158,11,.7);color:#fde68a}
.bad{background:rgba(127,29,29,.6);border-color:rgba(239,68,68,.7);color:#fecaca}
@media(max-width:768px){.grid{grid-template-columns:1fr}}
</style>
</head>

<body>
<div class="shell">
 <div class="header">
   <div class="eyebrow"><span class="dot"></span><span>Aeroq · Local Web</span></div>
   <div id="air" class="status ok">Healthy</div>
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
     <div class="row"><span id="fw" class="badge">—</span>
       <button id="chk" class="btn">Check for Update</button>
     </div>
     <div class="row">
       <button id="ins" class="btn" disabled>Install Update</button>
       <span id="msg" class="small"></span>
     </div>
   </div>
 </div>

 <div class="footer">
   <div>Visit: <strong>aeroq.local</strong></div>
   <div>Build: <span id="build">—</span></div>
 </div>
</div>

<script>
async function S(){
 try{
   const r = await fetch('/api/state');
   if(!r.ok) return;
   const j = await r.json();

   g('co2', j.co2?.toFixed?.(0) ?? '—');
   g('pm25', j.pm25?.toFixed?.(0) ?? '—');
   g('temp', (j.scd_temp ?? j.sen_temp)?.toFixed?.(1) ?? '—');
   g('hum',  (j.scd_rh   ?? j.sen_rh)?.toFixed?.(0) ?? '—');

   g('fw',    j.fw || '—');
   g('build', j.build || '—');

   let st='ok',txt='Healthy';
   const co2=j.co2??0, pm=j.pm25??0;
   if(co2>1200||pm>35){st='bad';txt='Poor'}
   else if(co2>800||pm>12){st='warn';txt='Moderate'}
   const el=document.getElementById('air');
   el.className='status '+st; el.textContent=txt;

 }catch(e){}
}
function g(id,v){const el=document.getElementById(id); if(el) el.textContent=v;}

document.getElementById('chk').addEventListener('click', async ()=>{
 g('msg','Checking...');
 await fetch('/api/update/check',{method:'POST'});
 g('msg','Update check requested');
 document.getElementById('ins').disabled=false;
});

document.getElementById('ins').addEventListener('click', async ()=>{
 document.getElementById('ins').disabled=true;
 g('msg','Installing... device will reboot');
 await fetch('/api/update/install',{method:'POST'});
});

S(); setInterval(S,2000);
</script>

</body></html>)HTML";

}  // namespace aeroq

#include "aws_iot.h"
#include "secrets.h"
#include "certs.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClientSecure net;
static PubSubClient mqtt(net);

static void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  for (int i=0; i<40 && now < 8*3600*2; ++i) { delay(500); now = time(nullptr); }
}

static void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(250); }
}

static void mqttConnect() {
  while (!mqtt.connected()) {
    if (mqtt.connect(THING_NAME)) break;
    delay(1000);
  }
}

void aws_iot_init() {
  wifiConnect();
  setClock();

  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  net.setHandshakeTimeout(30);

  mqtt.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
  mqtt.setKeepAlive(60);
  mqtt.setBufferSize(1024);
  mqttConnect();

  aws_iot_publish_status("boot");
}

void aws_iot_loop() {
  if (WiFi.status() != WL_CONNECTED) wifiConnect();
  if (!mqtt.connected()) mqttConnect();
  mqtt.loop();
}

bool aws_iot_publish_status(const char* msg) {
  return mqtt.publish(TOPIC_STATUS, msg, false);
}

bool aws_iot_publish_telemetry(const Telemetry& t) {
  ArduinoJson::JsonDocument doc;   // v7 API
  doc["schema"]     = 1;
  doc["device_id"]  = t.device_id;
  doc["ts_ms"]      = (uint64_t) millis();
  doc["co2"]        = t.co2;
  doc["pm25"]       = t.pm25;
  doc["pm10"]       = t.pm10;
  doc["voc_index"]  = t.voc_index;
  doc["temp_c"]     = t.temp_c;
  doc["rh"]         = t.rh;

  char buf[320];
  size_t n = serializeJson(doc, buf, sizeof(buf));
  return mqtt.publish(TOPIC_TELEMETRY, buf, n);
}

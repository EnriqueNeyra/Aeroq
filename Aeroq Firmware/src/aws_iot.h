#pragma once
#include <Arduino.h>

struct Telemetry {
  const char* device_id;
  uint16_t co2;
  float pm25, pm10;
  float voc_index;
  float temp_c;
  float rh;
};

void aws_iot_init();                             // Wi-Fi + time + TLS + MQTT connect
void aws_iot_loop();                             // keep MQTT alive (call each loop)
bool aws_iot_publish_status(const char* msg);    // optional status/LWT use
bool aws_iot_publish_telemetry(const Telemetry& t);

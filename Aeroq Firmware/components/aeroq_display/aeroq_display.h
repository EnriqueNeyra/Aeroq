#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <GxEPD2_BW.h>

// Fonts used in your legacy layout
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

namespace aeroq {

namespace aeroq_display {

// ==== SPI + Display Pin Configuration (from legacy main.cpp) ====
#define EPD_MOSI  7
#define EPD_SCK   6
#define EPD_CS    5
#define EPD_DC    4
#define EPD_RST   3
#define EPD_BUSY  1

// GDEY037T03 (416x240)
static GxEPD2_BW<
    GxEPD2_370_GDEY037T03,
    GxEPD2_370_GDEY037T03::HEIGHT> display(
        GxEPD2_370_GDEY037T03(EPD_CS, EPD_DC, EPD_RST, -1));

// --- State ---
static bool s_inited = false;

// Temperature unit flag: true = °F, false = °C (default °F)
static bool g_temp_unit_f = true;

// --- Cached readings (always stored in °C internally) ---
static uint16_t g_co2 = 0;
static float g_scdTemp = NAN, g_scdRH = NAN;
static float g_senTemp = NAN, g_senRH = NAN;
static float g_pm1 = NAN, g_pm25 = NAN, g_pm4 = NAN, g_pm10 = NAN;
static float g_voc = NAN;

// --- Public helpers to control unit from YAML / other components ---
inline void set_temp_unit_f(bool use_fahrenheit) { g_temp_unit_f = use_fahrenheit; }
inline bool get_temp_unit_f() { return g_temp_unit_f; }

// --- Helpers ---
inline void init_once() {
  if (s_inited) return;

  // SPI like legacy: remap only SCK & MOSI; no MISO
  SPI.begin(EPD_SCK, -1, EPD_MOSI);

  // Init display
  display.init(115200);
  display.setRotation(1);
  pinMode(EPD_BUSY, INPUT);  // safe; legacy didn't set it, but harmless

  // Clear screen fully (legacy did this prior to first draw)
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  s_inited = true;
}

inline void render_legacy_layout() {

  // Chosen display unit
  const bool useF = g_temp_unit_f;
  const char unitChar = useF ? 'F' : 'C';

  // Convert the SEN temp for display if needed
  const float displayTemp =
      useF ? (g_senTemp * 9.0f / 5.0f + 32.0f) : g_senTemp;

  // Partial window over full screen (same calls as legacy loop)
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // ---- Lines (exact coordinates from legacy) ----
    // First horizontal line
    display.drawLine(0, 102, 416, 102, GxEPD_BLACK);
    display.drawLine(0, 103, 416, 103, GxEPD_BLACK);

    // Vertical lines for Temp, RH, CO2, VOC
    display.drawLine(138, 0, 138, 102, GxEPD_BLACK);
    display.drawLine(139, 0, 139, 102, GxEPD_BLACK);

    display.drawLine(277, 0, 277, 102, GxEPD_BLACK);
    display.drawLine(278, 0, 278, 102, GxEPD_BLACK);

    // Horizontal lines for PM1.0, PM2.5, PM4.0, PM10
    display.drawLine(36, 148, 190, 148, GxEPD_BLACK);
    display.drawLine(226, 148, 380, 148, GxEPD_BLACK);
    display.drawLine(36, 194, 190, 194, GxEPD_BLACK);
    display.drawLine(226, 194, 380, 194, GxEPD_BLACK);

    // Bottom horizontal line
    display.drawLine(0, 215, 416, 215, GxEPD_BLACK);
    display.drawLine(0, 216, 416, 216, GxEPD_BLACK);

    display.setTextColor(GxEPD_BLACK);

    // ---- Temperature & Humidity (left two boxes) ----
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(20, 40);
    display.printf("%.1f %c", displayTemp, unitChar);

    display.setCursor(20, 90);
    display.printf("%.1f%%", g_senRH);

    // ---- CO2 (middle box) ----
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(147, 25);
    display.printf("CO2 (ppm)");
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(165, 80);
    display.printf("%d", g_co2);

    // ---- VOC (right box) ----
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(285, 25);
    display.printf("VOC Index");
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(310, 80);
    display.printf("%.0f", g_voc);

    // ---- Particulates grid ----
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(36, 146);
    display.printf("PM1:");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(105, 146);
    display.printf("%.1f", g_pm1);

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(226, 146);
    display.printf("PM2.5:");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(310, 146);
    display.printf("%.1f", g_pm25);

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(36, 192);
    display.printf("PM4:");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(105, 192);
    display.printf("%.1f", g_pm4);

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(226, 192);
    display.printf("PM10:");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(310, 192);
    display.printf("%.1f", g_pm10);

    // (Indicator triangle was commented out in legacy; left out intentionally.)
  } while (display.nextPage());
}

// --- Public API ---
// IMPORTANT: all temperature values passed here are in °C.
// We convert to °F for display if g_temp_unit_f == true.
inline void tick_and_draw(uint16_t co2_ppm,
                          float scd_temp_c, float scd_rh,
                          float sen_temp_c, float sen_rh,
                          float pm1, float pm25, float pm4, float pm10,
                          float voc_idx) {
  init_once();

  // cache latest values (in °C)
  g_co2 = co2_ppm;
  g_scdTemp = scd_temp_c;
  g_scdRH   = scd_rh;
  g_senTemp = sen_temp_c;
  g_senRH   = sen_rh;
  g_pm1  = pm1;
  g_pm25 = pm25;
  g_pm4  = pm4;
  g_pm10 = pm10;
  g_voc  = voc_idx;

  // draw exactly like legacy loop, but with configurable unit
  render_legacy_layout();
}

}  // namespace aeroq_display

}  // namespace aeroq

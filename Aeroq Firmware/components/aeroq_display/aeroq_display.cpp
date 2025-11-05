#include "aeroq_display.h"

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// ==== SPI + Display Pins (same as your sketch) ====
#define EPD_MOSI  7
#define EPD_SCK   6
#define EPD_CS    5
#define EPD_DC    4
#define EPD_RST   3
#define EPD_BUSY  1
#define EPD_MISO -1

// ==== Panel class (same as your sketch) ====
// GDEY037T03 416x240
GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT> display(
    GxEPD2_370_GDEY037T03(EPD_CS, EPD_DC, EPD_RST, -1));

namespace {
  bool inited = false;
  bool force_next = false;

  void init_once() {
    if (inited) return;

#if defined(ARDUINO_ARCH_ESP32)
    // Match your SPI remap
    SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI);
#endif

    display.init(115200);
    display.setRotation(1);

    // Your startup behavior: one initial full-window clear is OK at boot.
    display.setFullWindow();
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());

    display.setFont(&FreeSansBold18pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do {
      display.setCursor(120, 120);
      display.println("Initializing...");
    } while (display.nextPage());

    // Second clear exactly like your setup()
    display.setFullWindow();
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());

    inited = true;
  }

} // namespace

namespace aeroq_display {

void force_redraw() { force_next = true; }

void tick_and_draw(float co2, float pm2_5, float voc,
                   float avgTemp, float avgRH,
                   float pm25_alert_thresh) {
  init_once();

  // If the panel is busy, skip this tick (same safety as before)
  // Depending on wiring, BUSY may be LOW while busy; adjust if needed.
  if (digitalRead(EPD_BUSY) == LOW && !force_next) return;

  // === Partial refresh for the whole canvas, like your loop ===
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // ====== Your exact original layout (copied) ======
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

    // Temperature and Humidity (averaged, like your code)
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(20, 40);
    display.printf("%.1f C", avgTemp);

    display.setCursor(20, 90);
    display.printf("%.1f%%", avgRH);

    // CO2 and VOC
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(147, 25);
    display.printf("CO2 (ppm)");
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(165, 80);
    display.printf("%d", (int)roundf(co2));

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(285, 25);
    display.printf("VOC Index");
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(310, 80);
    display.printf("%.0f", voc);

    // PM metrics
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(36, 146);
    display.printf("PM1:");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(105, 146);
    // We'll reuse pm2_5 var for the other lines? No—draw each explicitly:
    // You can pass pm1, pm4, pm10 too if you want; for now we mirror your template usage.
    // If you want all four, add them to tick_and_draw signature and print like below.
    display.printf("--"); // replace with PM1.0 if you add it

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(226, 146);
    display.printf("PM2.5:");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(310, 146);
    display.printf("%.1f", pm2_5);

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(36, 192);
    display.printf("PM4:");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(105, 192);
    display.printf("--"); // replace with PM4.0 if you add it

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(226, 192);
    display.printf("PM10:");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(310, 192);
    display.printf("--"); // replace with PM10 if you add it

    // Optional alert indicator based on PM2.5 threshold
    if (!isnan(pm2_5) && pm2_5 > pm25_alert_thresh) {
      display.drawRect(280, 80, 120, 40, GxEPD_BLACK);
      display.setCursor(295, 106);
      display.print(F("ALERT"));
    }

    // ====== End original layout ======

  } while (display.nextPage());

  force_next = false;
}

} // namespace aeroq_display

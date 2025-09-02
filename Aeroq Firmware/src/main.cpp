#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SensirionI2CSen5x.h>
#include <SensirionI2cScd4x.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include "aws_iot.h"

// ==== SPI + Display Pin Configuration ====
#define EPD_MOSI  7
#define EPD_SCK   6
#define EPD_CS    5
#define EPD_DC    4
#define EPD_RST   3
#define EPD_BUSY  1

#define SDA 8
#define SCL 0

// ==== GDEY037T03 e-paper display (416x240) ====
GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT> display(
    GxEPD2_370_GDEY037T03(EPD_CS, EPD_DC, EPD_RST, -1));

// ==== Sensor Instances ====
SensirionI2CSen5x sen5x;
SensirionI2cScd4x scd41;

static char errorMessage[256];
static int16_t error;

#define NO_ERROR 0

void setup() {
    delay(1000);  // Delay to allow USB CDC setup
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 1000) {
        delay(10);
    }
    Serial.println("===== Booting ESP32-S3 Air Quality Monitor =====");

    Wire.begin(SDA, SCL);
    Serial.println("Wire (I2C) initialized");

    // SPI pin remap
    SPI.begin(EPD_SCK, -1, EPD_MOSI);
    Serial.println("SPI initialized");

    // Init e-paper display
    Serial.println("Initializing display...");
    display.init(115200);
    display.setRotation(1);

    // Fully clear the screen before anything else
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
    delay(1000);

    display.setFont(&FreeSansBold18pt7b);
    display.setTextColor(GxEPD_BLACK);
    // display.setFullWindow();
    display.firstPage();
    do {
        display.setCursor(120, 120);
        display.println("Initializing...");
    } while (display.nextPage());

    // Init SCD41
    Serial.println("Initializing SCD41...");
    scd41.begin(Wire, SCD41_I2C_ADDR_62);
    delay(30);
    error = scd41.wakeUp();
    Serial.printf("SCD41 wakeUp() -> %d\n", error);
    error = scd41.stopPeriodicMeasurement();
    Serial.printf("SCD41 stopPeriodicMeasurement() -> %d\n", error);
    error = scd41.reinit();
    Serial.printf("SCD41 reinit() -> %d\n", error);
    error = scd41.startPeriodicMeasurement();
    Serial.printf("SCD41 startPeriodicMeasurement() -> %d\n", error);

    // Init SEN54
    Serial.println("Initializing SEN54...");
    sen5x.begin(Wire);
    error = sen5x.deviceReset();
    Serial.printf("SEN54 deviceReset() -> %d\n", error);
    error = sen5x.setTemperatureOffsetSimple(0.0);
    Serial.printf("SEN54 setTemperatureOffsetSimple() -> %d\n", error);
    error = sen5x.startMeasurement();
    Serial.printf("SEN54 startMeasurement() -> %d\n", error);

    Serial.println("Setup complete.\n");
    aws_iot_init();
    delay(5000);

    // Clear the display again to ensure no old text remains
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
}

void loop() {
    // 5s update cycle
    for (int i=0; i<40; ++i) {
        aws_iot_loop();
        delay(100);
    }

    Serial.println("========== SENSOR UPDATE ==========");

    // ==== Read SCD41 ====
    Serial.println("[SCD41] Reading data...");
    bool dataReady = false;
    error = scd41.getDataReadyStatus(dataReady);
    if (error != NO_ERROR || !dataReady) {
        Serial.printf("[SCD41] getDataReadyStatus() error = %d\n", error);
        return;
    }

    aws_iot_loop();

    uint16_t co2 = 0;
    float scdTemp = 0.0, scdRH = 0.0;
    error = scd41.readMeasurement(co2, scdTemp, scdRH);
    if (error != NO_ERROR) {
        Serial.printf("[SCD41] readMeasurement() error = %d\n", error);
        return;
    }

    aws_iot_loop();

    Serial.printf("[SCD41] CO2: %d ppm\n", co2);
    Serial.printf("[SCD41] Temp: %.2f °C\n", scdTemp);
    Serial.printf("[SCD41] RH: %.2f %%\n", scdRH);

    // ==== Read SEN54 ====
    Serial.println("\n[SEN54] Reading data...");
    float pm1, pm2_5, pm4, pm10, senRH, senTemp, voc, nox;
    error = sen5x.readMeasuredValues(pm1, pm2_5, pm4, pm10, senRH, senTemp, voc, nox);
    if (error != NO_ERROR) {
        Serial.printf("[SEN54] readMeasuredValues() error = %d\n", error);
        return;
    }

    aws_iot_loop();

    Serial.printf("[SEN54] Temp: %.2f °C, RH: %.2f %%\n", senTemp, senRH);
    Serial.printf("[SEN54] PM1.0: %.2f, PM2.5: %.2f, PM4.0: %.2f, PM10: %.2f ug/m3\n", pm1, pm2_5, pm4, pm10);
    Serial.printf("[SEN54] VOC Index: %.2f, NOx Index: %.2f\n", voc, nox);

    // ==== Averaged Values ====
    float avgTemp = (senTemp + scdTemp) / 2.0;
    float avgRH = (senRH + scdRH) / 2.0;

    // ==== Update Display ====
    Serial.println("\n[Display] Updating e-paper...");
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    do {
        aws_iot_loop();
        display.fillScreen(GxEPD_WHITE);
        
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

        // Temperature and Humidity Display
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(20, 40);
        display.printf("%.1f C", avgTemp);

        display.setCursor(20, 90);
        display.printf("%.1f%%", avgRH);

        //Co2 and VOC Display
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(147, 25);
        display.printf("CO2 (ppm)");
        display.setFont(&FreeSansBold24pt7b);
        display.setCursor(165, 80);
        display.printf("%d", co2);
        
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(285, 25);
        display.printf("VOC Index");
        display.setFont(&FreeSansBold24pt7b);
        display.setCursor(310, 80);
        display.printf("%.0f", voc);

        // Particulate Matter Display
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(36, 146);
        display.printf("PM1:", pm1);
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(105, 146);
        display.printf("%.1f", pm1);

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(226, 146);
        display.printf("PM2.5:", pm2_5);
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(310, 146);
        display.printf("%.1f", pm2_5);

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(36, 192);
        display.printf("PM4:", pm4);
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(105, 192);
        display.printf("%.1f", pm4);

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(226, 192);
        display.printf("PM10:", pm10);
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(310, 192);
        display.printf("%.1f", pm10);

        //drawAirQualityIndicator(co2, voc, pm1, pm2_5, pm4, pm10);

    } while (display.nextPage());

    Serial.println("[Display] Refresh complete.\n");

    aws_iot_loop();  // keep the connection healthy

    Telemetry t {
    .device_id = "esp32c3-01",
    .co2       = co2,
    .pm25      = pm2_5,
    .pm10      = pm10,
    .voc_index = voc,
    .temp_c    = avgTemp,
    .rh        = avgRH
    };

    bool ok = aws_iot_publish_telemetry(t);
    Serial.printf("[AWS IoT] publish %s\n", ok ? "OK" : "FAIL");
}

// void drawAirQualityIndicator(uint16_t co2, float voc, float pm1, float pm2_5, float pm4, float pm10) {
//     bool isPoor = co2 > 1400 || voc > 250 || pm2_5 > 35 || pm10 > 50;
//     bool isModerate = co2 > 800 || voc > 120 || pm2_5 > 12 || pm10 > 20;

//     int16_t baseY = display.height() - 5;
//     int16_t triHeight = 10;
//     int16_t triWidth = 12;
//     int16_t halfWidth = triWidth / 2;

//     int16_t x;
//     if (isPoor) {
//         x = display.width() - 30;
//     } else if (isModerate) {
//         x = display.width() / 2;
//     } else {
//         x = 30;
//     }

//     display.fillTriangle(
//         x, baseY - triHeight,
//         x - halfWidth, baseY,
//         x + halfWidth, baseY,
//         GxEPD_BLACK);
// }

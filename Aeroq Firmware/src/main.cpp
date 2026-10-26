#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include <SensirionI2CSen5x.h>
#include <SensirionI2cScd4x.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

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
    GxEPD2_370_GDEY037T03(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// ==== Sensor Instances ====
SensirionI2CSen5x sen5x;
SensirionI2cScd4x scd4x;

#define NO_ERROR 0

// Full refresh every ~24 hours (17280 * 5s) to clear partial-refresh ghosting
#define FULL_REFRESH_INTERVAL 17280

// ==== Latest Readings (NAN until a valid sample arrives) ====
static float co2 = NAN, scdTemp = NAN, scdRH = NAN;
static float pm1 = NAN, pm2_5 = NAN, pm4 = NAN, pm10 = NAN;
static float senTemp = NAN, senRH = NAN, voc = NAN, nox = NAN;
static bool scdFailed = false;
static bool senFailed = false;
static uint32_t refreshCount = 0;

void setup() {
    delay(1000);  // Delay to allow USB CDC setup
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 1000) {
        delay(10);
    }
    Serial.println("===== Booting ESP32-C3 Air Quality Monitor =====");

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

    // Init SCD4x
    Serial.println("Initializing SCD4x...");
    scd4x.begin(Wire, SCD41_I2C_ADDR_62);
    delay(30);
    int16_t error = scd4x.wakeUp();
    Serial.printf("SCD4x wakeUp() -> %d\n", error);
    error = scd4x.stopPeriodicMeasurement();
    Serial.printf("SCD4x stopPeriodicMeasurement() -> %d\n", error);
    error = scd4x.reinit();
    Serial.printf("SCD4x reinit() -> %d\n", error);
    error = scd4x.startPeriodicMeasurement();
    Serial.printf("SCD4x startPeriodicMeasurement() -> %d\n", error);

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
    delay(5000);

    // Clear the display again to ensure no old text remains
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
}

void readScd4x() {
    Serial.println("[SCD4x] Reading data...");
    bool dataReady = false;
    int16_t error = scd4x.getDataReadyStatus(dataReady);
    if (error != NO_ERROR) {
        Serial.printf("[SCD4x] getDataReadyStatus() error = %d\n", error);
        scdFailed = true;
        co2 = scdTemp = scdRH = NAN;
        return;
    }
    if (!dataReady) {
        // A new sample arrives every 5s; keep showing the previous one until then
        Serial.println("[SCD4x] No new data yet");
        return;
    }

    uint16_t co2ppm = 0;
    error = scd4x.readMeasurement(co2ppm, scdTemp, scdRH);
    if (error != NO_ERROR) {
        Serial.printf("[SCD4x] readMeasurement() error = %d\n", error);
        scdFailed = true;
        co2 = scdTemp = scdRH = NAN;
        return;
    }
    scdFailed = false;
    co2 = co2ppm;

    Serial.printf("[SCD4x] CO2: %.0f ppm\n", co2);
    Serial.printf("[SCD4x] Temp: %.2f °C\n", scdTemp);
    Serial.printf("[SCD4x] RH: %.2f %%\n", scdRH);
}

void readSen54() {
    Serial.println("\n[SEN54] Reading data...");
    int16_t error = sen5x.readMeasuredValues(pm1, pm2_5, pm4, pm10, senRH, senTemp, voc, nox);
    if (error != NO_ERROR) {
        // 270 means the SEN54 isn't answering on I2C: check its cable and 5V supply
        Serial.printf("[SEN54] readMeasuredValues() error = %d\n", error);
        senFailed = true;
        pm1 = pm2_5 = pm4 = pm10 = senRH = senTemp = voc = nox = NAN;
        return;
    }
    senFailed = false;

    Serial.printf("[SEN54] Temp: %.2f °C, RH: %.2f %%\n", senTemp, senRH);
    Serial.printf("[SEN54] PM1.0: %.2f, PM2.5: %.2f, PM4.0: %.2f, PM10: %.2f ug/m3\n", pm1, pm2_5, pm4, pm10);
    Serial.printf("[SEN54] VOC Index: %.2f, NOx Index: %.2f\n", voc, nox);
}

// Average of the available readings, or NAN if neither sensor has one
float average(float a, float b) {
    if (isnan(a)) return b;
    if (isnan(b)) return a;
    return (a + b) / 2.0;
}

// Prints a reading, or "--" when it isn't available
void printReading(const char* format, float value) {
    if (isnan(value)) {
        display.print("--");
    } else {
        display.printf(format, value);
    }
}

void loop() {
    delay(5000); // 5s update cycle
    Serial.println("========== SENSOR UPDATE ==========");

    readScd4x();
    readSen54();

    // ==== Averaged Values ====
    float avgTemp = average(senTemp, scdTemp);
    float avgRH = average(senRH, scdRH);

    const char* status = nullptr;
    if (senFailed) {
        status = "SEN54 not responding - check cable/5V";
    } else if (scdFailed) {
        status = "CO2 sensor not responding";
    }

    // ==== Update Display ====
    Serial.println("\n[Display] Updating e-paper...");
    bool fullRefresh = (++refreshCount % FULL_REFRESH_INTERVAL == 0);
    if (fullRefresh) {
        display.setFullWindow();
    } else {
        display.setPartialWindow(0, 0, display.width(), display.height());
    }
    display.firstPage();
    do {
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
        printReading("%.1f C", avgTemp);

        display.setCursor(20, 90);
        printReading("%.1f%%", avgRH);

        //Co2 and VOC Display
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(147, 25);
        display.print("CO2 (ppm)");
        display.setFont(&FreeSansBold24pt7b);
        display.setCursor(165, 80);
        printReading("%.0f", co2);

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(285, 25);
        display.print("VOC Index");
        display.setFont(&FreeSansBold24pt7b);
        display.setCursor(310, 80);
        printReading("%.0f", voc);

        // Particulate Matter Display
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(36, 146);
        display.print("PM1:");
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(105, 146);
        printReading("%.1f", pm1);

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(226, 146);
        display.print("PM2.5:");
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(310, 146);
        printReading("%.1f", pm2_5);

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(36, 192);
        display.print("PM4:");
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(105, 192);
        printReading("%.1f", pm4);

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(226, 192);
        display.print("PM10:");
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(310, 192);
        printReading("%.1f", pm10);

        // Sensor status in the bottom strip
        if (status) {
            display.setFont(&FreeSansBold9pt7b);
            display.setCursor(10, 234);
            display.print(status);
        }

        //drawAirQualityIndicator(co2, voc, pm1, pm2_5, pm4, pm10);

    } while (display.nextPage());

    Serial.println("[Display] Refresh complete.\n");
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

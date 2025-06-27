// #include <Arduino.h>
// #include <Wire.h>
// #include <SensirionI2CSen5x.h>
// #include <SensirionI2cScd4x.h>

// // ========== Globals and Instances ==========
// SensirionI2CSen5x sen5x;
// SensirionI2cScd4x scd41;

// static char errorMessage[256];
// static int16_t error;

// #define NO_ERROR 0

// // ========== Setup ==========
// void setup() {
//     Serial.begin(115200);
//     while (!Serial) delay(100);

//     Wire.begin(SDA, SCL);

//     // --- SCD41 Initialization ---
//     scd41.begin(Wire, SCD41_I2C_ADDR_62);
//     delay(30);
//     error = scd41.wakeUp();
//     error = scd41.stopPeriodicMeasurement();
//     error = scd41.reinit();
//     uint64_t serialNumber = 0;
//     error = scd41.getSerialNumber(serialNumber);
//     if (error == NO_ERROR) {
//         Serial.print("SCD41 Serial Number: 0x");
//         Serial.print((uint32_t)(serialNumber >> 32), HEX);
//         Serial.println((uint32_t)(serialNumber & 0xFFFFFFFF), HEX);
//     }
//     error = scd41.startPeriodicMeasurement();

//     // --- SEN54 Initialization ---
//     sen5x.begin(Wire);
//     error = sen5x.deviceReset();
//     float tempOffset = 0.0;
//     error = sen5x.setTemperatureOffsetSimple(tempOffset);
//     error = sen5x.startMeasurement();
// }

// // ========== Loop ==========
// void loop() {
//     delay(5000); // Match SCD41 sampling rate

//     // --- Read SCD41 ---
//     bool dataReady = false;
//     error = scd41.getDataReadyStatus(dataReady);
//     if (error != NO_ERROR || !dataReady) return;

//     uint16_t co2Concentration = 0;
//     float scdTemp = 0.0, scdHumidity = 0.0;
//     error = scd41.readMeasurement(co2Concentration, scdTemp, scdHumidity);
//     if (error != NO_ERROR) return;

//     // --- Read SEN54 ---
//     float pm1p0, pm2p5, pm4p0, pm10p0, senHumidity, senTemp, vocIndex, noxIndex;
//     error = sen5x.readMeasuredValues(pm1p0, pm2p5, pm4p0, pm10p0,
//                                      senHumidity, senTemp, vocIndex, noxIndex);

//     // --- Print Output ---
//     Serial.println("===== Air Quality Readings =====");

//     Serial.print("CO2 [ppm]: "); Serial.println(co2Concentration);
//     Serial.print("Temp (SCD41) [°C]: "); Serial.println(scdTemp);
//     Serial.print("RH (SCD41) [%]: "); Serial.println(scdHumidity);

//     Serial.print("PM1.0 [µg/m³]: "); Serial.println(pm1p0);
//     Serial.print("PM2.5 [µg/m³]: "); Serial.println(pm2p5);
//     Serial.print("PM4.0 [µg/m³]: "); Serial.println(pm4p0);
//     Serial.print("PM10.0 [µg/m³]: "); Serial.println(pm10p0);

//     Serial.print("Temp (SEN54) [°C]: ");
//     Serial.println(isnan(senTemp) ? "n/a" : String(senTemp));

//     Serial.print("RH (SEN54) [%]: ");
//     Serial.println(isnan(senHumidity) ? "n/a" : String(senHumidity));

//     Serial.print("VOC Index: ");
//     Serial.println(isnan(vocIndex) ? "n/a" : String(vocIndex));

//     Serial.print("NOx Index: ");
//     Serial.println(isnan(noxIndex) ? "n/a" : String(noxIndex));

//     Serial.println("================================");
// }


#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SensirionI2CSen5x.h>
#include <SensirionI2cScd4x.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// ==== SPI + Display Pin Configuration ====
#define EPD_MOSI  11
#define EPD_SCK   12
#define EPD_CS    5
#define EPD_DC    17
#define EPD_RST   16
#define EPD_BUSY  4

// ==== GDEY037T03 e-paper display (416x240) ====
GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT> display(
    GxEPD2_370_GDEY037T03(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// ==== Sensor Instances ====
SensirionI2CSen5x sen5x;
SensirionI2cScd4x scd41;

static char errorMessage[256];
static int16_t error;

#define NO_ERROR 0

void setup() {
    delay(1000);  // Delay to allow USB CDC setup
    Serial.begin(115200);
    while (!Serial) delay(10);
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
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
    display.firstPage();
    do {
        display.setCursor(10, 40);
        display.println("Booting sensors...");
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
}

void loop() {
    delay(10000); // 10s update cycle
    Serial.println("========== SENSOR UPDATE ==========");

    // ==== Read SCD41 ====
    Serial.println("[SCD41] Reading data...");
    bool dataReady = false;
    error = scd41.getDataReadyStatus(dataReady);
    if (error != NO_ERROR || !dataReady) {
        Serial.printf("[SCD41] getDataReadyStatus() error = %d\n", error);
        return;
    }

    uint16_t co2 = 0;
    float scdTemp = 0.0, scdRH = 0.0;
    error = scd41.readMeasurement(co2, scdTemp, scdRH);
    if (error != NO_ERROR) {
        Serial.printf("[SCD41] readMeasurement() error = %d\n", error);
        return;
    }

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
        display.fillScreen(GxEPD_WHITE);

        display.setCursor(10, 30);
        display.printf("CO2: %d ppm", co2);

        display.setCursor(10, 55);
        display.printf("PM1.0: %.1f", pm1);

        display.setCursor(10, 80);
        display.printf("PM2.5: %.1f", pm2_5);

        display.setCursor(10, 105);
        display.printf("PM4.0: %.1f", pm4);

        display.setCursor(10, 130);
        display.printf("PM10: %.1f", pm10);

        display.setCursor(200, 30);
        display.printf("Temp: %.1f C", avgTemp);

        display.setCursor(200, 55);
        display.printf("RH: %.1f %%", avgRH);

        display.setCursor(200, 80);
        display.printf("VOC: %.1f", voc);

        display.setCursor(200, 105);
        display.printf("NOx: %.1f", nox);

    } while (display.nextPage());

    Serial.println("[Display] Refresh complete.\n");
}
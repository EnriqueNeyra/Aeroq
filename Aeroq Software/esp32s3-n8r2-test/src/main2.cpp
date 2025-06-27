// #include <Arduino.h>
// #include <SensirionI2cScd4x.h>
// #include <Wire.h>

// // macro definitions
// // make sure that we use the proper definition of NO_ERROR
// #ifdef NO_ERROR
// #undef NO_ERROR
// #endif
// #define NO_ERROR 0

// SensirionI2cScd4x sensor;

// static char errorMessage[64];
// static int16_t error;

// void PrintUint64(uint64_t& value) {
//     Serial.print("0x");
//     Serial.print((uint32_t)(value >> 32), HEX);
//     Serial.print((uint32_t)(value & 0xFFFFFFFF), HEX);
// }

// void setup() {

//     Serial.begin(115200);
//     while (!Serial) {
//         delay(100);
//     }
//     Wire.begin(SDA, SCL);
//     sensor.begin(Wire, SCD41_I2C_ADDR_62);

//     uint64_t serialNumber = 0;
//     delay(30);
//     // Ensure sensor is in clean state
//     error = sensor.wakeUp();
//     if (error != NO_ERROR) {
//         Serial.print("Error trying to execute wakeUp(): ");
//         errorToString(error, errorMessage, sizeof errorMessage);
//         Serial.println(errorMessage);
//     }
//     error = sensor.stopPeriodicMeasurement();
//     if (error != NO_ERROR) {
//         Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
//         errorToString(error, errorMessage, sizeof errorMessage);
//         Serial.println(errorMessage);
//     }
//     error = sensor.reinit();
//     if (error != NO_ERROR) {
//         Serial.print("Error trying to execute reinit(): ");
//         errorToString(error, errorMessage, sizeof errorMessage);
//         Serial.println(errorMessage);
//     }
//     // Read out information about the sensor
//     error = sensor.getSerialNumber(serialNumber);
//     if (error != NO_ERROR) {
//         Serial.print("Error trying to execute getSerialNumber(): ");
//         errorToString(error, errorMessage, sizeof errorMessage);
//         Serial.println(errorMessage);
//         return;
//     }
//     Serial.print("serial number: ");
//     PrintUint64(serialNumber);
//     Serial.println();
//     //
//     // If temperature offset and/or sensor altitude compensation
//     // is required, you should call the respective functions here.
//     // Check out the header file for the function definitions.
//     // Start periodic measurements (5sec interval)
//     error = sensor.startPeriodicMeasurement();
//     if (error != NO_ERROR) {
//         Serial.print("Error trying to execute startPeriodicMeasurement(): ");
//         errorToString(error, errorMessage, sizeof errorMessage);
//         Serial.println(errorMessage);
//         return;
//     }
//     //
//     // If low-power mode is required, switch to the low power
//     // measurement function instead of the standard measurement
//     // function above. Check out the header file for the definition.
//     // For SCD41, you can also check out the single shot measurement example.
//     //
// }

// void loop() {

//     bool dataReady = false;
//     uint16_t co2Concentration = 0;
//     float temperature = 0.0;
//     float relativeHumidity = 0.0;
//     //
//     // Slow down the sampling to 0.2Hz.
//     //
//     delay(5000);
//     error = sensor.getDataReadyStatus(dataReady);
//     if (error != NO_ERROR) {
//         Serial.print("Error trying to execute getDataReadyStatus(): ");
//         errorToString(error, errorMessage, sizeof errorMessage);
//         Serial.println(errorMessage);
//         return;
//     }
//     while (!dataReady) {
//         delay(100);
//         error = sensor.getDataReadyStatus(dataReady);
//         if (error != NO_ERROR) {
//             Serial.print("Error trying to execute getDataReadyStatus(): ");
//             errorToString(error, errorMessage, sizeof errorMessage);
//             Serial.println(errorMessage);
//             return;
//         }
//     }
//     //
//     // If ambient pressure compenstation during measurement
//     // is required, you should call the respective functions here.
//     // Check out the header file for the function definition.
//     error =
//         sensor.readMeasurement(co2Concentration, temperature, relativeHumidity);
//     if (error != NO_ERROR) {
//         Serial.print("Error trying to execute readMeasurement(): ");
//         errorToString(error, errorMessage, sizeof errorMessage);
//         Serial.println(errorMessage);
//         return;
//     }
//     //
//     // Print results in physical units.
//     Serial.print("CO2 concentration [ppm]: ");
//     Serial.print(co2Concentration);
//     Serial.println();
//     Serial.print("Temperature [°C]: ");
//     Serial.print(temperature);
//     Serial.println();
//     Serial.print("Relative Humidity [RH]: ");
//     Serial.print(relativeHumidity);
//     Serial.println();
// }

// #include <Arduino.h>
// #include <SensirionI2CSen5x.h>
// #include <Wire.h>

// // The used commands use up to 48 bytes. On some Arduino's the default buffer
// // space is not large enough
// #define MAXBUF_REQUIREMENT 48

// #if (defined(I2C_BUFFER_LENGTH) &&                 \
//      (I2C_BUFFER_LENGTH >= MAXBUF_REQUIREMENT)) || \
//     (defined(BUFFER_LENGTH) && BUFFER_LENGTH >= MAXBUF_REQUIREMENT)
// #define USE_PRODUCT_INFO
// #endif

// SensirionI2CSen5x sen5x;

// void printModuleVersions() {
//     uint16_t error;
//     char errorMessage[256];

//     unsigned char productName[32];
//     uint8_t productNameSize = 32;

//     error = sen5x.getProductName(productName, productNameSize);

//     if (error) {
//         Serial.print("Error trying to execute getProductName(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     } else {
//         Serial.print("ProductName:");
//         Serial.println((char*)productName);
//     }

//     uint8_t firmwareMajor;
//     uint8_t firmwareMinor;
//     bool firmwareDebug;
//     uint8_t hardwareMajor;
//     uint8_t hardwareMinor;
//     uint8_t protocolMajor;
//     uint8_t protocolMinor;

//     error = sen5x.getVersion(firmwareMajor, firmwareMinor, firmwareDebug,
//                              hardwareMajor, hardwareMinor, protocolMajor,
//                              protocolMinor);
//     if (error) {
//         Serial.print("Error trying to execute getVersion(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     } else {
//         Serial.print("Firmware: ");
//         Serial.print(firmwareMajor);
//         Serial.print(".");
//         Serial.print(firmwareMinor);
//         Serial.print(", ");

//         Serial.print("Hardware: ");
//         Serial.print(hardwareMajor);
//         Serial.print(".");
//         Serial.println(hardwareMinor);
//     }
// }

// void printSerialNumber() {
//     uint16_t error;
//     char errorMessage[256];
//     unsigned char serialNumber[32];
//     uint8_t serialNumberSize = 32;

//     error = sen5x.getSerialNumber(serialNumber, serialNumberSize);
//     if (error) {
//         Serial.print("Error trying to execute getSerialNumber(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     } else {
//         Serial.print("SerialNumber:");
//         Serial.println((char*)serialNumber);
//     }
// }

// void setup() {

//     Serial.begin(115200);
//     while (!Serial) {
//         delay(100);
//     }

//     Wire.begin(SDA, SCL);

//     sen5x.begin(Wire);

//     uint16_t error;
//     char errorMessage[256];
//     error = sen5x.deviceReset();
//     if (error) {
//         Serial.print("Error trying to execute deviceReset(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     }

// // Print SEN55 module information if i2c buffers are large enough
// #ifdef USE_PRODUCT_INFO
//     printSerialNumber();
//     printModuleVersions();
// #endif

//     // set a temperature offset in degrees celsius
//     // Note: supported by SEN54 and SEN55 sensors
//     // By default, the temperature and humidity outputs from the sensor
//     // are compensated for the modules self-heating. If the module is
//     // designed into a device, the temperature compensation might need
//     // to be adapted to incorporate the change in thermal coupling and
//     // self-heating of other device components.
//     //
//     // A guide to achieve optimal performance, including references
//     // to mechanical design-in examples can be found in the app note
//     // “SEN5x – Temperature Compensation Instruction” at www.sensirion.com.
//     // Please refer to those application notes for further information
//     // on the advanced compensation settings used
//     // in `setTemperatureOffsetParameters`, `setWarmStartParameter` and
//     // `setRhtAccelerationMode`.
//     //
//     // Adjust tempOffset to account for additional temperature offsets
//     // exceeding the SEN module's self heating.
//     float tempOffset = 0.0;
//     error = sen5x.setTemperatureOffsetSimple(tempOffset);
//     if (error) {
//         Serial.print("Error trying to execute setTemperatureOffsetSimple(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     } else {
//         Serial.print("Temperature Offset set to ");
//         Serial.print(tempOffset);
//         Serial.println(" deg. Celsius (SEN54/SEN55 only");
//     }

//     // Start Measurement
//     error = sen5x.startMeasurement();
//     if (error) {
//         Serial.print("Error trying to execute startMeasurement(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     }
// }

// void loop() {
//     uint16_t error;
//     char errorMessage[256];

//     delay(1000);

//     // Read Measurement
//     float massConcentrationPm1p0;
//     float massConcentrationPm2p5;
//     float massConcentrationPm4p0;
//     float massConcentrationPm10p0;
//     float ambientHumidity;
//     float ambientTemperature;
//     float vocIndex;
//     float noxIndex;

//     error = sen5x.readMeasuredValues(
//         massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
//         massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
//         noxIndex);

//     if (error) {
//         Serial.print("Error trying to execute readMeasuredValues(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     } else {
//         Serial.print("MassConcentrationPm1p0:");
//         Serial.print(massConcentrationPm1p0);
//         Serial.print("\t");
//         Serial.print("MassConcentrationPm2p5:");
//         Serial.print(massConcentrationPm2p5);
//         Serial.print("\t");
//         Serial.print("MassConcentrationPm4p0:");
//         Serial.print(massConcentrationPm4p0);
//         Serial.print("\t");
//         Serial.print("MassConcentrationPm10p0:");
//         Serial.print(massConcentrationPm10p0);
//         Serial.print("\t");
//         Serial.print("AmbientHumidity:");
//         if (isnan(ambientHumidity)) {
//             Serial.print("n/a");
//         } else {
//             Serial.print(ambientHumidity);
//         }
//         Serial.print("\t");
//         Serial.print("AmbientTemperature:");
//         if (isnan(ambientTemperature)) {
//             Serial.print("n/a");
//         } else {
//             Serial.print(ambientTemperature);
//         }
//         Serial.print("\t");
//         Serial.print("VocIndex:");
//         if (isnan(vocIndex)) {
//             Serial.print("n/a");
//         } else {
//             Serial.print(vocIndex);
//         }
//         Serial.print("\t");
//         Serial.print("NoxIndex:");
//         if (isnan(noxIndex)) {
//             Serial.println("n/a");
//         } else {
//             Serial.println(noxIndex);
//         }
//     }
// }
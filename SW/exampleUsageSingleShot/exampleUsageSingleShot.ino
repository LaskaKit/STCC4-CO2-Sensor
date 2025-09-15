/* Test code Single Shot for LaskaKit STCC4 CO2 Sensor
 * example from Sernsirion STCC4 library is used
 *
 * Board:   LaskaKit ESP32-S3-DEVKit        https://www.laskakit.cz/laskakit-esp32-s3-devkit
 * Display: LaskaKit STCC4 CO2 Sensor       https://www.laskakit.cz/laskakit-stcc4-senzor-co2--teploty-a-vlhkosti-vzduchu/
 *
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
 */

// Requires Library https://github.com/Sensirion/arduino-i2c-stcc4

#include <Arduino.h>
#include <SensirionI2cStcc4.h>
#include <Wire.h>

// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

#define POWER 47
#define SDA   42
#define SCL   2

SensirionI2cStcc4 sensor;

static char errorMessage[64];
static int16_t error;

void setup() {

    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

        pinMode(POWER, OUTPUT);
    digitalWrite(POWER, HIGH);   // turn the Display on (HIGH is the voltage level)
    Serial.println("Display power ON");
    delay(500);   

    Wire.begin(SDA, SCL);
    Wire.begin();
    sensor.begin(Wire, STCC4_I2C_ADDR_64);

    delay(6);
    // Ensure sensor is in idle state
    error = sensor.exitSleepMode();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute exitSleepMode(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    error = sensor.stopContinuousMeasurement();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute stopContinuousMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    // Enter sleep mode
    error = sensor.enterSleepMode();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute enterSleepMode(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
}

void loop() {

    int16_t co2Concentration = 0;
    float temperature = 0.0;
    float relativeHumidity = 0.0;
    uint16_t status = 0;
    //
    // Measure every 10 seconds.
    delay(10000);
    //
    // Exit sleep mode to put the sensor into idle mode
    // to be able to perform a single shot measurement
    error = sensor.exitSleepMode();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute exitSleepMode(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    //
    // If humidity/temperature and/or pressure compensation is
    // desired, you should call the respective compensation
    // functions here. Check-out the header file for the definition
    // of the compensation functions.
    //
    // Perform a single shot measurement and read the sensor data
    error = sensor.measureSingleShot();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute measureSingleShot(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    error = sensor.readMeasurement(co2Concentration, temperature,
                                   relativeHumidity, status);
    if (error != NO_ERROR) {
        // A failed read can be caused by clock shifting. We advise to retry
        // after a delay of 150ms.
        Serial.print(
            "Error trying to execute readMeasurement() (retry in 150ms): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        delay(150);
        error = sensor.readMeasurement(co2Concentration, temperature,
                                       relativeHumidity, status);
        if (error != NO_ERROR) {
            Serial.print("Error trying to execute readMeasurement() after "
                         "additional delay: ");
            errorToString(error, errorMessage, sizeof errorMessage);
            Serial.println(errorMessage);
            return;
        }
    }
    //
    // Power down the sensor to reduce power consumption.
    error = sensor.enterSleepMode();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute enterSleepMode(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    //
    // Print results as physical unit.
    Serial.print("CO2 concentration [ppm] = ");
    Serial.print(co2Concentration);
    Serial.println();
    Serial.print("Temperature [°C] = ");
    Serial.print(temperature);
    Serial.println();
    Serial.print("Humidity [RH] = ");
    Serial.print(relativeHumidity);
    Serial.println();
    Serial.print("Status = ");
    Serial.print(status);
    Serial.println();
}

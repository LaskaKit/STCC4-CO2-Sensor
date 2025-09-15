/* Test code for LaskaKit STCC4 CO2 Sensor
 * example from Sernsirion STCC4 library is used
 *
 * Board:   LaskaKit ESP32-S3-DEVKit        https://www.laskakit.cz/laskakit-esp32-s3-devkit
 * Display: LaskaKit STCC4 CO2 Sensor       
 *
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
 */

// Requires Library https://github.com/Sensirion/arduino-i2c-stcc4

#include <Arduino.h>
#include <SensirionI2cStcc4.h>
#include <Wire.h>

#define POWER 47
#define SDA   42
#define SCL   2

// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionI2cStcc4 sensor;

static char errorMessage[64];
static int16_t error;

void PrintUint64(uint64_t& value) {
    Serial.print("0x");
    Serial.print((uint32_t)(value >> 32), HEX);
    Serial.print((uint32_t)(value & 0xFFFFFFFF), HEX);
}

void setup() {

    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    pinMode(POWER, OUTPUT);
    digitalWrite(POWER, HIGH);   // turn the Power ON
    Serial.println("Power ON");
    delay(500);   

    Wire.begin(SDA, SCL);
    sensor.begin(Wire, 0x64);

    uint32_t productId = 0;
    uint64_t serialNumber = 0;
    error = sensor.stopContinuousMeasurement();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute stopContinuousMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    error = sensor.getProductId(productId, serialNumber);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute getProductId(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    Serial.print("productId: ");
    Serial.print(productId);
    Serial.print("\t");
    Serial.print("serialNumber: ");
    PrintUint64(serialNumber);
    Serial.println();
    error = sensor.startContinuousMeasurement();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute startContinuousMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
}

// helper: z sensorStatus alespoň detekujme "SHT není připojen"
static bool rhtWarned = false;

void loop() {
    int16_t co2 = 0;
    float t = 0.0f;
    float rh = 0.0f;
    uint16_t status = 0;

    delay(1000);

    error = sensor.readMeasurement(co2, t, rh, status);
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.print("readMeasurement error (retry 150ms): ");
        Serial.println(errorMessage);
        delay(150);
        error = sensor.readMeasurement(co2, t, rh, status);
        if (error != NO_ERROR) {
            errorToString(error, errorMessage, sizeof errorMessage);
            Serial.print("readMeasurement error (2nd try): ");
            Serial.println(errorMessage);
            return;
        }
    }

    // Pozn: dle datasheetu read_measurement vrací T a RH "as received" ze SHT4x.
    // Pokud STCC4 SHT4x nevidí, používá default 25°C / 50%RH.
    bool looksDefaultRHT = (fabsf(t - 25.0f) < 0.01f) && (fabsf(rh - 50.0f) < 0.01f);

    // Některé bity statusu indikují stavy; minimálně zvedneme varování, pokud RHT vypadá defaultně
    if (looksDefaultRHT && !rhtWarned) {
        Serial.println(F("[WARN] SHT40 neni detekovan na SDA_C/SCL_C (STCC4 jede na 25C/50%RH)."));
        Serial.println(F("       Zkontroluj VDDIO, zapojeni SDA_C/SCL_C a adresu 0x44."));
        rhtWarned = true;
    }

    Serial.print("CO2 [ppm]: "); Serial.print(co2);
    Serial.print("\tT [C]: "); Serial.print(t, 2);
    Serial.print("\tRH [%]: "); Serial.print(rh, 2);
    Serial.print("\tStatus: 0x"); Serial.println(status, HEX);
}

/* Test code Single Shot for LaskaKit STCC4 CO2 Sensor
 * example from Sernsirion STCC4 library is used
 *
 * Board:   LaskaKit ESP32-S3-DEVKit        https://www.laskakit.cz/laskakit-esp32-s3-devkit
 * Sensor: LaskaKit STCC4 CO2 Sensor       https://www.laskakit.cz/laskakit-stcc4-senzor-co2--teploty-a-vlhkosti-vzduchu/
 *
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
 *
 * Email:   podpora@laskakit.cz
 * Web:     laskakit.cz
 *
 * Requires: https://github.com/Sensirion/arduino-i2c-stcc4
 */

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cStcc4.h>

// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

// --- HW pins (ESP32-S3 DEVKIT) ---
#define POWER 47
#define SDA   42
#define SCL    2

// --- Settings ---
#define I2C_HZ             100000   // 100 kHz is safe for STCC4
#define I2C_TIMEOUT_MS          100 // ESP32 Wire timeout
#define MEAS_INTERVAL_MS      10000 // single-shot every 10 s
#define POLL_STEP_MS             50 // how often to poll for ready data
#define POLL_TIMEOUT_MS        1200 // max wait for single-shot result
#define WARMUP_CONT_MS        30000 // 30 s continuous warm-up after boot

SensirionI2cStcc4 sensor;

static char errorMessage[64];
static int16_t error;

// optional one-time warm-up to get rid of the 390 ppm start value faster
static void warmup_continuous_mode() {
  // leave sleep (if any) and start continuous for a short stabilization
  (void)sensor.exitSleepMode();
  if (sensor.startContinuousMeasurement() == NO_ERROR) {
    Serial.println(F("Warm-up: continuous measurement for 30 s..."));
    delay(WARMUP_CONT_MS);
    (void)sensor.stopContinuousMeasurement();
  } else {
    Serial.println(F("Warm-up skipped (could not start continuous)."));
  }
  (void)sensor.enterSleepMode();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);
  Serial.println(F("Power ON"));
  delay(300);

  // I2C init (only once)
  Wire.begin(SDA, SCL);
  Wire.setClock(I2C_HZ);
  Wire.setTimeOut(I2C_TIMEOUT_MS);

  // Sensor init
  sensor.begin(Wire, STCC4_I2C_ADDR_64);

  // make sure we are not stuck in sleep from a previous session
  (void)sensor.exitSleepMode();

  // --- Optional: one-time warm-up in continuous mode after boot ---
  warmup_continuous_mode();  // comment out if not desired

  // Go to sleep because we'll use single-shot
  (void)sensor.enterSleepMode();

  Serial.println(F("Init done, entering single-shot loop."));
}

void loop() {
  static uint32_t last_ms = 0;
  const uint32_t now = millis();
  if (now - last_ms < MEAS_INTERVAL_MS) {
    delay(5);
    return;
  }
  last_ms = now;

  int16_t co2 = 0;
  float t = 0.0f, rh = 0.0f;
  uint16_t status = 0;

  // wake up for single-shot
  error = sensor.exitSleepMode();
  if (error != NO_ERROR) {
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.print(F("exitSleepMode: ")); Serial.println(errorMessage);
    return;
  }

  // trigger single-shot
  error = sensor.measureSingleShot();
  if (error != NO_ERROR) {
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.print(F("measureSingleShot: ")); Serial.println(errorMessage);
    (void)sensor.enterSleepMode();
    return;
  }

  // poll until data are ready (prevents I2C NACK logs)
  {
    const uint32_t t0 = millis();
    int16_t e;
    do {
      delay(POLL_STEP_MS);
      e = sensor.readMeasurement(co2, t, rh, status);
      if (e == NO_ERROR) {
        error = NO_ERROR;
        break;
      }
      error = e;
    } while (millis() - t0 < POLL_TIMEOUT_MS);

    if (error != NO_ERROR) {
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.print(F("readMeasurement timeout: "));
      Serial.println(errorMessage);
      (void)sensor.enterSleepMode();
      return;
    }
  }

  // back to sleep (power saving)
  (void)sensor.enterSleepMode();

  // print nicely
  Serial.print(F("CO2 [ppm] = ")); Serial.println(co2);
  Serial.print(F("T [C]     = ")); Serial.println(t, 2);
  Serial.print(F("RH [%]    = ")); Serial.println(rh, 2);
  Serial.print(F("Status    = 0x")); Serial.println(status, HEX);
}
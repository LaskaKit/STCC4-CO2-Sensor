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
static bool conditioned = false;   // jednorázové po bootu

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);
  Serial.println("Power ON");
  delay(300);

  Wire.begin(SDA, SCL);
  Wire.setClock(100000);           // STCC4 = 100 kHz je jistota

  sensor.begin(Wire, STCC4_I2C_ADDR_64);

  // Ujisti se, že nejsme ve sleepu
  (void)sensor.exitSleepMode();

  // Doporučení: po delší pauze proveď kondicionování (~22 s)
  // aby se CO2 rychleji ustálilo a neviselo na 390 ppm.
  // Spusť jen jednou po bootu.
  if (!conditioned) {
    int16_t e = sensor.performConditioning(); // pokud tvoje verze knihovny má
    if (e == NO_ERROR) {
      Serial.println("Conditioning...");
      delay(22500); // vyčkej na dokončení
    }
    conditioned = true;
  }

  // Hned poté uspíme, protože pojedeme single-shotem
  (void)sensor.enterSleepMode();
}

void loop() {
  int16_t co2 = 0;
  float t = 0.0f, rh = 0.0f;
  uint16_t status = 0;

  delay(10000); // měř každých 10 s

  // probuď, změř single-shot
  error = sensor.exitSleepMode();
  if (error != NO_ERROR) {
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.print("exitSleepMode: "); Serial.println(errorMessage);
    return;
  }

  error = sensor.measureSingleShot();
  if (error != NO_ERROR) {
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.print("measureSingleShot: "); Serial.println(errorMessage);
    (void)sensor.enterSleepMode();
    return;
  }

  // počkej na data (typicky < 200 ms); místo delay můžeš udělat 3× polling
  delay(220);

  error = sensor.readMeasurement(co2, t, rh, status);
  if (error != NO_ERROR) {
    // když se zpozdí, zkus o chlup později
    delay(150);
    error = sensor.readMeasurement(co2, t, rh, status);
    if (error != NO_ERROR) {
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.print("readMeasurement: "); Serial.println(errorMessage);
      (void)sensor.enterSleepMode();
      return;
    }
  }

  // zpět do sleepu (úspora)
  (void)sensor.enterSleepMode();

  // výpis
  Serial.print("CO2 [ppm] = "); Serial.println(co2);
  Serial.print("T [C]     = "); Serial.println(t, 2);
  Serial.print("RH [%]    = "); Serial.println(rh, 2);
  Serial.print("Status    = 0x"); Serial.println(status, HEX);
}
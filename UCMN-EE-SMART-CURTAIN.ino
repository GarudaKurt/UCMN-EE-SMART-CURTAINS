#include <Arduino.h>
#include <Wire.h>       // I2C – BH1750  (SDA=pin 20, SCL=pin 21 on Mega)
#include <BH1750.h>     // Library Manager: "BH1750" by Christopher Laws
#include <DHT.h>        // Library Manager: "DHT sensor library" by Adafruit
#include "motorControl.h"
#include "lcdDisplay.h"

// ============================================================
//  PIN DEFINITIONS
// ============================================================
#define DHT_PIN   48     // DHT22 DATA → Mega pin 48
#define DHT_TYPE  DHT22

// BH1750: SDA → pin 20, SCL → pin 21, VCC → 5V, GND → GND
// DHT22:  DATA → pin 48, VCC → 5V, GND → GND

// ============================================================
//  OBJECTS
// ============================================================
MOTORCONTROL stepper;
LCDDISPLAY   lcd;
DHT          dht(DHT_PIN, DHT_TYPE);
BH1750       lightMeter;

// ============================================================
//  SENSOR STATE
// ============================================================
float    sensorTemp  = 0.0f;
float    sensorHumid = 0.0f;
float    sensorLux   = 0.0f;

uint32_t lastReadMs  = 0;
#define  READ_INTERVAL  2000UL   // poll every 2 seconds

// ============================================================
//  READ SENSORS
// ============================================================
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Only reject NaN – any non-NaN value from DHT22 is stored
  if (!isnan(t)) sensorTemp  = t;
  if (!isnan(h)) sensorHumid = h;

  float lux = lightMeter.readLightLevel();
  if (lux >= 0.0f) sensorLux = lux;

  Serial.print(F("T="));     Serial.print(sensorTemp,  1);
  Serial.print(F("C  H="));  Serial.print(sensorHumid, 1);
  Serial.print(F("%  Lux=")); Serial.println(sensorLux, 1);
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println(F("Smart Curtains v1.0 - Booting..."));

  Wire.begin();

  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 OK"));
  } else {
    Serial.println(F("BH1750 ERROR - check SDA/SCL"));
  }

  dht.begin();
  Serial.println(F("DHT22 OK - waiting 2s to stabilise..."));
  delay(2000);

  stepper.initStepper();

  lcd.begin();

  readSensors();
  lcd.showScreen(sensorTemp, sensorHumid, sensorLux);

  Serial.println(F("Ready. Send ON / OFF to control curtains."));
}

// ============================================================
//  LOOP
// ============================================================
void loop() {

  // ── Serial motor commands ─────────────────────────────────
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if      (cmd == "ON")  stepper.upWard();
    else if (cmd == "OFF") stepper.downWard();
  }

  // ── Sensor poll ───────────────────────────────────────────
  uint32_t now = millis();
  if (now - lastReadMs >= READ_INTERVAL) {
    lastReadMs = now;
    readSensors();
  }

  // ── LCD refresh (animation self-timed inside showScreen) ──
  lcd.showScreen(sensorTemp, sensorHumid, sensorLux);

  delay(20);
}
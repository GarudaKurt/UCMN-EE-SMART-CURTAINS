#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include "motorControl.h"
#include "lcdDisplay.h"

#define DHT_PIN   48
#define DHT_TYPE  DHT22

const int btnForward = 45;
const int btnReverse = 46;

MOTORCONTROL stepper;
LCDDISPLAY   lcd;
DHT          dht(DHT_PIN, DHT_TYPE);
BH1750       lightMeter;

float    sensorTemp  = 0.0f;
float    sensorHumid = 0.0f;
float    sensorLux   = 0.0f;

uint32_t lastReadMs = 0;
uint32_t lastLcdMs  = 0;              // ← declared ONCE here

#define READ_INTERVAL  2000UL
#define LCD_INTERVAL    200UL         // ← declared ONCE here

void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) sensorTemp  = t;
  if (!isnan(h)) sensorHumid = h;
  float lux = lightMeter.readLightLevel();
  if (lux >= 0.0f) sensorLux = lux;
  Serial.print(F("T="));      Serial.print(sensorTemp,  1);
  Serial.print(F("C  H="));   Serial.print(sensorHumid, 1);
  Serial.print(F("%  Lux=")); Serial.println(sensorLux, 1);
}

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

  stepper.initStepper();             // ← only once

  lcd.begin();
  readSensors();
  lcd.showScreen(sensorTemp, sensorHumid, sensorLux);

  pinMode(btnForward, INPUT_PULLUP);
  pinMode(btnReverse, INPUT_PULLUP);

  Serial.println(F("Ready."));
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if      (cmd == "OPEN")  stepper.manualOpen();
    else if (cmd == "BLOCK") stepper.manualSunBlock();
    else if (cmd == "CLOSE") stepper.manualClose();
    else if (cmd == "STOP")  stepper.stopMotor();
  }

  uint32_t now = millis();

  if (now - lastReadMs >= READ_INTERVAL) {
    lastReadMs = now;
    readSensors();
    stepper.checkAndControl(sensorTemp, sensorHumid, sensorLux);
  }

  bool fwdHeld = (digitalRead(btnForward) == LOW);
  bool revHeld = (digitalRead(btnReverse) == LOW);

  if (fwdHeld) {
    stepper.manualForwardStep();
  } else if (revHeld) {
    stepper.manualReverseStep();
  } else {
    stepper.disableDriver();
  }

  if (now - lastLcdMs >= LCD_INTERVAL) {
    lastLcdMs = now;
    lcd.showScreen(sensorTemp, sensorHumid, sensorLux);
  }
}
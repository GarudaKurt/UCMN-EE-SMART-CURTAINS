#include "motorControl.h"
#include <Arduino.h>

const int DIR_PIN = 7;
const int STEP_PIN = 8;
const int ENABLE_PIN = 6  ;

MOTORCONTROL::MOTORCONTROL() {}
MOTORCONTROL::~MOTORCONTROL() {}

void MOTORCONTROL::initStepper() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);
}

void MOTORCONTROL::downWard(float temp, float humid, float light) {

  digitalWrite(DIR_PIN, HIGH);

  for (int i = 0; i < 500 * 10; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(100);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(100);
  }
}

void MOTORCONTROL::upWard(float temp, float humid, float light) {
  digitalWrite(DIR_PIN, LOW);

  for (int i = 0; i < 500 * 10; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(100);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(100);
  }
}

void MOTORCONTROL::manualForwardStep() {
  digitalWrite(DIR_PIN, HIGH);

  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(100);
}


void MOTORCONTROL::manualReverseStep() {
  digitalWrite(DIR_PIN, LOW);

  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(100);
}


void MOTORCONTROL::stopMotor() {
  Serial.println("Motor STOP requested");
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(ENABLE_PIN, HIGH);
  delay(1000);
  digitalWrite(ENABLE_PIN, LOW);
}
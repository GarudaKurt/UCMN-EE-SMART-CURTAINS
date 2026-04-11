#include "motorControl.h"
#include <Arduino.h>

const int DIR_PIN    = 7;
const int STEP_PIN   = 8;
const int ENABLE_PIN = 6;

// ============================================================
//  CONSTRUCTOR / DESTRUCTOR
// ============================================================
MOTORCONTROL::MOTORCONTROL()
  : _currentState(BLIND_UNKNOWN), _currentSteps(0)
{}

MOTORCONTROL::~MOTORCONTROL() {}

// ============================================================
//  INIT
// ============================================================
void MOTORCONTROL::initStepper() {
  pinMode(STEP_PIN,   OUTPUT);
  pinMode(DIR_PIN,    OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  // Start disabled – moveTo() will enable when needed
  digitalWrite(ENABLE_PIN, HIGH);

  // Force physical position to 0° on boot
  // Set steps high first so moveTo() sees a delta and actually moves
  _currentSteps = STEPS_FULL_OPEN;   // pretend we're fully open
  _currentState = BLIND_UNKNOWN;

  manualClose();                      // physically drive to 0° (closed)

  Serial.println(F("Blind homed to 0deg (closed) on boot"));
}

// ============================================================
//  AUTOMATION  – call inside loop() on fresh sensor data only
// ============================================================
void MOTORCONTROL::checkAndControl(float temp, float humid, float lux) {

  // ── Evaluate conditions in priority order ─────────────────

  // Priority 1: Hot AND humid → open fully for ventilation
  bool isHotAndHumid = (temp >= TEMP_HOT_ABOVE) && (humid >= HUMID_HIGH_ABOVE);

  // Priority 2: Very bright sun → tilt to block glare
  bool isTooSunny = (lux > LUX_BLOCK_ABOVE);

  // Priority 3: Too dark → open fully to let light in
  bool isTooDark = (lux < LUX_OPEN_BELOW);

  // ── Determine desired state ───────────────────────────────
  BlindState desired;

  if (isHotAndHumid) {
    desired = BLIND_OPEN;       // ventilation takes top priority
  } else if (isTooSunny) {
    desired = BLIND_SUNBLOCK;   // block harsh sun
  } else if (isTooDark) {
    desired = BLIND_OPEN;       // let light in
  } else {
    return;                     // comfortable zone – do nothing
  }

  // ── Only move if state actually needs to change ───────────
  if (desired == _currentState) return;

  switch (desired) {

    case BLIND_OPEN:
      if (isHotAndHumid) {
        Serial.print(F("AUTO: Temp="));
        Serial.print(temp, 1);
        Serial.print(F("C Humid="));
        Serial.print(humid, 1);
        Serial.println(F("% -> Hot & humid, opening fully (180deg)"));
      } else {
        Serial.println(F("AUTO: Low light -> opening fully (180deg)"));
      }
      moveTo(STEPS_FULL_OPEN);
      _currentState = BLIND_OPEN;
      break;

    case BLIND_SUNBLOCK:
      Serial.println(F("AUTO: High light -> sun-block position (75deg)"));
      moveTo(STEPS_SUN_BLOCK);
      _currentState = BLIND_SUNBLOCK;
      break;

    default:
      break;
  }
}

// ============================================================
//  MANUAL CONTROLS
// ============================================================
void MOTORCONTROL::manualOpen() {
  Serial.println(F("MANUAL: Full open (180deg)"));
  moveTo(STEPS_FULL_OPEN);
  _currentState = BLIND_OPEN;
}

void MOTORCONTROL::manualSunBlock() {
  Serial.println(F("MANUAL: Sun-block (75deg)"));
  moveTo(STEPS_SUN_BLOCK);
  _currentState = BLIND_SUNBLOCK;
}

void MOTORCONTROL::manualClose() {
  Serial.println(F("MANUAL: Closing (0deg)"));
  moveTo(STEPS_CLOSED);
  _currentState = BLIND_CLOSED;
}

// ============================================================
//  STOP
// ============================================================
void MOTORCONTROL::stopMotor() {
  Serial.println(F("Motor STOP requested"));
  digitalWrite(STEP_PIN,   LOW);
  digitalWrite(ENABLE_PIN, HIGH);
  delay(1000);
  digitalWrite(ENABLE_PIN, LOW);
}

void MOTORCONTROL::manualForwardStep() {
  digitalWrite(ENABLE_PIN, LOW);
  digitalWrite(DIR_PIN, HIGH);
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_DELAY_US);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(STEP_DELAY_US);
  digitalWrite(ENABLE_PIN, HIGH);  // disable after each jog step
  _currentSteps++;
}

void MOTORCONTROL::manualReverseStep() {
  digitalWrite(ENABLE_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_DELAY_US);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(STEP_DELAY_US);
  digitalWrite(ENABLE_PIN, HIGH);  // disable after each jog step
  _currentSteps--;
}

// ============================================================
//  CORE MOVEMENT  – travels only the delta from current pos
// ============================================================
void MOTORCONTROL::moveTo(int targetSteps) {
  if (targetSteps == _currentSteps) return;

  bool forward     = (targetSteps > _currentSteps);
  int  stepsNeeded = abs(targetSteps - _currentSteps);

  digitalWrite(ENABLE_PIN, LOW);   // enable driver just before moving
  digitalWrite(DIR_PIN, forward ? HIGH : LOW);

  Serial.print(F("Moving "));
  Serial.print(stepsNeeded);
  Serial.println(forward ? F(" steps FORWARD") : F(" steps BACKWARD"));

  for (int i = 0; i < stepsNeeded; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_DELAY_US);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_DELAY_US);
  }

  _currentSteps = targetSteps;

  // Disable driver after move – motor runs cool, blind holds position
  // mechanically so holding torque is not needed
  delay(50);                        // brief settle before cut
  digitalWrite(ENABLE_PIN, HIGH);  // HIGH = disabled on TB6600
}
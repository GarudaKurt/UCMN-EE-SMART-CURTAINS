#ifndef __MOTORCONTROL__H
#define __MOTORCONTROL__H

#include <Arduino.h>

// ============================================================
//  TB6600 TUNING  – match STEPS_PER_REV to your DIP switches
// ============================================================
// Full step  (SW3=OFF, SW4=OFF) → 200
// Half step  (SW3=ON,  SW4=OFF) → 400
// 1/4  step  (SW3=OFF, SW4=ON ) → 800
// 1/8  step  (SW3=ON,  SW4=ON ) → 1600
// 1/16 step                     → 3200
// 1/32 step                     → 6400
#define STEPS_PER_REV     1600    // <-- change to match your SW3/SW4

#define STEPS_FULL_OPEN   (STEPS_PER_REV)                            // 180°
#define STEPS_SUN_BLOCK   ((int)((75.0f / 180.0f) * STEPS_PER_REV)) // 75°
#define STEPS_CLOSED      0

#define STEP_DELAY_US     100     // pulse speed – lower = faster

// ============================================================
//  LUX THRESHOLDS
// ============================================================
#define LUX_OPEN_BELOW    100.0f  // lux < 100  → open fully (180°)
#define LUX_BLOCK_ABOVE   500.0f  // lux > 500  → tilt to 75°

// ============================================================
//  TEMP / HUMIDITY THRESHOLDS
// ============================================================
#define TEMP_HOT_ABOVE    35.0f   // temp >= 35°C  ─┐ both must be
#define HUMID_HIGH_ABOVE  75.0f   // humid >= 75%  ─┘ true → open fully

// ============================================================
//  BLIND STATES
// ============================================================
enum BlindState {
  BLIND_UNKNOWN  = 0,
  BLIND_OPEN     = 1,   // 180° – full open
  BLIND_SUNBLOCK = 2,   //  75° – sun block
  BLIND_CLOSED   = 3    //   0° – closed
};

// ============================================================
//  TRIGGER REASONS  (for Serial logging)
// ============================================================
enum OpenReason {
  REASON_NONE     = 0,
  REASON_LOW_LUX  = 1,   // lux < LUX_OPEN_BELOW
  REASON_HOT_HUMID = 2   // temp >= TEMP_HOT_ABOVE && humid >= HUMID_HIGH_ABOVE
};

class MOTORCONTROL {
  public:
    MOTORCONTROL();
    ~MOTORCONTROL();

    void initStepper();
    void checkAndControl(float temp, float humid, float lux);  // automation
    void manualOpen();        // full open  180°
    void manualSunBlock();    // sun-block   75°
    void manualClose();      
    void stopMotor();
    void manualForwardStep(); 
    void manualReverseStep();

    BlindState getState()  { return _currentState; }

  private:
    BlindState _currentState;
    int        _currentSteps;

    void moveTo(int targetSteps);
};

#endif
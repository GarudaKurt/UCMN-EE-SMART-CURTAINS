#ifndef __LCDDISPLAY__H
#define __LCDDISPLAY__H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// I2C address – most common is 0x27, try 0x3F if blank
#define LCD_I2C_ADDR  0x27
#define LCD_COLS      16
#define LCD_ROWS       2

class LCDDISPLAY {
  public:
    LCDDISPLAY();
    void begin();
    void showScreen(float temp, float humid, float lux);
    void forceRedraw() { _needFullRedraw = true; }

  private:
    LiquidCrystal_I2C *_lcd;

    bool     _needFullRedraw;
    float    _lastTemp;
    float    _lastHumid;
    float    _lastLux;
    uint8_t  _page;
    uint32_t _lastPageMs;

    void showPage0(float lux);
    void showPage1(float temp, float humid);
};

#endif
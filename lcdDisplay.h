#ifndef __LCDDISPLAY__H
#define __LCDDISPLAY__H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Arduino.h>

// -----------------------------------------------------------
//  TFT PIN DEFINITIONS  (Arduino Mega hardware SPI)
//  MOSI = Pin 51,  SCK = Pin 52
// -----------------------------------------------------------
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST  12

// -----------------------------------------------------------
//  COLOURS  (RGB565)
// -----------------------------------------------------------
#define C_BLACK      0x0000
#define C_WHITE      0xFFFF
#define C_YELLOW     0xFFE0
#define C_GOLD       0xFEA0
#define C_ORANGE     0xFD20
#define C_RED        0xF800
#define C_GREEN      0x07E0
#define C_CYAN       0x07FF
#define C_BLUE       0x001F
#define C_NAVY       0x000F
#define C_DKGRAY     0x4208
#define C_MDGRAY     0x7BEF
#define C_LTGRAY     0xC618
#define C_SKY        0x867D
#define C_NIGHT      0x0849
#define C_CLOUD      0xDEFB
#define C_SUNHALO    0xFFC0
#define C_HOTRED     0xFA20

// -----------------------------------------------------------
//  LAYOUT CONSTANTS
// -----------------------------------------------------------
#define SCREEN_W    240
#define SCREEN_H    320
#define HEADER_H     48
#define ICON_ZONE_Y  52
#define ICON_ZONE_H 120
#define CARD_Y      182
#define CARD_H      130
#define FOOTER_Y    318

// Weather icon states
#define ICON_SUNNY   0
#define ICON_PARTLY  1
#define ICON_CLOUDY  2
#define ICON_HOT     3

// -----------------------------------------------------------
//  CLASS
// -----------------------------------------------------------
class LCDDISPLAY {
  public:
    LCDDISPLAY();
    void begin();
    void showScreen(float temp, float humid, float lux);  // call every loop()
    void forceRedraw() { _needFullRedraw = true; }

  private:
    Adafruit_ST7789 *tft;

    bool     _needFullRedraw;
    float    _lastTemp;
    float    _lastHumid;
    float    _lastLux;
    uint8_t  _lastIcon;
    uint8_t  _animFrame;
    uint32_t _lastAnimMs;
    uint8_t  _sunPulse;

    void    drawStaticChrome();
    void    drawHeader();
    void    drawSensorCards(float temp, float humid, float lux);
    void    drawFooterDots(float lux);

    uint8_t chooseIcon(float temp, float humid, float lux);
    void    drawIconArea(uint8_t icon, uint8_t frame);
    void    drawSunnyIcon(int cx, int cy, uint8_t frame);
    void    drawPartlyCloudyIcon(int cx, int cy, uint8_t frame);
    void    drawCloudyIcon(int cx, int cy, uint8_t frame);
    void    drawHotIcon(int cx, int cy, uint8_t frame);

    void    drawSunRays(int cx, int cy, int innerR, int outerR, uint16_t col);
    void    drawCloud(int x, int y, uint16_t col, float scale);
    void    printCentered(const char *str, int y, uint8_t sz, uint16_t col);
    void    printValue(int x, int y, float val, const char *unit,
                       uint16_t valCol, uint16_t unitCol);
};

#endif
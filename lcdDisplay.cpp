#include "lcdDisplay.h"
#include <Arduino.h>

// ============================================================
//  CONSTRUCTOR / BEGIN
// ============================================================
LCDDISPLAY::LCDDISPLAY()
  : _needFullRedraw(true),
    _lastTemp(-999.0f), _lastHumid(-999.0f), _lastLux(-999.0f),
    _lastIcon(255), _animFrame(0), _lastAnimMs(0), _sunPulse(0)
{
  tft = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
}

void LCDDISPLAY::begin() {
  tft->init(240, 320);
  tft->setRotation(0);
  tft->fillScreen(C_BLACK);
  _needFullRedraw = true;
}

// ============================================================
//  MAIN UPDATE  – call every loop()
// ============================================================
void LCDDISPLAY::showScreen(float temp, float humid, float lux) {

  // Advance animation frame every 400 ms
  uint32_t now = millis();
  bool newFrame = false;
  if (now - _lastAnimMs >= 400UL) {
    _animFrame  = (_animFrame + 1) & 0x03;   // 0–3
    _sunPulse   = (_sunPulse  + 1) & 0x07;   // 0–7
    _lastAnimMs = now;
    newFrame    = true;
  }

  uint8_t icon = chooseIcon(temp, humid, lux);
  bool iconChanged = (icon != _lastIcon);

  // Use epsilon comparison – avoids "never updates" with float !=
  bool valuesChanged = (fabsf(temp  - _lastTemp)  > 0.05f)
                    || (fabsf(humid - _lastHumid) > 0.05f)
                    || (fabsf(lux   - _lastLux)   > 0.5f);

  // Full redraw on first boot or after forceRedraw()
  if (_needFullRedraw) {
    tft->fillScreen(C_BLACK);
    drawStaticChrome();
    drawIconArea(icon, _animFrame);
    drawSensorCards(temp, humid, lux);
    drawFooterDots(lux);
    _needFullRedraw = false;
  } else {
    if (newFrame || iconChanged) drawIconArea(icon, _animFrame);
    if (valuesChanged)           drawSensorCards(temp, humid, lux);
    if (valuesChanged)           drawFooterDots(lux);
  }

  _lastTemp  = temp;
  _lastHumid = humid;
  _lastLux   = lux;
  _lastIcon  = icon;
}

// ============================================================
//  STATIC CHROME  (drawn once)
// ============================================================
void LCDDISPLAY::drawStaticChrome() {
  drawHeader();

  // Card panel background
  tft->fillRect(0, CARD_Y, SCREEN_W, CARD_H, C_NIGHT);

  // Divider
  tft->drawFastHLine(10, CARD_Y - 1, SCREEN_W - 20, C_DKGRAY);

  // Three sensor card outlines
  tft->drawRoundRect(6,   CARD_Y + 6, 70, 54, 6, C_DKGRAY);   // Temp
  tft->drawRoundRect(85,  CARD_Y + 6, 70, 54, 6, C_DKGRAY);   // Humid
  tft->drawRoundRect(164, CARD_Y + 6, 70, 54, 6, C_DKGRAY);   // Light

  // Card labels
  tft->setTextSize(1);
  tft->setTextColor(C_MDGRAY);
  tft->setCursor(14,  CARD_Y + 12); tft->print("TEMP");
  tft->setCursor(92,  CARD_Y + 12); tft->print("HUMID");
  tft->setCursor(170, CARD_Y + 12); tft->print("LIGHT");

  // Curtain status card
  tft->drawRoundRect(6, CARD_Y + 68, 228, 50, 6, C_DKGRAY);
  tft->setTextColor(C_MDGRAY);
  tft->setCursor(14, CARD_Y + 74);
  tft->print("CURTAIN STATUS");
}

// ============================================================
//  HEADER
// ============================================================
void LCDDISPLAY::drawHeader() {
  tft->fillRect(0, 0, SCREEN_W, HEADER_H, C_NAVY);
  tft->fillRect(0, HEADER_H - 4, SCREEN_W, 4, C_BLUE);

  tft->setTextSize(2);
  tft->setTextColor(C_WHITE);
  tft->setCursor(14, 8);
  tft->print("Smart Curtains");

  tft->setTextSize(1);
  tft->setTextColor(C_CYAN);
  tft->setCursor(14, 32);
  tft->print("Environmental Monitor");
}

// ============================================================
//  ICON AREA
// ============================================================
void LCDDISPLAY::drawIconArea(uint8_t icon, uint8_t frame) {
  // Sky gradient (3 colour bands)
  uint16_t skyTop, skyMid, skyBot;
  if (icon == ICON_HOT || icon == ICON_SUNNY) {
    skyTop = C_SKY;   skyMid = 0x867F; skyBot = 0xC6FF;
  } else if (icon == ICON_PARTLY) {
    skyTop = 0x630F;  skyMid = C_SKY;  skyBot = 0xAEFF;
  } else {
    skyTop = C_NIGHT; skyMid = C_DKGRAY; skyBot = C_DKGRAY;
  }
  uint16_t bands[6] = {skyTop, skyTop, skyMid, skyMid, skyBot, skyBot};
  int bandH = ICON_ZONE_H / 6;
  for (int i = 0; i < 6; i++) {
    tft->fillRect(0, ICON_ZONE_Y + i * bandH, SCREEN_W, bandH, bands[i]);
  }

  int cx = SCREEN_W / 2;
  int cy = ICON_ZONE_Y + ICON_ZONE_H / 2;

  switch (icon) {
    case ICON_SUNNY:  drawSunnyIcon(cx, cy, frame);        break;
    case ICON_PARTLY: drawPartlyCloudyIcon(cx, cy, frame); break;
    case ICON_CLOUDY: drawCloudyIcon(cx, cy, frame);       break;
    case ICON_HOT:    drawHotIcon(cx, cy, frame);          break;
  }
}

// ---- Sunny ------------------------------------------------
void LCDDISPLAY::drawSunnyIcon(int cx, int cy, uint8_t frame) {
  tft->fillCircle(cx, cy, 36 + (_sunPulse & 3), C_SUNHALO);
  int outerR = 46 + (frame & 1) * 4;
  drawSunRays(cx, cy, 30, outerR, C_YELLOW);
  tft->fillCircle(cx, cy, 28, C_YELLOW);
  tft->fillCircle(cx, cy, 22, C_GOLD);
  tft->fillCircle(cx - 7, cy - 7, 5, C_WHITE);
  tft->fillCircle(cx - 7, cy - 7, 3, C_GOLD);
}

// ---- Partly cloudy ----------------------------------------
void LCDDISPLAY::drawPartlyCloudyIcon(int cx, int cy, uint8_t frame) {
  int sx = cx - 20, sy = cy - 14;
  tft->fillCircle(sx, sy, 20, C_SUNHALO);
  tft->fillCircle(sx, sy, 16, C_YELLOW);
  drawSunRays(sx, sy, 18, 28 + (frame & 1) * 3, C_ORANGE);
  int drift = (frame & 1) ? 2 : 0;
  drawCloud(cx - 10 + drift, cy + 6, C_CLOUD, 1.0f);
}

// ---- Cloudy -----------------------------------------------
void LCDDISPLAY::drawCloudyIcon(int cx, int cy, uint8_t frame) {
  int bob = (frame & 1) ? 2 : 0;
  drawCloud(cx - 30, cy - 8 + bob, C_MDGRAY, 0.85f);
  drawCloud(cx + 10, cy + 4 - bob, C_CLOUD,  1.0f);
}

// ---- Hot --------------------------------------------------
void LCDDISPLAY::drawHotIcon(int cx, int cy, uint8_t frame) {
  int sy = cy - 10;
  tft->fillCircle(cx, sy, 42 + (frame & 3), 0xFB00);
  tft->fillCircle(cx, sy, 30, C_ORANGE);
  tft->fillCircle(cx, sy, 22, C_RED);
  drawSunRays(cx, sy, 32, 50 + frame * 2, C_HOTRED);

  int waveY = cy + 26;
  for (int w = 0; w < 3; w++) {
    int wy = waveY + w * 8;
    uint16_t wc = (w == 1) ? C_ORANGE : C_HOTRED;
    for (int x = 30; x < SCREEN_W - 30; x += 8) {
      int yo = ((x / 8 + frame + w) & 1) ? 2 : 0;
      tft->drawFastHLine(x, wy + yo, 6, wc);
    }
  }
}

// ============================================================
//  SENSOR CARDS
// ============================================================
void LCDDISPLAY::drawSensorCards(float temp, float humid, float lux) {

  // Helper: clear a value area before redrawing
  auto clr = [&](int x, int y) {
    tft->fillRect(x, y, 68, 30, C_NIGHT);
  };

  // ── Temperature ──────────────────────────────────────────
  clr(8, CARD_Y + 22);
  uint16_t tCol = (temp > 35) ? C_RED
               : (temp > 28) ? C_ORANGE
               :                C_CYAN;
  printValue(8, CARD_Y + 22, temp, "C", tCol, C_MDGRAY);

  // ── Humidity ─────────────────────────────────────────────
  clr(87, CARD_Y + 22);
  uint16_t hCol = (humid > 80) ? C_CYAN
               : (humid < 30)  ? C_YELLOW
               :                  C_GREEN;
  printValue(87, CARD_Y + 22, humid, "%", hCol, C_MDGRAY);

  // ── Light ────────────────────────────────────────────────
  clr(164, CARD_Y + 22);
  char luxBuf[8];
  if (lux >= 1000) {
    int whole = (int)(lux / 1000.0f);
    int frac  = (int)(((lux / 1000.0f) - whole) * 10.0f + 0.5f);
    if (frac >= 10) { whole++; frac = 0; }
    snprintf(luxBuf, sizeof(luxBuf), "%d.%dk", whole, frac);
  } else {
    snprintf(luxBuf, sizeof(luxBuf), "%d", (int)(lux + 0.5f));
  }
  tft->setTextColor(C_YELLOW);
  tft->setCursor(168, CARD_Y + 24);
  tft->print(luxBuf);
  tft->setTextSize(1);
  tft->setTextColor(C_MDGRAY);
  tft->setCursor(168, CARD_Y + 44);
  tft->print("lux");

  // ── Curtain status ───────────────────────────────────────
  tft->fillRect(8, CARD_Y + 76, 224, 34, C_NIGHT);

  const char *msg;
  uint16_t    mCol;
  if      (lux  > 800) { msg = "CLOSING - High Light"; mCol = C_ORANGE; }
  else if (lux  > 300) { msg = "OPEN  - Good Light";   mCol = C_GREEN;  }
  else if (temp > 35)  { msg = "CLOSING - Too Hot";    mCol = C_RED;    }
  else                 { msg = "OPEN  - Normal";        mCol = C_CYAN;   }

  tft->setTextSize(1);
  tft->setTextColor(mCol);
  tft->setCursor(14, CARD_Y + 91);
  tft->print(msg);

  uint16_t dotCol = (mCol == C_GREEN || mCol == C_CYAN) ? C_GREEN : C_RED;
  tft->fillCircle(222, CARD_Y + 91, 5, dotCol);
}

// ============================================================
//  FOOTER DOTS  – lux bar (0-1000 lux → 0-5 dots)
// ============================================================
void LCDDISPLAY::drawFooterDots(float lux) {
  int filled = constrain((int)(lux / 200.0f), 0, 5);
  tft->fillRect(0, FOOTER_Y - 4, SCREEN_W, 8, C_NIGHT);
  for (int i = 0; i < 5; i++) {
    tft->fillCircle(100 + i * 10, FOOTER_Y, 3,
                    (i < filled) ? C_YELLOW : C_DKGRAY);
  }
}

// ============================================================
//  ICON SELECTION
// ============================================================
uint8_t LCDDISPLAY::chooseIcon(float temp, float humid, float lux) {
  if (temp > 36) return ICON_HOT;
  if (lux  > 600) return ICON_SUNNY;
  if (lux  > 200) return ICON_PARTLY;
  return ICON_CLOUDY;
}

// ============================================================
//  DRAW PRIMITIVES
// ============================================================
void LCDDISPLAY::drawSunRays(int cx, int cy, int innerR, int outerR, uint16_t col) {
  const float angles[8] = {0, 45, 90, 135, 180, 225, 270, 315};
  for (int i = 0; i < 8; i++) {
    float rad = angles[i] * PI / 180.0f;
    int x1 = cx + (int)(cos(rad) * innerR);
    int y1 = cy + (int)(sin(rad) * innerR);
    int x2 = cx + (int)(cos(rad) * outerR);
    int y2 = cy + (int)(sin(rad) * outerR);
    tft->drawLine(x1,   y1, x2,   y2, col);
    tft->drawLine(x1+1, y1, x2+1, y2, col);   // thicken
  }
}

void LCDDISPLAY::drawCloud(int x, int y, uint16_t col, float scale) {
  int s = (int)(scale * 10);
  tft->fillRoundRect(x,       y,      5*s, 2*s, s,            col);
  tft->fillCircle   (x + s,   y,      s,                       col);
  tft->fillCircle   (x + 2*s, y - s,  (int)(1.4f * s),         col);
  tft->fillCircle   (x + 3*s, y,      (int)(1.1f * s),         col);
}

// ============================================================
//  TEXT HELPERS
// ============================================================
void LCDDISPLAY::printCentered(const char *str, int y, uint8_t sz, uint16_t col) {
  tft->setTextSize(sz);
  tft->setTextColor(col);
  int16_t x1, y1; uint16_t w, h;
  tft->getTextBounds(str, 0, y, &x1, &y1, &w, &h);
  tft->setCursor((SCREEN_W - (int)w) / 2, y);
  tft->print(str);
}

void LCDDISPLAY::printValue(int x, int y, float val, const char *unit,
                             uint16_t valCol, uint16_t unitCol) {
  // Split float into integer and one decimal digit manually
  // avoids snprintf float bug on AVR (outputs '?' without linker flag)
  bool negative = (val < 0);
  if (negative) val = -val;

  int whole = (int)val;
  int frac  = (int)((val - whole) * 10.0f + 0.5f);  // rounded 1 d.p.
  if (frac >= 10) { whole++; frac = 0; }             // carry

  char buf[10];
  if (negative)
    snprintf(buf, sizeof(buf), "-%d.%d", whole, frac);
  else
    snprintf(buf, sizeof(buf), "%d.%d",  whole, frac);

  tft->setTextSize(2);
  tft->setTextColor(valCol);
  tft->setCursor(x + 2, y);
  tft->print(buf);

  tft->setTextSize(1);
  tft->setTextColor(unitCol);
  tft->setCursor(x + 2, y + 18);
  tft->print(unit);
}
#include "lcdDisplay.h"

#define PAGE_MS  3000UL

LCDDISPLAY::LCDDISPLAY()
  : _needFullRedraw(true),
    _lastTemp(-999.f), _lastHumid(-999.f), _lastLux(-999.f),
    _page(0), _lastPageMs(0)
{
  _lcd = new LiquidCrystal_I2C(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
}

void LCDDISPLAY::begin() {
  _lcd->init();
  _lcd->backlight();
  _lcd->clear();
  _needFullRedraw = true;
}

// ── Main update ──────────────────────────────────────────────
void LCDDISPLAY::showScreen(float temp, float humid, float lux) {
  uint32_t now = millis();

  bool valuesChanged = (fabsf(temp  - _lastTemp)  > 0.05f)
                    || (fabsf(humid - _lastHumid) > 0.05f)
                    || (fabsf(lux   - _lastLux)   > 0.5f);

  if (_needFullRedraw || (now - _lastPageMs >= PAGE_MS)) {
    _page           = (_page + 1) & 0x01;
    _lastPageMs     = now;
    _needFullRedraw = false;
    valuesChanged   = true;
  }

  if (valuesChanged) {
    if (_page == 0) showPage0(lux);
    else            showPage1(temp, humid);
  }

  _lastTemp  = temp;
  _lastHumid = humid;
  _lastLux   = lux;
}

// ── Page 0 ───────────────────────────────────────────────────
//  Row 0:  Sun reflection:
//  Row 1:  320 lx
void LCDDISPLAY::showPage0(float lux) {
  char line[17];
  _lcd->clear();

  // Row 0 – static title
  _lcd->setCursor(0, 0);
  _lcd->print("Sun reflection: ");

  // Row 1 – lux value only
  if (lux >= 1000.f) {
    int whole = (int)(lux / 1000.f);
    int frac  = (int)(((lux / 1000.f) - whole) * 10.f + 0.5f);
    if (frac >= 10) { whole++; frac = 0; }
    snprintf(line, sizeof(line), "%d.%d klx        ", whole, frac);
  } else {
    snprintf(line, sizeof(line), "%d lx            ", (int)(lux + 0.5f));
  }
  line[16] = '\0';
  _lcd->setCursor(0, 1);
  _lcd->print(line);
}

// ── Page 1 ───────────────────────────────────────────────────
//  Row 0:  Temp:   Humidity
//  Row 1:  28.5C    65.2%
void LCDDISPLAY::showPage1(float temp, float humid) {
  char line[17];
  _lcd->clear();

  // Row 0 – labels only
  _lcd->setCursor(0, 0);
  _lcd->print("Temp:   Humidity");

  // Row 1 – both values
  int tWhole = (int)fabsf(temp);
  int tFrac  = (int)((fabsf(temp) - tWhole) * 10.f + 0.5f);
  if (tFrac >= 10) { tWhole++; tFrac = 0; }

  int hWhole = (int)humid;
  int hFrac  = (int)((humid - hWhole) * 10.f + 0.5f);
  if (hFrac >= 10) { hWhole++; hFrac = 0; }

  snprintf(line, sizeof(line), "%d.%dC    %d.%d%%  ", tWhole, tFrac, hWhole, hFrac);
  line[16] = '\0';
  _lcd->setCursor(0, 1);
  _lcd->print(line);
}
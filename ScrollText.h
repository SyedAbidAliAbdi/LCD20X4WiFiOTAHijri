#ifndef SCROLLTEXT_H
#define SCROLLTEXT_H
#include <LiquidCrystal_I2C.h>

void scrollText(LiquidCrystal_I2C& lcd, const String& text, bool scrollRightToLeft);

#endif

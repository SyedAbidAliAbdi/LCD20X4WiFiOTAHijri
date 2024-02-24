#include "ScrollText.h"

void scrollText(LiquidCrystal_I2C& lcd, const String& text, bool scrollRightToLeft) {
  int textLength = text.length();
  int displayLength = 20; // Assuming a 20x4 LCD
  int scrollDelay = 400; // Delay between each scroll step in milliseconds

  // Add spaces to the text to create the scrolling effect
  String paddedText = text;
  if (scrollRightToLeft) {
    paddedText = String(displayLength, ' ') + text + String(displayLength, ' ');
  } else {
    paddedText = String(displayLength, ' ') + text + String(displayLength - textLength, ' ');
  }

  while (true) {
    for (int i = 0; i <= paddedText.length() - displayLength; i++) {
      lcd.setCursor(0, 3); // Set cursor to 4th row
      lcd.print(paddedText.substring(i, i + displayLength));
      if (i == paddedText.length() - displayLength) {
        // If the end of the padded text is reached, pause for a moment
        delay(scrollDelay * 3);
      } else {
        // Otherwise, continue scrolling with the specified delay
        delay(scrollDelay);
      }
    }
  }
}

#include "menu.h"
#include <Arduino.h>

#define JOYSTICK_PIN=A2;

Menu::Menu() : JOYSTICK_PIN(A2), r(4), currentIndex(0), lastMoveTime(0) {
  r=4;
  currentIndex(0), lastMoveTime(0
  menuItems[0] = "MULTI";
  menuItems[1] = "SOLO";
  menuItems[2] = "PARAMETRES";
}

bool Menu::handleJoystick(Adafruit_SH1107& display) {
  int val = analogRead(JOYSTICK_PIN);
  if (millis() - lastMoveTime > 250) {
    if (val < 1000 && currentIndex > 0) {
      currentIndex--;
      lastMoveTime = millis();
    }
    else if (val > 2000 && val < 4000 && currentIndex < NUM_ITEMS - 1) { 
      currentIndex++;
      lastMoveTime = millis();
    }
    else if (val == 4095) {
      lastMoveTime = millis();
      return true;
    }
  }
  return false;
}

void Menu::drawMenu(Adafruit_SH1107& display) {
  display.clearDisplay();
  display.setTextSize(1);

  int hY = (currentIndex == 0) ? 2 : (currentIndex == 1) ? 20 : 41;
  display.fillRoundRect(2, hY, 123, (currentIndex == 1) ? 24 : 20, r, SH110X_WHITE);

  for (int i = 0; i < NUM_ITEMS; i++) {
    bool isSel = (currentIndex == i);
    display.setTextColor(isSel ? SH110X_BLACK : SH110X_WHITE);
    
    display.setCursor(8, 8 + (i * 20));

    if (isSel) display.write(0x10);
    else if (i == 0 && currentIndex > 0) display.write(0x18);
    else if (i == 2 && currentIndex < 2) display.write(0x19);
    else display.print(" ");
    
    display.print("  "); display.print(menuItems[i]);
  }

  display.drawRoundRect(0, 0, 127, 63, r, SH110X_WHITE);
  display.display();
}

void Menu::executeAction(int index, Adafruit_SH1107& display) {
  Serial.printf("Action : %s\n", menuItems[index]);
  for(int i = 0; i < 2; i++) {
    display.invertDisplay(true); delay(80);
    display.invertDisplay(false); delay(80);
  }
}

void Menu::reset() { currentIndex = 0; }
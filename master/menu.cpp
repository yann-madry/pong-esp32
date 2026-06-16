#include "menu.h"
#include <Arduino.h>

#define JOYSTICK_PIN A2

bool Menu::handleJoystick() {
    int val = analogRead(JOYSTICK_PIN);

    if (millis() - lastMoveTime > 250) {
        if (val < 1000 && currentIndex > 0) {
            currentIndex--;
            lastMoveTime = millis();
        }
        else if (val > 2000 && val < 4000 && currentIndex < numItems - 1) {
            currentIndex++;
            lastMoveTime = millis();
        }
        else if (val >= 4090) {
            lastMoveTime = millis();
            return true;
        }
    }
    return false;
}

void Menu::drawMenu(Adafruit_SH1107& display) {
    display.clearDisplay();
    display.setTextSize(1);

    const int menuTop = 2;
    const int menuHeight = 60;
    const int itemHeight = menuHeight / numItems;

    display.fillRoundRect(2, menuTop + currentIndex * itemHeight, 123, itemHeight, r, SH110X_WHITE);

    for (int i = 0; i < numItems; i++) {
        bool selected = (i == currentIndex);
        display.setTextColor(selected ? SH110X_BLACK : SH110X_WHITE);
        display.setCursor(8, menuTop + i * itemHeight + itemHeight / 2 - 4);

        if (selected) display.write(0x10);
        else display.print(" ");

        display.print("  ");
        display.print(items[i].label);
    }
    display.drawRoundRect(
        0,
        0,
        127,
        63,
        r,
        SH110X_WHITE
    );
    display.display();
}

int Menu::getSelectedIndex() const {
    return currentIndex;
}

int Menu::getSelectedValue() const {
    return items[currentIndex].value;
}

void Menu::reset() {
    currentIndex = 0;
}
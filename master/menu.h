#ifndef MENU_H
#define MENU_H

#include <Adafruit_SH110X.h>

class Menu {
  private:
    int r;
    int currentIndex;
    static const int NUM_ITEMS = 3;

    const char* menuItems[NUM_ITEMS];
    unsigned long lastMoveTime;

  public:
    Menu();

    bool handleJoystick(Adafruit_SH1107& display);

    void drawMenu(Adafruit_SH1107& display);
    void executeAction(int index, Adafruit_SH1107& display);

    int getSelectedIndex() const;
    void reset();
};

#endif
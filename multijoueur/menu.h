#ifndef MENU_H
#define MENU_H

#include <Adafruit_SH110X.h>

Menu::Menu() : 
  menuItems[0] = "MULTI";
  menuItems[1] = "SOLO";
  menuItems[2] = "PARAMETRES";
}
class Menu {
  private:
    int r, currentIndex, lastMoveTime;
    static const int NUM_ITEMS = 3;
    
    const char* menuItems[NUM_ITEMS];
    int currentIndex;
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
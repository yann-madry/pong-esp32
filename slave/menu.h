#ifndef MENU_H
#define MENU_H

#include <Adafruit_SH110X.h>

struct MenuItem {
    const char* label;
    int value;
};

class Menu {
private:
    MenuItem* items;
    int numItems;

    int currentIndex;
    int r;
    unsigned long lastMoveTime;
    bool boutonEtaitAppuye = false;

public:
    template <size_t N>
    Menu(MenuItem (&items)[N]) {
        this->items = items;
        this->numItems = N;

        currentIndex = 0;
        r = 4;
        lastMoveTime = 0;
    }

    template <size_t N>
    void setItems(MenuItem (&items)[N]) {
        this->items = items;
        this->numItems = N;
        currentIndex = 0;
    }

    bool handleJoystick();
    void drawMenu(Adafruit_SH1107& display);

    int getSelectedIndex() const;
    int getSelectedValue() const;

    void reset();
};

#endif
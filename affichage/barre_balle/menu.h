#ifndef MENU_H
#define MENU_H

#include <Adafruit_SH110X.h>

class Menu {
  private:
    // static const permet de définir la taille du tableau directement
    static const int NUM_ITEMS = 3; 
    const int JOYSTICK_PIN;
    const int r;
    
    const char* menuItems[NUM_ITEMS];
    int currentIndex;
    unsigned long lastMoveTime;

  public:
    Menu(); // Constructeur
    
    // Retourne true si un clic est validé
    bool handleJoystick(Adafruit_SH1107& display); 
    
    // On passe l'objet de l'écran en paramètre par référence
    void drawMenu(Adafruit_SH1107& display); 
    void executeAction(int index, Adafruit_SH1107& display);
    
    int getSelectedIndex() const { return currentIndex; }
    void reset();
};

#endif
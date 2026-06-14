#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "barre.h"
#include "balle.h"
#include "menu.h"  
  
#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);


enum State {
  MenuAcceuil,
  GameSolo,
  GameMulti,
  GameOver
};

State mode = MenuAcceuil;

Barre barre;
Balle balle;
Menu menu;

void setup() {
  Serial.begin(115200);;
  display.begin(0x3C, true);

  display.display();
  delay(800);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
}

void loop() {
  switch (mode) {
    case MenuAcceuil:
      // 1. On dessine le menu
      menu.drawMenu(display);
      
      // 2. On vérifie si handleJoystick renvoie "true" (clic détecté)
      if (menu.handleJoystick(display)) {
        // Optionnel : un petit effet visuel de validation
        menu.executeAction(menu.getSelectedIndex(), display);
        
        // 3. C'est ici qu'on récupère l'index pour changer de mode !
        int choix = menu.getSelectedIndex();
        if (choix == 0) mode = GameMulti;
        if (choix == 1) mode = GameSolo;
        // index 2 pour les paramètres si tu veux plus tard
      }
      delay(10);
      break;

    case GameSolo:
      display.clearDisplay();
      if (balle.perdu()) gameOver();
      else {
        barre.deplacement(analogRead(A3));
        balle.deplacement(barre);
        if(!digitalRead(BUTTON_C)){
          barre.reset();
          balle.reset();
          menu.reset();
          mode=MenuAcceuil;
        }
        
        else {
          balle.afficher(display);
          barre.afficher(display);
          display.display();
        }
      }      
      break;

    case GameMulti:
      display.clearDisplay();
      display.print("mode multi");
      display.setCursor(0,0);
      display.display();
      if(!digitalRead(BUTTON_C)){
        barre.reset();
        balle.reset();
        menu.reset();
        mode=MenuAcceuil;
      }
      menu.reset();
      break;  

    case GameOver:
      gameOver();
      break;
  }
}

void gameOver(){
  display.clearDisplay();
  display.println("Perdue");
  display.println("Cliquer sur le joystick pour rejouer");
  display.print("Ou C pour retourner au MenuAcceuil");
  display.display();
  display.setCursor(0, 0);

  if (analogRead(A2)==4095){
    mode=GameSolo;
    barre.reset();
    balle.reset();
  } 
  if(!digitalRead(BUTTON_C)){
    mode=MenuAcceuil;
    barre.reset();
    balle.reset();
  }   
}
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
  GameOver,
  PauseGame
};

State mode = MenuAcceuil;

Barre barre;
Balle balle;
Menu menu;

bool joystickAppuyer=false;
int tDep=0;
int vie=3;

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
      menu.drawMenu(display);
      if (menu.handleJoystick(display)) {
        menu.executeAction(menu.getSelectedIndex(), display);
        int choix = menu.getSelectedIndex();
        if (choix == 0) mode = GameMulti;
        if (choix == 1) mode = GameSolo;
      }
      delay(10);
      break;

    case GameSolo:
      display.clearDisplay();
      if (vie==0) gameOver();
      if (balle.perdu()){
        vie--;
        balle.reset();
        barre.reset();
      }
      else {
        barre.deplacement(analogRead(A3));
        balle.deplacement(barre);
        
        if(!digitalRead(BUTTON_C)){
          barre.reset();
          balle.reset();
          menu.reset();
          mode=MenuAcceuil;
        }

        if (analogRead(A2)==4095) {
          if (joystickAppuyer==false) {
            joystickAppuyer=true;
            tDep = millis();
          } 
          else {
            if (millis()-tDep>=1000) { 
              joystickAppuyer = false;
              mode = PauseGame; 
            }
          }
        } 
        else {
          joystickAppuyer = false;
        }
        balle.afficher(display);
        barre.afficher(display);
        display.display();
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

    case PauseGame:
      pauseGame();
      break;
  }
}

void gameOver(){
  display.clearDisplay();
  vie=3;
  display.setCursor(0, 0);
  display.println("Perdue");
  display.println("Cliquer sur le joystick pour rejouer");
  display.print("Ou C pour retourner au Menu");
  display.display();

  delay(250);
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

void pauseGame(){
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Pause");
  display.println("Cliquer sur le joystick pour reprendre");
  display.print("Ou maintener le pour aller au Menu");
  display.display();

  delay(250);
  if (analogRead(A2)==4095){
    mode=GameSolo;
    barre.reset();
    balle.reset();
  }  
}
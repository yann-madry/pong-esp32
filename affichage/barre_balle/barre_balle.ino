#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "barre.h"
#include "balle.h"  
  
#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

enum State {
  Menu,
  GameSolo,
  GameMulti,
  GameOver
};

State mode = Menu;

Barre barre;
Balle balle;

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
    case Menu:
      menu();
      break;

    case GameSolo:
      gameSolo();
      break;

    case GameMulti:
      gameMulti();
      break;  

    case GameOver:
      gameOver();
      break;
  }
}

void menu(){
  display.clearDisplay();
  display.println("Appuyer sur A pour jouer en solo");
  display.print("Appuyer sur B pour jouer en multi");
  if(!digitalRead(BUTTON_A)) mode=GameSolo;
  if(!digitalRead(BUTTON_B)) mode=GameMulti;
  
  display.display();
  display.setCursor(0,0);
}

void gameSolo() {
  display.clearDisplay();
  barre.deplacement(analogRead(A3));
  balle.deplacement(barre);
  if (balle.perdu()) gameOver();
  if(!digitalRead(BUTTON_C)){
    barre.reset();
    balle.reset();
    mode=Menu;
  }
  else {
    balle.afficher(display);
    barre.afficher(display);
    display.display();
  }    
}

void gameMulti(){
  display.clearDisplay();
  display.print("mode multi");
  display.setCursor(0,0);
  display.display();
  if(!digitalRead(BUTTON_C)){
    barre.reset();
    balle.reset();
    mode=Menu;
  }
}

void gameOver(){
  display.clearDisplay();
  display.println("Perdue");
  display.println("Cliquer sur le joystick pour rejouer");
  display.print("Ou C pour retourner au menu");
  display.display();
  display.setCursor(0, 0);

  if (analogRead(A2)==4095){
    mode=GameSolo;
    barre.reset();
    balle.reset();
  } 
  if(!digitalRead(BUTTON_C)){
    mode=Menu;
    barre.reset();
    balle.reset();
  }   
}
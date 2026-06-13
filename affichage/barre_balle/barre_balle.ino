#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "barre.h"
#include "balle.h"

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

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
}

enum State {
  MENU,
  GAME,
  GAMEOVER
};

State mode = MENU;

Barre barre;
Balle balle;

void loop() {
  switch (mode) {
    case MENU:
      menu();
      break;

    case GAME:
      game();
      break;

    case GAMEOVER:
      gameOver();
      break;
  }
}

void menu(){
  display.clearDisplay();
  display.print("Appuyer sur le joystick pour jouer");
  if (analogRead(A2)==4095) mode=GAME;
  display.display();
  display.setCursor(0,0);
}

void game() {
  display.clearDisplay();
  barre.deplacement(analogRead(A3));
  balle.deplacement(barre);
  if (balle.perdu()) gameOver();
  else {
    balle.afficher(display);
    barre.afficher(display);
    display.display();
  }    
}

void gameOver(){
  display.clearDisplay();
  display.print("Perdue");
  display.display();
  delay(1000);
  barre.reset();
  balle.reset();
  display.setCursor(0, 0);
  mode=MENU;
  display.clearDisplay();
}
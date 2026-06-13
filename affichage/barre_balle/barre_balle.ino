#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "barre.h"
#include "balle.h"

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

void setup() {
  Serial.begin(115200);

  Serial.println("128x64 OLED FeatherWing test");
  display.begin(0x3C, true);

  Serial.println("OLED begun");

  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
}

Barre barre;
Balle balle;

void loop() {
  display.clearDisplay();

  barre.deplacement(analogRead(A3));
  balle.deplacement(barre);
  if (balle.perdu()){
    display.clearDisplay();
    display.print("Perdue");
    display.display();
    delay(1000);
    barre.reset();
    balle.reset();
    display.setCursor(0, 0);
  }
  else {
    balle.afficher(display);
    barre.afficher(display);
    display.display();
  }  
}
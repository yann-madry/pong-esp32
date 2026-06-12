#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "barre.h"

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

void loop() {
  
  display.clearDisplay();
  
  int sensorValue2 = analogRead(A3);
  if (sensorValue2<1000 && 0<barre.x){
    barre.x-=barre.speed;
  }
  // faire attention a limite avec vitesse deplacement
  if (sensorValue2>2000 && barre.x<128-30){
    barre.x+=barre.speed;
  }

  display.fillCircle(64, 32, 2, 1);
  display.fillRect(barre.x, barre.y, barre.width, barre.height, 1);
  delay(5);
  yield();
  display.display();
  delay(5);  
}

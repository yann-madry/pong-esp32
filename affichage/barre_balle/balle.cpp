#include "balle.h"

Balle::Balle() {
  x = 64;
  y = 32;
  r = 2;
  sensX = 0;
  sensY = 1;
  speed = 1;
}

void Balle::deplacement(const Barre& barre) {
  if (x+2==128 || x==2){
    sensX=-sensX;
  }

  if (y==2 || (y+r+barre.height==64 && (barre.x<=x && x<=barre.x+barre.width))){
    sensY=-sensY;
  }

  x+=sensX*speed;
  y+=sensY*speed;  
}

void Balle::afficher(Adafruit_GFX& display){
  display.fillCircle(x, y, r, 1);
}
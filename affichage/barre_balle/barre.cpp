#include "barre.h"

Barre::Barre() {
  x = 53;
  y = 60;
  width = 30;
  height = 4;
  speed = 2;
}

void Barre::deplacement(int y) {
  if (y < 1000 && x>0) {
    x -= speed;
  }

  if (y > 2000 && x<128-width-speed) {
    x += speed;
  }  
}

void Barre::afficher(Adafruit_GFX& display){
  display.fillRect(x, y, width, height, 1);
}
#include "barre.h"

Barre::Barre() {
  x = 53;
  y = 60;
  width = 30;
  height = 4;
  speed = 2;
}

void Barre::deplacement(int y) {
  if (y < 1000 && x > 0) {
    x -= speed;
  }

  if (y > 2000 && x < 98) {
    x += speed;
  }
}
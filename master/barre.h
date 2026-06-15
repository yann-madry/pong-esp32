#ifndef BARRE_H
#define BARRE_H

#include <Adafruit_GFX.h>

struct Barre {
  int x, y, width, height, speed;

  Barre();
  void deplacement(int y);
  void afficher(Adafruit_GFX& display);
  void suivreBalle(int balleX);
  void reset();
};

#endif
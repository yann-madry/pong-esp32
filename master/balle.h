#ifndef BALLE_H
#define BALLE_H

#include <Adafruit_GFX.h>
#include "barre.h"

struct Balle {
  int x, y, r, sensX, sensY, speed, maxSpeed;
  bool vientDeToucher;

  Balle();
  void deplacement(const Barre& barre);
  void afficher(Adafruit_GFX& display);
  bool toucheBarre(const Barre& b);
  bool perdu();
  void reset();
};

#endif
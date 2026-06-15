#include "balle.h"

Balle::Balle() {
  x=64;
  y=32;
  r=2;
  sensX=0;
  sensY=1;
  speed=1;
  maxSpeed=2;
}

void Balle::deplacement(const Barre& barre) {

  int nextX=x+sensX*speed;
  int nextY=y+sensY*speed;

  if (nextX-r<=0||nextX+r>=128) {
    sensX=-sensX;
    if (speed<maxSpeed) speed++;
  }

  if (nextY-r<=0 || (nextY+r>=64-barre.height && x>=barre.x && x<=barre.x+barre.width)) {
    sensY=-sensY;
    if (sensX==0) sensX=-1;
    if (speed<maxSpeed) speed++;
  }

  x=x+sensX*speed;
  y=y+sensY*speed;
}

void Balle::afficher(Adafruit_GFX& display){
  display.fillCircle(x, y, r, 1);
}

bool Balle::perdu(){
  if (y+sensY*speed+r>=64) return true;
  return false;
}

void Balle::reset(){
  x=64;
  y=32;
  r=2;
  sensX=0;
  sensY=1;
  speed=1;
  maxSpeed=2;
}
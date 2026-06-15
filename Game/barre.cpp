#include "barre.h"

Barre::Barre() {
  x=49;
  y=60;
  width=30;
  height=4;
  speed=2;
}

void Barre::deplacement(int v) {
  if (v<1000 && x>0) {
    x-=speed;
  }

  if (v>2000 && x<128-width-speed) {
    x+=speed;
  }

  if (v<=128-width && 0<=v-width/2) {
    x=v-width/2;
  }
}

void Barre::afficher(Adafruit_GFX& display){
  display.fillRoundRect(x, y, width, height, 4, 1);
}

void Barre::reset(){
  x=49;
  y=60;
  width=30;
  height=4;
  speed=2;
}
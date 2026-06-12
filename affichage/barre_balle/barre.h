#ifndef BARRE_H
#define BARRE_H

struct Barre {
  int x, y, width, height, speed;

  Barre();
  void deplacement(int y);
};

#endif
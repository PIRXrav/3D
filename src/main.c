#include "geo.h"
#include "window.h"
#include <stdio.h>

void user_loop(struct hwindow *hw) {
  static int c = 0;
  c++;
  for (unsigned int i = 0; i < HW_GetX(hw); i++)
    for (unsigned int j = 0; j < HW_GetY(hw); j++)
      HW_SetPx(hw, i, j, (int)(255 * i / HW_GetX(hw)),
               (int)(255 * j / HW_GetY(hw)),
               (int)(255 * (i + j) / (HW_GetX(hw) + HW_GetY(hw))) + c);
}

int main(int argc, char **argv) {
  // HW_start("Test", 900, 600, user_loop);
  VECT_test();
  return 0;
}

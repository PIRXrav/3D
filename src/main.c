#include "geo.h"
#include "render.h"
#include "window.h"
#include <math.h>
#include <stdio.h>

const unsigned int X = 400;
const unsigned int Y = 300;
struct Render *rd;

void user_loop(struct hwindow *hw) {
  /*
  static int c = 0;
  c++;
  for (unsigned int i = 0; i < HW_GetX(hw); i++)
    for (unsigned int j = 0; j < HW_GetY(hw); j++)
      HW_SetPx(hw, i, j, (int)(255 * i / HW_GetX(hw)),
               (int)(255 * j / HW_GetY(hw)),
               (int)(255 * (i + j) / (HW_GetX(hw) + HW_GetY(hw))) + c);
               */
  static struct Vector ray;

  static double angle = 0;

  static struct Vector barycentre = {-5, -5, 10};

  static struct Vector cam_pos;
  VECT_Cpy(&cam_pos, &rd->cam_pos);
  static struct Vector cam_vect = {0, 0, -10};
  static struct Vector cam_up_world = {0, 1, 0};

  angle += 0.01;

  /* pos */
  cam_pos.x = cos(angle) * 30 + rd->mesh->a.x;
  cam_pos.y = sin(angle) * 30 + rd->mesh->a.y;
  // cam_pos.z = 1;
  // cam_pos.x / 10 + 1;

  // make
  // VECT_Sub(&cam_vect, &cam_pos, &barycentre);

  RD_SetCam(rd, &cam_pos, &cam_vect, &cam_up_world);

  for (unsigned int y = 0; y < HW_GetY(hw); y++) {
    for (unsigned int x = 0; x < HW_GetX(hw); x++) {
      RD_CalcRayDir(rd, x, y, &ray);
      if (RD_RayTraceOnce(rd, &ray)) {
        // printf("##");
        HW_SetPx(hw, x, y, 0x22, 0xBA, 0xBA);
      } else {
        HW_SetPx(hw, x, y, 0, 0, 0);
        // printf("--");
      }
    }
    // printf("\n");
  }
  // RD_Print(rd);
}

int main(int argc, char **argv) {
  // HW_start("Test", 900, 600, user_loop);
  // VECT_test();
  rd = RD_Init(X, Y);
  HW_start("Test", X, Y, user_loop);

  return 0;
}

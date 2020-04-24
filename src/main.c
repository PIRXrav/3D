#include "color.h"
#include "geo.h"
#include "parsers/parser.h"
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
      HW_SetPx(hw, i, j,
               CL_rgb(255 * i / HW_GetX(hw), 255 * j / HW_GetY(hw),
                      255 * (i + j) / (HW_GetX(hw) + HW_GetY(hw)) + c));
  return;
  */
  static struct Vector ray;
  static struct Vector hit; // Hit point

  static double angle = 0;
  static struct Vector barycentre = {.25, .25, .25};

  rd->fov_rad = 1.05;
  static struct Vector cam_vect = {-1, -1, -1};
  static struct Vector cam_up_world = {0, 0, 1};

  angle += 0.05;

  /* pos */
  rd->cam_pos.x = cos(angle) * 5;     // + rd->mesh->vertices[0].x;
  rd->cam_pos.y = sin(angle) * 5;     // + rd->mesh->vertices[0].y;
  rd->cam_pos.z = sin(angle / 2) * 5; // + rd->mesh->vertices[0].y;
  // make
  VECT_Sub(&cam_vect, &rd->cam_pos, &barycentre);
  RD_SetCam(rd, &rd->cam_pos, &cam_vect, &cam_up_world);

  for (unsigned int y = 0; y < HW_GetY(hw); y++) {
    for (unsigned int x = 0; x < HW_GetX(hw); x++) {
      RD_CalcRayDir(rd, x, y, &ray);
      HW_SetPx(hw, x, y, RD_RayTraceOnRD(rd, &ray, &hit));
    }
    // printf("\n");
  }
  // RD_Print(rd);
}

int main(void) {
  // HW_start("Test", 900, 600, user_loop);
  // VECT_test();
  rd = RD_Init(X, Y);
  RD_Print(rd);

  unsigned nbMeshes;
  struct Mesh **meshes = PARSER_Load("data/icosphere-20.obj", &nbMeshes);
  printf("Loaded mesh !\n");
  for (unsigned i = 0; i < nbMeshes; i++)
    RD_AddMesh(rd, meshes[i]);

  // RD_Print(rd);

  HW_start("Test", X, Y, user_loop);

  return 0;
}

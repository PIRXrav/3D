#include "color.h"
#include "geo.h"
#include "parsers/parser.h"
#include "raster.h"
#include "render.h"
#include "window.h"
#include <math.h>
#include <stdio.h>

const unsigned int X = 400;
const unsigned int Y = 400;
struct Render *rd;

void mouse_event(const struct event *event) {
  if (event->data.mouse.dx > 0 || event->data.mouse.dy > 0) {
    struct Mesh *mesh = NULL;
    struct MeshFace *face = NULL;
    struct Vector ray, collisionPoint;
    RD_CalcRayDir(rd, event->data.mouse.x, event->data.mouse.y, &ray);
    RD_RayCastOnRD(rd, &ray, &collisionPoint, &mesh, &face);

    printf("Mesh : %p, face : %p\n", mesh, face);
    rd->highlightedMesh = mesh;
    rd->highlightedFace = face;
  }
}

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

  static struct Vector cam_vect = {0, 0, -1};
  static struct Vector cam_up_world = {0, 0, 1};

  angle += 0.05;

  /* pos */
  rd->cam_pos.x = cos(angle) * 5;     // + rd->mesh->vertices[0].x;
  rd->cam_pos.y = sin(angle) * 5;     // + rd->mesh->vertices[0].y;
  rd->cam_pos.z = sin(angle * 2) * 5; // + rd->mesh->vertices[0].y;
                                      // make
  VECT_Sub(&cam_vect, &rd->cam_pos, &barycentre);
  RD_SetCam(rd, &rd->cam_pos, &cam_vect, &cam_up_world);

  // Raytracing
  for (unsigned int y = 0; y < HW_GetY(hw); y++) {
    for (unsigned int x = 0; x < HW_GetX(hw); x++) {
      RD_CalcRayDir(rd, x, y, &ray);
      HW_SetPx(hw, x, y, RD_RayTraceOnRD(rd, &ray, &hit));
    }
  }

  // Projection
  calc_projection(rd);
  for (uint32_t y = 0; y < HW_GetY(hw); y++) {
    for (uint32_t x = 0; x < HW_GetX(hw); x++) {
      color c = RASTER_GetPixelxy(rd->raster, x, y);
      if (c.raw != CL_BLACK.raw)
        HW_SetPx(hw, x, y, c);
    }
  }
  // RD_Print(rd);
}

int main(void) {
  // VECT_test();
  rd = RD_Init(X, Y);
  rd->highlightedMesh = 0;
  RD_Print(rd);

  unsigned nbMeshes;
  struct Mesh **meshes = PARSER_Load("data/cube.obj", &nbMeshes);
  printf("Loaded mesh !\n");
  for (unsigned i = 0; i < nbMeshes; i++)
    RD_AddMesh(rd, meshes[i]);

  // RD_Print(rd);

  struct hwindow *fenetre = HW_Init("Rendu 3D", X, Y);

  HW_SetCallback(fenetre, EVENT_MOUSE, mouse_event);

  HW_Loop(fenetre, user_loop);

  HW_Close(fenetre);

  return 0;
}

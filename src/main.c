#include "color.h"
#include "geo.h"
#include "parsers/parser.h"
#include "raster.h"
#include "render.h"
#include "window.h"
#include <math.h>
#include <stdio.h>

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

void user_loop(unsigned int cpt) {
  printf("%d\n", cpt);

  static double angle = 0;
  static struct Vector barycentre = {.25, .25, .25};

  static struct Vector cam_vect = {0, 0, -1};
  static struct Vector cam_up_world = {0, 0, 1};

  /* pos */
  angle += 0.05;
  rd->cam_pos.x = cos(angle) * 5;     // + rd->mesh->vertices[0].x;
  rd->cam_pos.y = sin(angle) * 5;     // + rd->mesh->vertices[0].y;
  rd->cam_pos.z = sin(angle * 2) * 5; // + rd->mesh->vertices[0].y;
  VECT_Sub(&cam_vect, &rd->cam_pos, &barycentre);
  RD_SetCam(rd, &rd->cam_pos, &cam_vect, &cam_up_world);

  RD_DrawRaytracing(rd);
  // RD_DrawFill(rd);
  RD_DrawWirefram(rd);
  RD_DrawVertices(rd);
  RD_DrawAxis(rd);
}

int main(void) {
  // VECT_test();
  rd = RD_Init(400, 400);
  rd->highlightedMesh = 0;
  RD_Print(rd);

  unsigned nbMeshes;
  struct Mesh **meshes = PARSER_Load("data/cube.obj", &nbMeshes);
  printf("Loaded mesh !\n");
  for (unsigned i = 0; i < nbMeshes; i++)
    RD_AddMesh(rd, meshes[i]);

  // RD_Print(rd);

  struct hwindow *fenetre = HW_Init("Rendu 3D", rd->raster);
  HW_SetCallback(fenetre, EVENT_MOUSE, mouse_event);
  HW_Loop(fenetre, user_loop);
  HW_Close(fenetre);

  return 0;
}

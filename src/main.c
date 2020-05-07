#include "color.h"
#include "geo.h"
#include "parsers/parser.h"
#include "raster.h"
#include "render.h"
#include "terminal.h"
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
  // printf("%d\n", cpt);

  static double angle = 0;
  struct Vector cam_pos = {0, 0, 0};
  static struct Vector barycentre = {0, 0, 0};
  static struct Vector cam_vect = {0, 0, -1};

  /* pos */
  angle += 0.005;
  cam_pos.x = cos(angle) * 10;
  cam_pos.y = cos(angle / 4) * 10;
  cam_pos.z = sin(angle) * 10;
  VECT_Sub(&cam_vect, &rd->cam_pos, &barycentre);
  RD_SetCam(rd, &cam_pos, &cam_vect, NULL);

  // RD_DrawRaytracing(rd);
  RD_DrawFill(rd);
  RD_DrawWireframe(rd);
  // RD_DrawVertices(rd);
  RD_DrawAxis(rd);
}

void mainFenetre() {

  struct hwindow *fenetre = HW_Init("Rendu 3D", rd->raster);
  HW_SetCallback(fenetre, EVENT_MOUSE, mouse_event);
  HW_Loop(fenetre, user_loop);
  HW_Close(fenetre);
}

void mainTerm() {

  struct tty *tty = TTY_Init(rd->raster);
  TTY_Loop(tty, user_loop);
  TTY_Close(tty);
}

int main() {
  rd = RD_Init(900, 900);
  rd->highlightedMesh = 0;
  RD_Print(rd);

  unsigned nbMeshes;
  struct Mesh **meshes = PARSER_Load("data/extern/cow.obj", &nbMeshes);
  printf("Loaded mesh !\n");
  for (unsigned i = 0; i < nbMeshes; i++)
    RD_AddMesh(rd, meshes[i]);

  mainFenetre();
  // mainTerm();

  return 0;
}

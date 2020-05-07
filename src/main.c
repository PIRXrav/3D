#include "color.h"
#include "geo.h"
#include "parsers/parser.h"
#include "raster.h"
#include "render.h"
#include "terminal.h"
#include "window.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MODE_TERMINAL 0
#define MODE_SDL2 1

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
  struct Vector cam_pos;
  static struct Vector barycentre = {.25, .25, .25};
  static struct Vector cam_vect = {0, 0, -1};

  /* pos */
  angle += 0.05;
  cam_pos.x = cos(angle) * 3;
  cam_pos.y = sin(angle) * 3;
  cam_pos.z = sin(angle * 2) * 3;
  VECT_Sub(&cam_vect, &rd->cam_pos, &barycentre);
  RD_SetCam(rd, &cam_pos, &cam_vect, NULL);

  RD_DrawRaytracing(rd);
  // RD_DrawFill(rd);
  // RD_DrawWireframe(rd);
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

int main(int argc, char **argv) {

  char *modele = "data/cube.obj";
  unsigned w = 400;
  unsigned h = 400;
  int mode = MODE_SDL2;

  if (argc >= 2) {
    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
      printf("Usage %s [graphic|terminal [modelFile]]\n", argv[0]);
      return 0;
    }

    if (!strcmp(argv[1], "graphic"))
      mode = MODE_SDL2;
    else if (!strcmp(argv[1], "terminal")) {
      TTY_QuerySize(&w, &h);
      mode = MODE_TERMINAL;
    } else {
      printf("Unknown display mode %s\n", argv[1]);
      return 1;
    }
  }
  if (argc >= 3) {
    modele = argv[2];
  }

  rd = RD_Init(w, h);

  unsigned nbMeshes;
  printf("Loading %s...\n", modele);
  struct Mesh **meshes = PARSER_Load(modele, &nbMeshes);
  printf("Loaded %u meshs !\n", nbMeshes);
  for (unsigned i = 0; i < nbMeshes; i++)
    RD_AddMesh(rd, meshes[i]);

  switch (mode) {
  case MODE_SDL2:
    mainFenetre();
    break;
  case MODE_TERMINAL:
    mainTerm();
    break;
  }

  return 0;
}

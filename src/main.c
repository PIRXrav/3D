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

    // printf("Mesh : %p, face : %p\n", mesh, face);
    rd->highlightedMesh = mesh;
    rd->highlightedFace = face;
  }
}

void user_loop(unsigned int cpt) {
  static double angle = 7.1;
  struct Vector *barycentre = &rd->meshs[0]->box.center;
  double d = 0.5 * sqrt(VECT_DistanceSquare(&rd->meshs[0]->box.min,
                                            &rd->meshs[0]->box.max));

  static struct Vector cam_vect;
  static struct Vector cam_pos;
  // VECT_Print(barycentre);
  // printf("d : %f \n", d);
  /* pos */

  angle += 0.05;
  cam_pos.x = cos(angle) * d;
  cam_pos.y = cos(angle / 2) * d;
  cam_pos.z = sin(angle) * d;
  VECT_Sub(&cam_vect, &cam_pos, barycentre);
  RD_SetCam(rd, &cam_pos, &cam_vect, NULL);
  fflush(stdout);

  // Mise a jour des objets
  RD_CalcProjectionVertices(rd); // Calcul des projections
  RD_CalcZbuffer(rd);            // Calcul du Z buffer
  RD_CalcNormales(rd);

  // Rendu
  // RD_DrawRaytracing(rd);
  RD_DrawFill(rd);
  RD_DrawZbuffer(rd);
  RD_DrawGbuffer(rd);
  Vector lum = {1, 1, 1};
  VECT_Normalise(&lum);
  RD_DrawFbufferWithLum(rd, &lum, CL_CHARTREUSE);

  // RD_DrawWireframe(rd);
  // RD_DrawVertices(rd);
  // RD_DrawNormales(rd);
  RD_DrawAxis(rd);

  // Filtres vidéo
  // RASTER_Negate(rd->raster);
  // printf("\n================= CONFIG ===============\n");
  // RD_Print(rd);
  // getchar();
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

  char *modele = "data/extern/teapot.obj";
  unsigned w = 400;
  unsigned h = 400;
  int mode = MODE_SDL2;

  char *helpstr =
      "\033[31mNAME\033[m                                              \n"
      "      3D renderer                                               \n"
      "\033[31mSYNOPSIS\033[m                                          \n"
      "      \033[31m%s\033[m [\033[32mOPTIONS\033[m]\n"
      "      \033[31m%s\033[m -g -f data/extern/elephant.obj -x=900 -y=600\n"
      "\033[31mDESCRIPTION\033[m                                       \n"
      "      \033[31m-h\033[m        display help menu                 \n"
      "      \033[31m-g\033[m        set graphic output                \n"
      "      \033[31m-t\033[m        set terminal output               \n"
      "      \033[31m-f\033[m \033[32mFILE\033[m    set 3D file        \n"
      "      \033[31m-x\033[m=\033[32mSIZE\033[m    set windows width  \n"
      "      \033[31m-y\033[m=\033[32mSIZE\033[m    set windows height \n"
      "                                                                \n";

  for (int optind = 1; optind < argc; optind++) {
    switch (argv[optind][1]) {
    case 'g':
      mode = MODE_SDL2;
      break;
    case 't':
      mode = MODE_TERMINAL;
      break;
    case 'f':
      modele = argv[++optind];
      break;
    case 'x':
      sscanf(argv[optind], "-x=%d", &w);
      break;
    case 'y':
      sscanf(argv[optind], "-y=%d", &h);
      break;
    case 'h':
      printf(helpstr, argv[0], argv[0]);
      exit(EXIT_SUCCESS);
      break;
    default:
      fprintf(stderr, "Usage: %s -h\n", argv[0]);
      exit(EXIT_FAILURE);
    }
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

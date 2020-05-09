/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "render.h"
#include "color.h"
#include "geo.h"
#include "mesh.h"
#include "raster.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static void calcProjectionVertex3(struct Render *rd, struct MeshVertex *p);

/*
 * Calcule d'une raie
 */
void RD_CalcRayDir(struct Render *rd, unsigned int sx, unsigned int sy,
                   struct Vector *ray) {
  ray->x = rd->cam_u.x * (double)sx - rd->cam_v.x * (double)sy + rd->cam_wp.x;
  ray->y = rd->cam_u.y * (double)sx - rd->cam_v.y * (double)sy + rd->cam_wp.y;
  ray->z = rd->cam_u.z * (double)sx - rd->cam_v.z * (double)sy + rd->cam_wp.z;
  // VECT_Normalise(ray);
  // Vec3f ray_dir = normalize(x * u + y * (-v) + w_p);
}

/*
 * Retourne l'intersection de la ray, sa couleur et la distance de collsion
 * return bool : etat du succes. 1 si collision
 * vector x       : POint de croisement                     [OUT]
 * distanceSquare : la distance maximale (dernier valide).  [IN/OUT]
 * face           : Pointeur sur la plus proche face pour l'instant [OUT]
 *
 * La couleur et la distance sont mit à jour si collision dans ce mesh
 */
static bool RD_RayTraceOnMesh(const struct Mesh *mesh,
                              const struct Vector *cam_pos,
                              const struct Vector *cam_ray, struct Vector *x,
                              double *distance, struct MeshFace **face) {
  static bool hit;
  static double d;
  static struct MeshFace *mf;

  hit = false;
  for (unsigned int i_face = 0; i_face < MESH_GetNbFace(mesh); i_face++) {
    mf = MESH_GetFace(mesh, i_face);
    if (RayIntersectsTriangle(cam_pos, cam_ray, &mf->p0->world, &mf->p1->world,
                              &mf->p2->world, x)) {
      d = VECT_DistanceSquare(cam_pos, x);
      if (d < *distance) {
        *face = MESH_GetFace(mesh, i_face);
        *distance = d;
        hit = true;
      }
    }
  }
  return hit;
}

/*
 * Intersection d'un rayon avec toutes les meshs, on retourne le point, la
 * face et la mesh en collision
 */
extern bool RD_RayCastOnRD(const struct Render *rd, const struct Vector *ray,
                           struct Vector *x, struct Mesh **mesh,
                           struct MeshFace **face) {
  double distance = 99999999; // Max dist
  int hit = false;
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    if (RD_RayTraceOnMesh(rd->meshs[i_mesh], &rd->cam_pos, ray, x, &distance,
                          face)) {
      hit = true;
      *mesh = rd->meshs[i_mesh];
    }
  }
  return hit;
}

/*
 * Intersection avec tout les meshs
 * On retourne le point de croisement x
 */
extern color RD_RayTraceOnRD(const struct Render *rd, const struct Vector *ray,
                             struct Vector *x) {

  struct Mesh *mesh = NULL;
  struct MeshFace *face = NULL;
  if (RD_RayCastOnRD(rd, ray, x, &mesh, &face)) {
    if (mesh == rd->highlightedMesh && face == rd->highlightedFace)
      return CL_Negate(face->color);
    return face->color;
  }
  return CL_BLACK; // background color
}

/*
 * https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function
 */
void RD_SetCam(struct Render *rd, const struct Vector *cam_pos,
               const struct Vector *cam_forward,
               const struct Vector *cam_up_world) {

  // Mise a jout des variables maitresses
  if (cam_pos != NULL)
    VECT_Cpy(&rd->cam_pos, cam_pos);
  if (cam_forward != NULL)
    VECT_Cpy(&rd->cam_forward, cam_forward);
  if (cam_up_world != NULL)
    VECT_Cpy(&rd->cam_up_world, cam_up_world);

  // forward
  VECT_Cpy(&rd->cam_w, &rd->cam_forward);
  VECT_Normalise(&rd->cam_w);
  // right
  VECT_CrossProduct(&rd->cam_u, &rd->cam_up_world, &rd->cam_forward);
  VECT_Normalise(&rd->cam_u);
  // up
  VECT_CrossProduct(&rd->cam_v, &rd->cam_w, &rd->cam_u);
  VECT_Normalise(&rd->cam_v);

  /* Précalcul w' */
  struct Vector un, vn, wn;
  VECT_MultSca(&un, &rd->cam_u, -(double)rd->raster->xmax / 2);
  VECT_MultSca(&vn, &rd->cam_v, (double)rd->raster->ymax / 2);
  VECT_MultSca(&wn, &rd->cam_w,
               ((double)rd->raster->ymax / 2) / tan(rd->fov_rad * 0.5));
  VECT_Add(&rd->cam_wp, &un, &vn);
  VECT_Sub(&rd->cam_wp, &rd->cam_wp, &wn);

  /* Précalcul projection */
  // Matrice world to camera
  rd->tx = -VECT_DotProduct(&rd->cam_u, &rd->cam_pos);
  rd->ty = -VECT_DotProduct(&rd->cam_v, &rd->cam_pos);
  rd->tz = +VECT_DotProduct(&rd->cam_w, &rd->cam_pos);
}

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 * Initialisation
 */
extern struct Render *RD_Init(unsigned int xmax, unsigned int ymax) {
  struct Render *ret = malloc(sizeof(struct Render));

  // Allocations
  ret->nb_meshs = 0;
  ret->meshs = malloc(sizeof(struct mesh *) * ret->nb_meshs);
  assert(ret->meshs);
  ret->raster = MATRIX_Init(xmax, ymax, sizeof(color), "color");
  ret->zbuffer = MATRIX_Init(xmax, ymax, sizeof(double), "double");

  // Repere
  VECT_Cpy(&ret->p0.world, &VECT_0);
  VECT_Cpy(&ret->px.world, &VECT_X);
  VECT_Cpy(&ret->py.world, &VECT_Y);
  VECT_Cpy(&ret->pz.world, &VECT_Z);

  ret->highlightedMesh = NULL;
  ret->highlightedFace = NULL;

  // cam
  ret->fov_rad = 1.0;
  struct Vector cam_pos = {100000, 100000, 100000};
  struct Vector cam_forward = {100000, 100000, 100000};
  struct Vector cam_up = {0, 1, 0};
  RD_SetCam(ret, &cam_pos, &cam_forward, &cam_up);

  // Projection values
  ret->s = 1 / (tan(ret->fov_rad / 2));
  ret->scalex = (double)ret->raster->ymax / (double)ret->raster->xmax;
  ret->scaley = 1;
  return ret;
}

/* Ajoute une mesh au render, aucune copie n'est faite */
extern void RD_AddMesh(struct Render *rd, struct Mesh *m) {
  rd->nb_meshs++;
  rd->meshs = realloc(rd->meshs, sizeof(struct mesh *) * rd->nb_meshs);
  rd->meshs[rd->nb_meshs - 1] = m;
}

void RD_Print(struct Render *rd) {
  printf(" ============ RENDER =========== \n");
  printf("\nCAM VECT : ");
  VECT_Print(&rd->cam_u);
  VECT_Print(&rd->cam_v);
  VECT_Print(&rd->cam_w);
  printf("\nCAM POS: ");
  VECT_Print(&rd->cam_pos);
  printf("\n");
  printf("NB MESHS: %d\n", rd->nb_meshs);
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    // MESH_Print(rd->meshs[i_mesh]);
    printf("\t->[F:%ld V:%ld]", MESH_GetNbFace(rd->meshs[i_mesh]),
           MESH_GetNbVertice(rd->meshs[i_mesh]));
    VECT_Print(&rd->meshs[i_mesh]->box.center);
  }
  printf("SIZE : x = %d, y = %d\n", rd->raster->xmax, rd->raster->ymax);
  printf("\n");
}

extern void RD_CalcProjectionVertices(struct Render *rd) {
  Mesh *mesh;
  // Vertices
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    for (unsigned int i_v = 0; i_v < MESH_GetNbVertice(mesh); i_v++) {
      calcProjectionVertex3(rd, MESH_GetVertex(mesh, i_v));
    }
  }
  // Repere
  calcProjectionVertex3(rd, &rd->p0);
  calcProjectionVertex3(rd, &rd->px);
  calcProjectionVertex3(rd, &rd->py);
  calcProjectionVertex3(rd, &rd->pz);
}

extern void RD_DrawRaytracing(struct Render *rd) {
  // Raytracing
  static struct Vector ray;
  static struct Vector hit; // Hit point
  for (unsigned int y = 0; y < rd->raster->ymax; y++) {
    for (unsigned int x = 0; x < rd->raster->xmax; x++) {
      RD_CalcRayDir(rd, x, y, &ray);
      RASTER_DrawPixelxy(rd->raster, x, y, RD_RayTraceOnRD(rd, &ray, &hit));
    }
  }
}

extern void RD_DrawWireframe(struct Render *rd) {
  Mesh *mesh;
  MeshFace *f;
  // Wirefram
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    for (unsigned int i_f = 0; i_f < MESH_GetNbFace(mesh); i_f++) {
      f = MESH_GetFace(mesh, i_f);
      RASTER_DrawTriangle(rd->raster, &f->p0->screen, &f->p1->screen,
                          &f->p2->screen, CL_ORANGE);
    }
  }
}

int isDoubleGreater(void *a, void *b) {
  double aa = *(double *)a;
  double bb = *(double *)b;
  return aa < bb;
}

int isDoubleLower(void *a, void *b) {
  double aa = *(double *)a;
  double bb = *(double *)b;
  if (bb < 0)
    return 0;
  if (aa < 0)
    return 1;
  return bb < aa;
}

void RD_DrawZbuffer(struct Render *rd) {
  double maxz = *(double *)MATRIX_Max(rd->zbuffer, isDoubleGreater);
  double minz = *(double *)MATRIX_Max(rd->zbuffer, isDoubleLower);
  // printf("maxz : %f, minz : %f\n", maxz, minz);
  for (size_t x = 0; x < rd->raster->xmax; x++) {
    for (size_t y = 0; y < rd->raster->ymax; y++) {
      double z = *(double *)MATRIX_Edit(rd->zbuffer, x, y);
      if (z >= 0) {
        double coef = (z - minz) / (maxz - minz);
        RASTER_DrawPixelxy(rd->raster, x, y, CL_Mix(CL_WHITE, CL_BLACK, coef));
      }
    }
  }
}
/*
 *
 * https://codeplea.com/triangular-interpolation?fbclid=IwAR38TFpipmfuQ5bM2P0Y07eym1ZHlt7-ZlcZAnEIb7EeOYU3uJzqWxuK0Ws
 */
void callbackWriteZbuffer(uint32_t x, uint32_t y, void **args) {

  Vector *p1 = &((struct MeshFace *)args[1])->p0->sc;
  Vector *p2 = &((struct MeshFace *)args[1])->p1->sc;
  Vector *p3 = &((struct MeshFace *)args[1])->p2->sc;
  Matrix *m = (Matrix *)args[0];

  if (x > m->xmax || y > m->ymax)
    return;

  // to INT
  p1->x = (double)(int)(p1->x);
  p1->y = (double)(int)(p1->y);

  p2->x = (double)(int)(p2->x);
  p2->y = (double)(int)(p2->y);

  p3->x = (double)(int)(p3->x);
  p3->y = (double)(int)(p3->y);

  // Barycentre
  double denum =
      (p2->y - p3->y) * (p1->x - p3->x) + (p3->x - p2->x) * (p1->y - p3->y);
  if (denum == 0) // Face invisible car dans le mauvais plan.
    return;

  double w1 =
      ((p2->y - p3->y) * (x - p3->x) + (p3->x - p2->x) * (y - p3->y)) / denum;
  double w2 =
      ((p3->y - p1->y) * (x - p3->x) + (p1->x - p3->x) * (y - p3->y)) / denum;
  double w3 = 1 - w1 - w2;

  w1 = w1 > 0 ? w1 : 0;
  w2 = w2 > 0 ? w2 : 0;
  w3 = w3 > 0 ? w3 : 0;

  double sum = (w1 + w2 + w3);
  w1 /= sum;
  w2 /= sum;
  w3 /= sum;

  double z4 = w1 * p1->z + w2 * p2->z + w3 * p3->z;

  if (z4 > 1000000000 || z4 < 0 || isnan(z4)) {
    printf("Z = %f\n", z4);
    printf("denum = %f\n", denum);
    printf("w1 = %f, w2 = %f, w3 = %f\n", w1, w2, w3);
    VECT_Print(p1);
    VECT_Print(p2);
    VECT_Print(p3);
    printf("\n");
    assert(0);
  }

  if (*(double *)MATRIX_Edit(m, x, y) > z4 ||
      *(double *)MATRIX_Edit(m, x, y) < 0.f)
    *(double *)MATRIX_Edit(m, x, y) = z4;
}

extern void RD_CalcZbuffer(struct Render *rd) {
  Mesh *mesh;
  MeshFace *f;
  for (size_t x = 0; x < rd->zbuffer->xmax; x++) {
    for (size_t y = 0; y < rd->zbuffer->ymax; y++) {
      double *z = (double *)MATRIX_Edit(rd->zbuffer, x, y);
      *z = -1.f;
    }
  }
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    for (unsigned int i_f = 0; i_f < MESH_GetNbFace(mesh); i_f++) {
      f = MESH_GetFace(mesh, i_f);
      void *args[2];
      args[0] = rd->zbuffer;
      args[1] = f;
      RASTER_GenerateFillTriangle(&f->p0->screen, &f->p1->screen,
                                  &f->p2->screen, callbackWriteZbuffer, args);
    }
  }
}

extern void RD_DrawVertices(struct Render *rd) {
  Mesh *mesh;
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    for (unsigned int i_v = 0; i_v < MESH_GetNbVertice(mesh); i_v++) {
      RASTER_DrawCircle(rd->raster, &MESH_GetVertex(mesh, i_v)->screen, 5,
                        CL_PAPAYAWHIP);
    }
  }
}

extern void RD_DrawAxis(struct Render *rd) {
  // Axes
  RASTER_DrawLine(rd->raster, &rd->p0.screen, &rd->px.screen, CL_RED);
  RASTER_DrawLine(rd->raster, &rd->p0.screen, &rd->py.screen, CL_GREEN);
  RASTER_DrawLine(rd->raster, &rd->p0.screen, &rd->pz.screen, CL_BLUE);
}

extern void RD_DrawFill(struct Render *rd) {
  RASTER_DrawFill(rd->raster, (color)0xFF000000); // Alpha
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*
 * 3D projection
 * http://www.cse.psu.edu/~rtc12/CSE486/lecture12.pdf
 */
static void calcProjectionVertex3(struct Render *rd, struct MeshVertex *p) {
  static double nnpx, nnpy;
  // World to camera
  p->cam.x = VECT_DotProduct(&rd->cam_u, &p->world) + rd->tx;
  p->cam.y = VECT_DotProduct(&rd->cam_v, &p->world) + rd->ty;
  p->cam.z = -VECT_DotProduct(&rd->cam_w, &p->world) + rd->tz;
  // Projection
  nnpx = (rd->s * p->cam.x) / p->cam.z;
  nnpy = (rd->s * p->cam.y) / p->cam.z;
  // Rendu
  p->sc.x = ((nnpx * rd->scalex + 1) * 0.5 * rd->raster->xmax);
  p->sc.y = ((1 - (nnpy * rd->scaley + 1) * 0.5) * rd->raster->ymax);
  p->sc.z = p->cam.z;
  //
  p->screen.x = (int32_t)p->sc.x;
  p->screen.y = (int32_t)p->sc.y;
  // printf("ps :[%d, %d]\n", ps.x, ps.y);
}

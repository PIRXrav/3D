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
// D'apres nos savants calculs
#define MAX_VERTICES_AFTER_CLIP 7

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static void calcProjectionVertex3(struct Render *rd, struct MeshVertex *p);

static void calcCacheBarycentreFace(struct MeshFace *f);

static void calcWbarycentre(struct MeshFace *f, uint32_t x, uint32_t y,
                            Vector *outW);

static int computePlaneSegmentIntersection(const Vector segment[2],
                                           const Vector **facePoints,
                                           Vector *intersection);
static void RD_ClipAndRasterFace(struct Render *rd, const MeshFace *face,
                                 void (*callback)(uint32_t, uint32_t, void **),
                                 void **args);

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
  ret->fbuffer = MATRIX_Init(xmax, ymax, sizeof(MeshFace *), "MF*");

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

extern void RD_CalcNormales(struct Render *rd) {
  Mesh *mesh;
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    // Calcul des normales de face
    for (unsigned int i = 0; i < MESH_GetNbFace(mesh); i++) {
      MESH_FACE_CalcNormaleFace(MESH_GetFace(mesh, i));
    }
    // Calcul des normales de sommets
    MESH_CalcVerticesNormales(mesh);
  }
}

extern void RD_calcCacheBarycentres(struct Render *rd) {
  Mesh *mesh;
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    for (unsigned int i = 0; i < MESH_GetNbFace(mesh); i++) {
      calcCacheBarycentreFace(MESH_GetFace(mesh, i));
    }
  }
}

/*
 * https://codeplea.com/triangular-interpolation?fbclid=IwAR38TFpipmfuQ5bM2P0Y07eym1ZHlt7-ZlcZAnEIb7EeOYU3uJzqWxuK0Ws
 */
static void callbackWriteZbuffer(uint32_t x, uint32_t y, void **args) {
  // Args
  MeshFace *f = (struct MeshFace *)args[1];
  struct Render *rd = (struct Render *)args[0];

  // Check args
  assert(x < rd->zbuffer->xmax || y < rd->zbuffer->ymax);

  static struct Vector w; // C'est plus un triplet de 3 coefs qu'un vector
  calcWbarycentre(f, x, y, &w);
  double z4 = w.x * f->p0->sc.z + w.y * f->p1->sc.z + w.z * f->p2->sc.z;

  if (z4 > 1000000000 || z4 < 0 || isnan(z4)) {
    printf("Z = %f\n", z4);
    printf("w1 = %f, w2 = %f, w3 = %f\n", w.x, w.y, w.z);
    VECT_Print(&f->p0->sc);
    VECT_Print(&f->p1->sc);
    VECT_Print(&f->p2->sc);
    printf("\n");
    assert(0);
  }

  if (*(double *)MATRIX_Edit(rd->zbuffer, x, y) > z4 ||
      *(double *)MATRIX_Edit(rd->zbuffer, x, y) < 0.f) { // SI plus proche
    *(double *)MATRIX_Edit(rd->zbuffer, x, y) = z4;
    *(MeshFace **)MATRIX_Edit(rd->fbuffer, x, y) = f;
  }
}

extern void RD_CalcZbuffer(struct Render *rd) {
  Mesh *mesh;
  MeshFace *f;
  for (size_t x = 0; x < rd->zbuffer->xmax; x++) {
    for (size_t y = 0; y < rd->zbuffer->ymax; y++) {
      *(double *)MATRIX_Edit(rd->zbuffer, x, y) = -1.f;
      *(MeshFace **)MATRIX_Edit(rd->fbuffer, x, y) = NULL;
    }
  }
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    for (unsigned int i_f = 0; i_f < MESH_GetNbFace(mesh); i_f++) {
      f = MESH_GetFace(mesh, i_f);
      void *args[2];
      args[0] = rd;
      args[1] = f;
      RD_ClipAndRasterFace(rd, f, callbackWriteZbuffer, args);
    }
  }
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

extern void RD_DrawVertices(struct Render *rd) {
  Mesh *mesh;
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    for (unsigned int i_v = 0; i_v < MESH_GetNbVertice(mesh); i_v++) {
      RASTER_DrawCircle(rd->raster, &MESH_GetVertex(mesh, i_v)->screen, 5,
                        CL_GREEN);
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
extern void RD_DrawZbuffer(struct Render *rd) {
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

extern void RD_DrawNormales(struct Render *rd) {
  Mesh *mesh;
  MeshVertex b1; // b0
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    mesh = rd->meshs[i_mesh];
    for (unsigned int i = 0; i < MESH_GetNbFace(mesh); i++) {
      MeshFace *f = MESH_GetFace(mesh, i);
      b1.world.x = f->p0->world.x + f->normal.x;
      b1.world.y = f->p0->world.y + f->normal.y;
      b1.world.z = f->p0->world.z + f->normal.z;
      calcProjectionVertex3(rd, &b1);
      RASTER_DrawLine(rd->raster, &f->p0->screen, &b1.screen, CL_BLUE);
    }
  }
}

extern void RD_DrawGbuffer(struct Render *rd) {
  Vector normal;
  Vector w;
  for (size_t x = 0; x < rd->raster->xmax; x++) {
    for (size_t y = 0; y < rd->raster->ymax; y++) {
      MeshFace *f = *(MeshFace **)MATRIX_Edit(rd->fbuffer, x, y);
      if (f != NULL) {
        calcWbarycentre(f, x, y, &w);
        normal.x = f->p0->normal.x * w.x + f->p1->normal.x * w.y +
                   f->p2->normal.x * w.z;
        normal.y = f->p0->normal.y * w.x + f->p1->normal.y * w.y +
                   f->p2->normal.y * w.z;
        normal.z = f->p0->normal.z * w.x + f->p1->normal.z * w.y +
                   f->p2->normal.z * w.z;
        VECT_Normalise(&normal);
        RASTER_DrawPixelxy(rd->raster, x, y,
                           CL_rgb(abs((int)(normal.x * 255)),
                                  abs((int)(normal.y * 255)),
                                  abs((int)(normal.z * 255))));
      }
    }
  }
}

extern void RD_DrawFbufferWithLum(struct Render *rd, struct Vector *lv,
                                  color lc) {
  for (size_t x = 0; x < rd->raster->xmax; x++) {
    for (size_t y = 0; y < rd->raster->ymax; y++) {
      MeshFace *f = *(MeshFace **)MATRIX_Edit(rd->fbuffer, x, y);
      if (f != NULL) {
        double k = VECT_DotProduct(lv, &f->normal);
        k = k < 0 ? 0 : k;
        RASTER_DrawPixelxy(rd->raster, x, y,
                           CL_rgb(k * lc.rgb.r, k * lc.rgb.g, k * lc.rgb.b));
      }
    }
  }
}
/*******************************************************************************
 * Internal function
 ******************************************************************************/

// https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
static int computePlaneSegmentIntersection(const Vector segment[2],
                                           const Vector **facePoints,
                                           Vector *intersection) {
  const Vector *a = &segment[0], *b = &segment[1];
  Vector ab;
  VECT_Sub(&ab, b, a);

  const Vector *p0 = facePoints[0], *p1 = facePoints[1], *p2 = facePoints[2];
  Vector p01, p02;
  VECT_Sub(&p01, p1, p0);
  VECT_Sub(&p02, p2, p0);

  Vector num1, num2, denom1, denom2;
  VECT_CrossProduct(&num1, &p01, &p02);
  VECT_Sub(&num2, a, p0);
  double numerateur = VECT_DotProduct(&num1, &num2);

  VECT_MultSca(&denom1, &ab, -1);
  VECT_CrossProduct(&denom2, &p01, &p02);
  double denominateur = VECT_DotProduct(&denom1, &denom2);

  // Si parallele : denominateur == 0
  if (fabs(denominateur) < 0.00001)
    return 0;

  double t = numerateur / denominateur;

  // Si 0 <= t <= 1, on est sur le segment
  if (t <= 0 || t >= 1)
    return 0;

  VECT_MultSca(&ab, &ab, t);
  VECT_Add(intersection, a, &ab);
  return 1;
}

static void callbackDrawXY(uint32_t x, uint32_t y, void **args) {
  Matrix *s = args[0];
  color c = *(color *)args[1];
  RASTER_DrawPixelxy(s, x, y, c);
}

extern void RD_RenderRaster(struct Render *rd) {
  for (unsigned i = 0; i < rd->nb_meshs; i++) {
    struct Mesh *mesh = rd->meshs[i];
    for (unsigned j = 0; j < MESH_GetNbFace(mesh); j++) {
      void *args[2] = {rd->raster, &MESH_GetFace(mesh, j)->color};
      RD_ClipAndRasterFace(rd, MESH_GetFace(mesh, j), callbackDrawXY, args);
    }
  }
}

// TODO: opti : remplacer les allocations dynamiques par des tableaux
// statiques avec comme taille le nombre maximum de sommets possibles (7 ?)
// https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
static void RD_ClipAndRasterFace(struct Render *rd, const MeshFace *face,
                                 void (*callback)(uint32_t, uint32_t, void **),
                                 void **args) {
  /* Pseudo code
   *
   * for (cube_face in projection_cube) {
   *    newFace = Face.empty();
   *    prev_point = face.last_point;
   *    for (current_point in face) {
   *        intersection =
   *            Intersection_Segment_Face((prev_point, current_point),
   *                                    cube_face);

   *        if (current_point inside cube_face)
   *            newFace.add(current_point);
   *        if (intersection)
   *            newFace.add(intersection);
   *        prev_point = current;
   *    }
   *    face = newFace;
   * }
   */
  static const double NEAR = 0.01, FAR = 100000;
  // Sommets du projectionCube face avant, face arriere, on commence en haut a
  // gauche, sens trigo
  static Vector projectionCubeVertices[8] = {
      {0, 0, NEAR}, {0, 1, NEAR}, {1, 1, NEAR}, {1, 0, NEAR},
      {0, 0, FAR},  {0, 1, FAR},  {1, 1, FAR},  {1, 0, FAR}};

  for (unsigned i = 0; i < 8; i += 4) {
    projectionCubeVertices[i + 1].y = rd->raster->ymax - 1;

    projectionCubeVertices[i + 2].x = rd->raster->xmax - 1;
    projectionCubeVertices[i + 2].y = rd->raster->ymax - 1;

    projectionCubeVertices[i + 3].x = rd->raster->xmax - 1;
  }

  // projectionCube de projection (seuls les 3 premiers sommets sont utilises
  // mais pour etre plus clair on met tout, ca coute rien)
  static const Vector *projectionCube[6][4] = {
      {projectionCubeVertices, projectionCubeVertices + 1,
       projectionCubeVertices + 2, projectionCubeVertices + 3}, // Face devant
      {projectionCubeVertices + 3, projectionCubeVertices + 2,
       projectionCubeVertices + 6, projectionCubeVertices + 7}, // Face droite
      {projectionCubeVertices + 7, projectionCubeVertices + 4,
       projectionCubeVertices + 5, projectionCubeVertices + 6}, // Face arriere
      {projectionCubeVertices + 4, projectionCubeVertices + 5,
       projectionCubeVertices + 1, projectionCubeVertices}, // Face gauche
      {projectionCubeVertices + 4, projectionCubeVertices,
       projectionCubeVertices + 3, projectionCubeVertices + 7}, // Face dessus
      {projectionCubeVertices + 5, projectionCubeVertices + 1,
       projectionCubeVertices + 2, projectionCubeVertices + 6} // Face dessous
  };
  static const Vector vecteursNormaux[6] = {{0, 0, -1}, {1, 0, 0},  {0, 0, 1},
                                            {-1, 0, 0}, {0, -1, 0}, {0, 1, 0}};

  static Vector facePointsBuff[MAX_VERTICES_AFTER_CLIP];
  Vector *facePoints = facePointsBuff;
  unsigned facePointsNb = 3;
  facePoints[0] = face->p0->sc;
  facePoints[1] = face->p1->sc;
  facePoints[2] = face->p2->sc;

  static Vector newFacePointsBuff[MAX_VERTICES_AFTER_CLIP];
  Vector *newFacePoints = newFacePointsBuff;
  unsigned newFacePointsNb = 0;

  for (unsigned cf = 0; cf < 6; cf++) {
    newFacePointsNb = 0;

    for (unsigned p = 0; p < facePointsNb; p++) {
      Vector *currentPoint = &facePoints[p];
      Vector *prevPoint = &facePoints[(p + facePointsNb - 1) % facePointsNb];

      Vector segment[2] = {*prevPoint, *currentPoint};
      Vector intersection;

      int hasIntersection = computePlaneSegmentIntersection(
          segment, projectionCube[cf], &intersection);

      Vector vectPrev;
      VECT_Sub(&vectPrev, prevPoint, projectionCube[cf][0]);

      if (VECT_DotProduct(&vecteursNormaux[cf], &vectPrev) <= 0) {
        newFacePoints[newFacePointsNb++] = *prevPoint;
      }
      if (hasIntersection) {
        newFacePoints[newFacePointsNb++] = intersection;
      }
    }

    void *tmp = facePoints;
    facePoints = newFacePoints;
    newFacePoints = tmp;

    facePointsNb = newFacePointsNb;
    newFacePointsNb = 0;

    if (!facePointsNb)
      break;
  }

  // Apres triangulation (comme on la fait ici), on aura nbSommets - 2 faces
  int nbFaces = facePointsNb - 2;
  // S'il n'y a que deux sommets on ne dessine rien
  if (nbFaces <= 0)
    return;

  // On triangule la face puis on rasterise 'on the fly' comme Pierre aime a
  // le dire
  for (int i = 0; i < nbFaces; i++) {
    Vector *p0 = &facePoints[0];
    Vector *p1 = &facePoints[i + 1];
    Vector *p2 = &facePoints[i + 2];
    RasterPos a = {p0->x, p0->y}, b = {p1->x, p1->y}, c = {p2->x, p2->y};
    RASTER_GenerateFillTriangle(&a, &b, &c, callback, args);
  }
}

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

static void calcCacheBarycentreFace(struct MeshFace *f) {
  // Barycentre
  double denum = (f->p1->sc.y - f->p2->sc.y) * (f->p0->sc.x - f->p2->sc.x) +
                 (f->p2->sc.x - f->p1->sc.x) * (f->p0->sc.y - f->p2->sc.y);
  if (denum == 0) { // Face invisible car dans le mauvais plan
                    // ATTENTION! CHangement comportement !!!
    f->wp1 = 0;
    f->wp2 = 0;
    f->wp3 = 0;
    f->wp4 = 0;
    return;
  }
  f->wp1 = (f->p1->sc.y - f->p2->sc.y) / denum;
  f->wp3 = (f->p2->sc.y - f->p0->sc.y) / denum;
  f->wp2 = (f->p2->sc.x - f->p1->sc.x) / denum;
  f->wp4 = (f->p0->sc.x - f->p2->sc.x) / denum;
}

// Retourne les 3 coefficients du point par rapport au triangle
static void calcWbarycentre(struct MeshFace *f, uint32_t x, uint32_t y,
                            Vector *outW) {
  outW->x = f->wp1 * (x - f->p2->sc.x) + f->wp2 * (y - f->p2->sc.y);
  outW->y = f->wp3 * (x - f->p2->sc.x) + f->wp4 * (y - f->p2->sc.y);
  outW->z = 1 - outW->x - outW->y;

  outW->x = outW->x > 0 ? outW->x : 0;
  outW->y = outW->y > 0 ? outW->y : 0;
  outW->z = outW->z > 0 ? outW->z : 0;

  double sum = (outW->x + outW->y + outW->z);
  outW->x /= sum;
  outW->y /= sum;
  outW->z /= sum;
}

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "render.h"
#include "color.h"
#include "geo.h"
#include "mesh.h"

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

/*
 * Initialisation
 */
struct Render *RD_Init(unsigned int xmax, unsigned int ymax) {
  struct Render *ret = malloc(sizeof(struct Render));

  ret->nb_meshs = 0;
  ret->meshs = malloc(sizeof(struct mesh *) * ret->nb_meshs);

  ret->xmax = xmax;
  ret->ymax = ymax;

  ret->highlightedMesh = NULL;
  ret->highlightedFace = NULL;

  ret->fov_rad = 3;
  struct Vector cam_pos = {-5, -5, 0};
  struct Vector cam_forward = {-1, -1, 0};
  struct Vector cam_up = {0, 0, 1};
  RD_SetCam(ret, &cam_pos, &cam_forward, &cam_up);

  return ret;
}

/*
 * Initialisation
 */
struct Render *RD_InitTetrahedrons(unsigned int xmax, unsigned int ymax) {
  struct Render *ret = malloc(sizeof(struct Render));

  unsigned int cote = 2;
  ret->nb_meshs = cote * cote;
  ret->meshs = malloc(sizeof(struct mesh *) * ret->nb_meshs);
  for (unsigned int i = 0; i < cote; i++) {
    for (unsigned int j = 0; j < cote; j++) {
      static struct Vector pos;
      VECT_Set(&pos, i, j, 0);
      ret->meshs[i * cote + j] = MESH_InitTetrahedron(&pos);
    }
  }

  ret->xmax = xmax;
  ret->ymax = ymax;
  printf("MAXMIN%d %d\n", xmax, ymax);

  VECT_Set(&ret->cam_pos, -5, -5, 0);
  VECT_Set(&ret->cam_u, 1, 0, 0);
  VECT_Set(&ret->cam_v, 0, 1, 0);
  VECT_Set(&ret->cam_w, 0, 0, -1);
  VECT_Set(&ret->cam_wp, -1, -1, -1);
  ret->fov_rad = 3;

  return ret;
}

/* Ajoute une mesh au render, aucune copie n'est faite */
void RD_AddMesh(struct Render *rd, struct Mesh *m) {
  rd->nb_meshs++;
  rd->meshs = realloc(rd->meshs, sizeof(struct mesh *) * rd->nb_meshs);
  rd->meshs[rd->nb_meshs - 1] = m;
}

void RD_Print(struct Render *rd) {
  printf(" ============ RENDER =========== \n");
  printf("\tVECT CAM: ");
  VECT_Print(&rd->cam_wp);
  printf("\n");
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    MESH_Print(rd->meshs[i_mesh]);
    printf("\n");
  }
}

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
    if (RayIntersectsTriangle(cam_pos, cam_ray, mf->p0, mf->p1, mf->p2, x)) {
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
 * Intersection d'un rayon avec toutes les meshs, on retourne le point, la face
 * et la mesh en collision
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
  static struct Vector cam_forward_1, cam_up_world_1;
  VECT_Cpy(&cam_forward_1, cam_forward);
  VECT_Cpy(&cam_up_world_1, cam_up_world);
  VECT_Normalise(&cam_forward_1);
  VECT_Normalise(&cam_up_world_1);

  // Cam pos
  VECT_Cpy(&rd->cam_pos, cam_pos);
  // forward
  VECT_Cpy(&rd->cam_w, &cam_forward_1);
  // right
  VECT_CrossProduct(&rd->cam_u, &cam_up_world_1, &cam_forward_1);
  // up
  VECT_CrossProduct(&rd->cam_v, &rd->cam_w, &rd->cam_u);

  /* Précalcul w' */
  struct Vector un, vn, wn;
  VECT_MultSca(&un, &rd->cam_u, -(double)rd->xmax / 2);
  VECT_MultSca(&vn, &rd->cam_v, (double)rd->ymax / 2);
  VECT_MultSca(&wn, &rd->cam_w,
               ((double)rd->ymax / 2) / tan(rd->fov_rad * 0.5));
  VECT_Add(&rd->cam_wp, &un, &vn);
  VECT_Sub(&rd->cam_wp, &rd->cam_wp, &wn);
}

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*******************************************************************************
 * Internal function
 ******************************************************************************/

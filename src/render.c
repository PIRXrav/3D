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

  VECT_Set(&ret->cam_pos, -5, -5, 0);
  VECT_Set(&ret->cam_u, 1, 0, 0);
  VECT_Set(&ret->cam_v, 0, 1, 0);
  VECT_Set(&ret->cam_w, 0, 0, -1);
  ret->fov_rad = 3;

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
  ret->fov_rad = 3;

  return ret;
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
  /* Précalcul w' */
  struct Vector un, vn, wn;
  VECT_MultSca(&un, &rd->cam_u, -(double)rd->xmax / 2);
  VECT_MultSca(&vn, &rd->cam_v, (double)rd->ymax / 2);
  VECT_MultSca(&wn, &rd->cam_w,
               ((double)rd->ymax / 2) / tan(rd->fov_rad * 0.5));
  VECT_Add(&rd->cam_wp, &un, &vn);
  VECT_Sub(&rd->cam_wp, &rd->cam_wp, &wn);

  //
  // printf("un vn wn :");
  // VECT_Print(&un);
  // VECT_Print(&vn);
  // VECT_Print(&wn);
  // printf("\n");
  //
  /* Calcul ray */
  struct Vector ux, vy;
  VECT_MultSca(&ux, &rd->cam_u, (double)sx);
  VECT_MultSca(&vy, &rd->cam_v, -(double)sy);
  VECT_Add(ray, &ux, &vy);
  VECT_Add(ray, ray, &rd->cam_wp);
  // VECT_Normalise(ray);
  // Vec3f ray_dir = normalize(x * u + y * (-v) + w_p);
}

/*
 * Retourne l'intersection de la ray, sa couleur et la distance de collsion
 * return bool : etat du succes. 1 si collision
 * vector x       : POint de croisement                     [OUT]
 * distanceSquare : la distance maximale (dernier valide).  [IN/OUT]
 * color          : La couleur du dernier valide.           [IN/OUT]
 *
 * La couleur et la distance sont mit à jour si collision dans ce mesh
 */
static bool RD_RayTraceOnMesh(const struct Mesh *mesh,
                              const struct Vector *cam_pos,
                              const struct Vector *cam_ray, struct Vector *x,
                              double *distance, color *color) {
  bool hit = false;
  for (unsigned int i_face = 0; i_face < MESH_GetNbFace(mesh); i_face++) {
    struct Triangle *tr =
        MESH_FACE_ToTriangleStatique(MESH_GetFace(mesh, i_face));
    if (RayIntersectsTriangle(cam_pos, cam_ray, tr, x)) {
      double d = VECT_DistanceSquare(cam_pos, x);
      if (d < *distance) {
        *color = MESH_GetFace(mesh, i_face)->color;
        *distance = d;
        hit = true;
      }
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
  color color = CL_rgb(0, 0, 0); // background color
  double distance = 99999;       // Max dist
  for (unsigned int i_mesh = 0; i_mesh < rd->nb_meshs; i_mesh++) {
    RD_RayTraceOnMesh(rd->meshs[i_mesh], &rd->cam_pos, ray, x, &distance,
                      &color);
  }
  return color;
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

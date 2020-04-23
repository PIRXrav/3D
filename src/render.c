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

  ret->mesh = MESH_InitTetrahedron();

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
  printf("\tMESH: \n");
  MESH_Print(rd->mesh);
  printf("\n");
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

color RD_RayTraceOnce(struct Render *rd, struct Vector *ray) {
  struct Vector x;
  color best_c = CL_rgb(0, 0, 0);
  double best_dist = 9999999;

  for (unsigned int i_face = 0; i_face < rd->mesh->nb_faces; i_face++) {
    static struct Triangle tr;
    VECT_Cpy(&tr.a, rd->mesh->faces[i_face].p0);
    VECT_Cpy(&tr.b, rd->mesh->faces[i_face].p1);
    VECT_Cpy(&tr.c, rd->mesh->faces[i_face].p2);
    if (RayIntersectsTriangle(&rd->cam_pos, ray, &tr, &x)) {
      double d = VECT_Distance(&rd->cam_pos, &x);
      if (d < best_dist) {
        best_c = rd->mesh->faces[i_face].color;
        best_dist = d;
      }
    }
  }
  return best_c;
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

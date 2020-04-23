#ifndef _RENDER_H_
#define _RENDER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "color.h"
#include "geo.h"
#include "mesh.h"

#include <stdint.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

struct Render {
  /*data*/
  struct Mesh *mesh;

  /*Plan*/
  unsigned int xmax;
  unsigned int ymax;
  double fov_rad;

  /* Raies */
  // struct Vecteur **raies_dir;

  /* Camera */
  struct Vector cam_pos;
  struct Vector cam_u;
  struct Vector cam_v;
  struct Vector cam_w;  // cam dir
  struct Vector cam_wp; // précalcul unique
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 * Initialisation
 */
struct Render *RD_Init(unsigned int xmax, unsigned int ymax);

/*
 * Calcule d'une raie
 */
void RD_CalcRayDir(struct Render *rd, unsigned int sx, unsigned int sy,
                   struct Vector *ray);

color RD_RayTraceOnce(struct Render *rd, struct Vector *ray);

void RD_Print(struct Render *rd);

/*
 * https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function
 */
void RD_SetCam(struct Render *rd, const struct Vector *cam_pos,
               const struct Vector *cam_forward,
               const struct Vector *cam_up_world);

#endif /* _RENDER_H_ */

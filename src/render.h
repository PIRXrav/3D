#ifndef _RENDER_H_
#define _RENDER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "color.h"
#include "containers/matrix.h"
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
  unsigned int nb_meshs;
  struct Mesh **meshs; // Tableau de pointeur de mesh

  /* Repere world */
  struct MeshVertex p0, px, py, pz;

  /*Plan*/
  struct Mesh *highlightedMesh;
  struct MeshFace *highlightedFace;

  /* Camera */
  double fov_rad;
  struct Vector cam_pos;
  struct Vector cam_forward;
  struct Vector cam_up_world;

  /* Précalcul world <-> camera*/
  struct Vector cam_u;
  struct Vector cam_v;
  struct Vector cam_w; // cam dir

  /* Précalcul raytracting */
  struct Vector cam_wp;

  /* Précalul Projection */
  double tx, ty, tz;     // changement de plan de la camera
  double s;              // Fc du fov
  double scalex, scaley; // relations à la taille de l'écran

  /* Ecran */
  Matrix *raster; // Rendu de la scene 2D matrix
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

/* Ajoute une mesh au render, aucune copie n'est faite */
void RD_AddMesh(struct Render *rd, struct Mesh *m);

/*
 * Calcule d'une raie
 */
void RD_CalcRayDir(struct Render *rd, unsigned int sx, unsigned int sy,
                   struct Vector *ray);

/*
 * Intersection avec tout les meshs
 * On retourne le point de croisement x
 */
extern color RD_RayTraceOnRD(const struct Render *rd, const struct Vector *ray,
                             struct Vector *x);

/*
 * Intersection d'un rayon avec toutes les meshs, on retourne le point, la face
 * et la mesh en collision
 */
extern bool RD_RayCastOnRD(const struct Render *rd, const struct Vector *ray,
                           struct Vector *x, struct Mesh **m,
                           struct MeshFace **face);

void RD_Print(struct Render *rd);

/*
 * https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function
 */
void RD_SetCam(struct Render *rd, const struct Vector *cam_pos,
               const struct Vector *cam_forward,
               const struct Vector *cam_up_world);

void RD_DrawRaytracing(struct Render *rd);
void RD_DrawWireframe(struct Render *rd);
void RD_DrawVertices(struct Render *rd);
void RD_DrawAxis(struct Render *rd);
void RD_DrawFill(struct Render *rd);

void RD_CalcProjectionVertices(struct Render *rd);

#endif /* _RENDER_H_ */

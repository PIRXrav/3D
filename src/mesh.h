#ifndef _MESH_H_
#define _MESH_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "color.h"
#include "geo.h"

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

struct MeshFace {
  struct Vector *p0, *p1, *p2; // Uniquement des triangles
  color color;
};

struct MeshEdges {
  struct Vector *p0, *p1;
};

struct Mesh {

  unsigned int nb_vertices;
  struct Vector *vertices;
  unsigned int nb_faces;
  struct MeshFace *faces;

  struct Vector origin;
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 * Initialise un tetrahedre
 * https://en.wikipedia.org/wiki/Tetrahedron
 */
struct Mesh *MESH_InitTetrahedron(struct Vector *origin);

void MESH_Print(struct Mesh *mesh);

#endif /* _GEO_H_ */

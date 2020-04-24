#ifndef _MESH_H_
#define _MESH_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "color.h"
#include "containers/arraylist.h"
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
  const char *name;    // Le nom du mesh
  ArrayList *vertices; // Vector
  ArrayList *faces;    // MeshFace

  struct Vector origin;
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*
 * Initialise une mesh vide
 */
struct Mesh *MESH_Init();

/*
 * Initialise un tetrahedre
 * https://en.wikipedia.org/wiki/Tetrahedron
 */
struct Mesh *MESH_InitTetrahedron(struct Vector *origin);

/*
 * Retourne le nombre de faces du mesh
 */
size_t MESH_GetNbFace(const struct Mesh *mesh);

/* Definit le nom de la mesh */
void MESH_SetName(struct Mesh *mesh, const char *name);

/*
 * Retourne le nombre de sommets du mesh
 */
size_t MESH_GetNbVertice(const struct Mesh *mesh);

/*
 * Retourne le vecteur de face
 */
struct MeshFace *MESH_GetFace(const struct Mesh *mesh, size_t index);

/*
 * Retourne le vecteur de sommets
 */
struct Vector *MESH_GetVertice(const struct Mesh *mesh, size_t index);

/*
 *  Ajoute une face au mesh
 */
struct MeshFace *MESH_AddFace(struct Mesh *mesh, const struct Vector *vertex0,
                              const struct Vector *vertex1,
                              const struct Vector *vertex2, color c);

struct Triangle *MESH_FACE_ToTriangleStatique(struct MeshFace *mf);

void MESH_Print(struct Mesh *mesh);

#endif /* _GEO_H_ */

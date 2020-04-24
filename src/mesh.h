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

typedef struct Vector MeshVertex;

typedef struct MeshFace MeshFace;
struct MeshFace {
  MeshVertex *p0, *p1, *p2; // Uniquement des triangles
  color color;
};

typedef struct MeshEdge MeshEdge;
struct MeshEdge {
  MeshVertex *p0, *p1;
};

typedef struct Mesh Mesh;
struct Mesh {
  char *name;          // Le nom du mesh
  ArrayList *vertices; // Vector
  ArrayList *faces;    // MeshFace
  MeshVertex origin;
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

// Initialisation VERTEX
extern MeshVertex *MESH_VERT_Set(MeshVertex *v, double x, double y, double z);
extern MeshVertex *MESH_VERT_Init(double x, double y, double z);
extern void MESH_VERT_Print(MeshVertex *v);

// Initialisation face
extern MeshFace *MESH_FACE_Set(MeshFace *mf, MeshVertex *p0, MeshVertex *p1,
                               MeshVertex *p2, color c);
extern MeshFace *MESH_FACE_Init(MeshVertex *p1, MeshVertex *p2, MeshVertex *p3,
                                color c);
extern void MESH_FACE_Print(struct MeshFace *face);

// Mesh
extern Mesh *MESH_Init(void);
extern size_t MESH_GetNbFace(const Mesh *mesh);
extern size_t MESH_GetNbVertice(const Mesh *mesh);
extern MeshFace *MESH_GetFace(const Mesh *mesh, size_t index);
extern MeshVertex *MESH_GetVertex(const Mesh *mesh, size_t index);
extern MeshVertex *MESH_AddVertex(Mesh *mesh, MeshVertex *vertex);
extern MeshFace *MESH_AddFace(Mesh *mesh, MeshFace *face);
extern void MESH_SetName(Mesh *mesh, const char *name);
extern Mesh *MESH_InitTetrahedron(MeshVertex *origin);
extern void MESH_Print(const Mesh *mesh);

#endif /* _GEO_H_ */

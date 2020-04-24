/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "mesh.h"
#include "color.h"
#include "containers/arraylist.h"
#include "geo.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
 * Initialise une mesh vide
 */
struct Mesh *MESH_Init(void) {
  struct Mesh *m = malloc(sizeof(struct Mesh));
  // m->nb_vertices = 0;
  m->vertices = ARRLIST_Create(sizeof(struct Vector));
  // m->nb_faces = 0;
  m->faces = ARRLIST_Create(sizeof(struct MeshFace));
  VECT_Set(&m->origin, 0, 0, 0);
  return m;
}

/*
 * Set une face
 */
void MESH_FACE_Set(struct MeshFace *mf, struct Vector *p0, struct Vector *p1,
                   struct Vector *p2, color c) {
  mf->p0 = p0;
  mf->p1 = p1;
  mf->p2 = p2;
  mf->color = c;
}

/*
 * Set une face statique
 */
struct MeshFace *MESH_FACE_SetStatic(struct Vector *p0, struct Vector *p1,
                                     struct Vector *p2, color c) {
  static struct MeshFace mf;
  mf.p0 = p0;
  mf.p1 = p1;
  mf.p2 = p2;
  mf.color = c;
  return &mf;
}

/*
 * Retourne une instance statique du triangle géometrique
 */
struct Triangle *MESH_FACE_ToTriangleStatique(struct MeshFace *mf) {
  static struct Triangle tr;
  VECT_Cpy(&tr.a, mf->p0);
  VECT_Cpy(&tr.b, mf->p1);
  VECT_Cpy(&tr.c, mf->p2);
  return &tr;
}

/*
 * Retourne le nombre de faces du mesh
 */
size_t MESH_GetNbFace(const struct Mesh *mesh) {
  return ARRLIST_GetSize(mesh->faces);
}

/*
 * Retourne le nombre de sommets du mesh
 */
size_t MESH_GetNbVertice(const struct Mesh *mesh) {
  return ARRLIST_GetSize(mesh->vertices);
}

/*
 * Retourne le vecteur de face
 */
struct MeshFace *MESH_GetFace(const struct Mesh *mesh, size_t index) {
  return ARRLIST_Get(mesh->faces, index);
}

/*
 * Retourne le vecteur de sommets
 */
struct Vector *MESH_GetVertices(const struct Mesh *mesh, size_t index) {
  return ARRLIST_Get(mesh->vertices, index);
}

/*
 * Ajoute un sommet au mesh si il n'existe pas. On le retourne.
 */
static struct Vector *MESH_AddVertex(struct Mesh *mesh,
                                     const struct Vector *vertex) {
  int index = ARRLIST_Search(mesh->vertices, vertex);
  if (index != -1)
    return ARRLIST_Get(mesh->vertices, index);
  return ARRLIST_Add(mesh->vertices, vertex);
}

/*
 *  Ajoute une face au mesh
 */
static struct MeshFace *MESH_AddFace(struct Mesh *mesh,
                                     const struct Vector *vertices,
                                     size_t nb_vertices, color c) {
  assert(nb_vertices == 3);
  return ARRLIST_Add(
      mesh->faces, MESH_FACE_SetStatic(MESH_AddVertex(mesh, &vertices[0]),
                                       MESH_AddVertex(mesh, &vertices[1]),
                                       MESH_AddVertex(mesh, &vertices[2]), c));
}

/*
 * Initialise un tetrahedre
 * https://en.wikipedia.org/wiki/Tetrahedron
 */
struct Mesh *MESH_InitTetrahedron(struct Vector *origin) {
  struct Mesh *p = MESH_Init();

  ARRLIST_Add(p->vertices, VECT_SetStatic(0, 0, 0));
  ARRLIST_Add(p->vertices, VECT_SetStatic(1, 0, 0));
  ARRLIST_Add(p->vertices, VECT_SetStatic(0, 1, 0));
  ARRLIST_Add(p->vertices, VECT_SetStatic(0, 0, 1));
  assert(MESH_GetNbVertice(p) == 4);

  for (unsigned int i = 0; i < MESH_GetNbVertice(p); i++) {
    struct Vector *vert = ARRLIST_Get(p->vertices, i);
    VECT_Add(vert, vert, origin);
  }

  struct Vector *raw_verts = ARRLIST_GetData(p->vertices);
  ARRLIST_Add(p->faces, MESH_FACE_SetStatic(&raw_verts[0], &raw_verts[1],
                                            &raw_verts[2], CL_rgb(0, 0, 255)));
  ARRLIST_Add(p->faces, MESH_FACE_SetStatic(&raw_verts[0], &raw_verts[1],
                                            &raw_verts[3], CL_rgb(0, 255, 0)));
  ARRLIST_Add(p->faces, MESH_FACE_SetStatic(&raw_verts[0], &raw_verts[2],
                                            &raw_verts[3], CL_rgb(255, 0, 0)));
  ARRLIST_Add(p->faces, MESH_FACE_SetStatic(&raw_verts[1], &raw_verts[2],
                                            &raw_verts[3], CL_rgb(90, 90, 90)));
  assert(MESH_GetNbFace(p) == 4);

  return p;
}

void MESH_Print(struct Mesh *mesh) {
  printf("NB TR = %lu\n", MESH_GetNbFace(mesh));
  for (size_t i_face = 0; i_face < MESH_GetNbFace(mesh); i_face++) {
    printf("TR[%lu]:{", i_face);
    VECT_Print(((struct MeshFace *)ARRLIST_Get(mesh->faces, i_face))->p0);
    VECT_Print(((struct MeshFace *)ARRLIST_Get(mesh->faces, i_face))->p1);
    VECT_Print(((struct MeshFace *)ARRLIST_Get(mesh->faces, i_face))->p2);
    printf("}\n");
  }
}

/*
 * Translate le mesh suivant le vecteur depl
 */
void MESH_Translate(struct Mesh *mesh, struct Vector *depl) {
  ; // TODO
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

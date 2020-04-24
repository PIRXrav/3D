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
 * Initialise une mesh vide
 */
struct Mesh *MESH_Init(void) {
  struct Mesh *m = malloc(sizeof(struct Mesh));
  m->vertices = ARRLIST_Create(sizeof(struct Vector));
  m->faces = ARRLIST_Create(sizeof(struct MeshFace));
  m->name = NULL;
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
struct MeshFace *MESH_AddFace(struct Mesh *mesh, const struct Vector *vertex0,
                              const struct Vector *vertex1,
                              const struct Vector *vertex2, color c) {
  return ARRLIST_Add(mesh->faces,
                     MESH_FACE_SetStatic(MESH_AddVertex(mesh, vertex0),
                                         MESH_AddVertex(mesh, vertex1),
                                         MESH_AddVertex(mesh, vertex2), c));
}

/* Definit le nom de la mesh */
void MESH_SetName(struct Mesh *mesh, const char *name) {
  mesh->name = realloc(mesh->name, strlen(name) + 1);
  strcpy(mesh->name, name);
}

/*
 * Initialise un tetrahedre
 * https://en.wikipedia.org/wiki/Tetrahedron
 */
struct Mesh *MESH_InitTetrahedron(struct Vector *origin) {
  struct Mesh *p = MESH_Init();
  MESH_SetName(p, "Tetrahedron");

  struct Vector o = {0, 0, 0};
  struct Vector x = {1, 0, 0};
  struct Vector y = {0, 1, 0};
  struct Vector z = {0, 0, 1};

  VECT_Add(&o, &o, origin);
  VECT_Add(&x, &x, origin);
  VECT_Add(&y, &y, origin);
  VECT_Add(&z, &z, origin);

  MESH_AddFace(p, &o, &x, &y, CL_rgb(0, 0, 255));
  MESH_AddFace(p, &o, &x, &z, CL_rgb(0, 255, 0));
  MESH_AddFace(p, &o, &y, &z, CL_rgb(255, 0, 0));
  MESH_AddFace(p, &x, &y, &z, CL_DEEPPINK);

  assert(MESH_GetNbFace(p) == 4);
  assert(MESH_GetNbVertice(p) == 4);
  return p;
}

void MESH_FACE_Print(struct MeshFace *face) {
  VECT_Print(face->p0);
  VECT_Print(face->p1);
  VECT_Print(face->p2);
}

void MESH_Print(struct Mesh *mesh) {
  printf("MESH : %s\n", mesh->name);
  printf("NBTR : %lu\n", MESH_GetNbFace(mesh));
  for (size_t i_face = 0; i_face < MESH_GetNbFace(mesh); i_face++) {
    printf("TR[%lu]:{", i_face);
    MESH_FACE_Print(ARRLIST_Get(mesh->faces, i_face));
    printf("}\n");
  }
}

/*
 * Translate le mesh suivant le vecteur depl
 */
/*
void MESH_Translate(struct Mesh *mesh, struct Vector *depl) {
  ; // TODO
}
*/
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "mesh.h"
#include "color.h"
#include "containers/arraylistp.h"
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

// Initialisation VERTEX
extern MeshVertex *MESH_VERT_Set(MeshVertex *v, double x, double y, double z) {
  assert(v);
  v->x = x;
  v->y = y;
  v->z = z;
  return v;
}

extern MeshVertex *MESH_VERT_Init(double x, double y, double z) {
  return MESH_VERT_Set(malloc(sizeof(MeshVertex)), x, y, z);
}

extern void MESH_VERT_Print(MeshVertex *v) { VECT_Print(v); }
/*
 * Set une face
 */
extern MeshFace *MESH_FACE_Set(MeshFace *mf, MeshVertex *p0, MeshVertex *p1,
                               MeshVertex *p2, color c) {
  assert(mf);
  mf->p0 = p0;
  mf->p1 = p1;
  mf->p2 = p2;
  mf->color = c;
  return mf;
}

/*
 * Initialise une face
 */
extern MeshFace *MESH_FACE_Init(MeshVertex *p1, MeshVertex *p2, MeshVertex *p3,
                                color c) {
  return MESH_FACE_Set(malloc(sizeof(MeshFace)), p1, p2, p3, c);
}

extern void MESH_FACE_Print(struct MeshFace *face) {
  MESH_VERT_Print(face->p0);
  MESH_VERT_Print(face->p1);
  MESH_VERT_Print(face->p2);
}

/*
 * Initialise une mesh vide
 */
extern Mesh *MESH_Init(void) {
  Mesh *m = malloc(sizeof(Mesh));
  m->vertices = ARRLISTP_Create();
  m->faces = ARRLISTP_Create();
  m->name = NULL;
  MESH_VERT_Set(&m->origin, 0, 0, 0);
  return m;
}

/*
 * Set une face statique
 */
/*
struct MeshFace *MESH_FACE_SetStatic(struct Vector *p0, struct Vector *p1,
                                     struct Vector *p2, color c) {
  static struct MeshFace mf;
  mf.p0 = p0;
  mf.p1 = p1;
  mf.p2 = p2;
  mf.color = c;
  return &mf;
}
*/
/*
 * Retourne une instance statique du triangle géometrique
 */

extern struct Triangle *MESH_FACE_ToTriangleStatique(MeshFace *mf) {
  static struct Triangle tr;
  VECT_Cpy(&tr.a, mf->p0);
  VECT_Cpy(&tr.b, mf->p1);
  VECT_Cpy(&tr.c, mf->p2);
  return &tr;
}

/*
 * Retourne le nombre de faces du mesh
 */
extern size_t MESH_GetNbFace(const Mesh *mesh) {
  return ARRLISTP_GetSize(mesh->faces);
}

/*
 * Retourne le nombre de sommets du mesh
 */
extern size_t MESH_GetNbVertice(const Mesh *mesh) {
  return ARRLISTP_GetSize(mesh->vertices);
}

/*
 * Retourne le vecteur de face
 */
extern MeshFace *MESH_GetFace(const Mesh *mesh, size_t index) {
  return ARRLISTP_Get(mesh->faces, index);
}

/*
 * Retourne le sommet
 */
extern MeshVertex *MESH_GetVertex(const Mesh *mesh, size_t index) {
  return ARRLISTP_Get(mesh->vertices, index);
}

/*
 * Ajoute un sommet au mesh si il n'existe pas. On le retourne.
 */
extern MeshVertex *MESH_AddVertex(Mesh *mesh, MeshVertex *vertex) {
  return ARRLISTP_Add(mesh->vertices, vertex);
}

/*
 *  Ajoute une face au mesh
 */
extern MeshFace *MESH_AddFace(Mesh *mesh, MeshFace *face) {
  return ARRLISTP_Add(mesh->faces, face);
}

/* Definit le nom de la mesh */
extern void MESH_SetName(Mesh *mesh, const char *name) {
  mesh->name = realloc(mesh->name, strlen(name) + 1);
  strcpy(mesh->name, name);
}

/*
 * Initialise un tetrahedre
 * https://en.wikipedia.org/wiki/Tetrahedron
 */
extern Mesh *MESH_InitTetrahedron(MeshVertex *origin) {
  Mesh *p = MESH_Init();
  MESH_SetName(p, "Tetrahedron");

  MeshVertex *o = MESH_VERT_Init(0, 0, 0);
  MeshVertex *x = MESH_VERT_Init(1, 0, 0);
  MeshVertex *y = MESH_VERT_Init(0, 1, 0);
  MeshVertex *z = MESH_VERT_Init(0, 0, 1);

  VECT_Add(o, o, origin);
  VECT_Add(x, x, origin);
  VECT_Add(y, y, origin);
  VECT_Add(z, z, origin);

  MESH_AddVertex(p, o);
  MESH_AddVertex(p, x);
  MESH_AddVertex(p, y);
  MESH_AddVertex(p, z);

  MESH_AddFace(p, MESH_FACE_Init(o, x, y, CL_rgb(0, 0, 255)));
  MESH_AddFace(p, MESH_FACE_Init(o, x, z, CL_rgb(0, 255, 0)));
  MESH_AddFace(p, MESH_FACE_Init(o, y, z, CL_rgb(255, 0, 0)));
  MESH_AddFace(p, MESH_FACE_Init(x, y, z, CL_DEEPPINK));

  assert(MESH_GetNbFace(p) == 4);
  assert(MESH_GetNbVertice(p) == 4);
  return p;
}

extern void MESH_Print(const Mesh *mesh) {
  printf("MESH : %s\n", mesh->name);
  printf("NBTR : %lu\n", MESH_GetNbFace(mesh));
  for (size_t i_face = 0; i_face < MESH_GetNbFace(mesh); i_face++) {
    printf("TR[%lu]:{", i_face);
    MESH_FACE_Print(ARRLISTP_Get(mesh->faces, i_face));
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

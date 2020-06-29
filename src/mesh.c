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

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

// Initialisation VERTEX
extern MeshVertex *MESH_VERT_Set(MeshVertex *v, double x, double y, double z) {
  assert(v);
  v->world.x = x;
  v->world.y = y;
  v->world.z = z;
  return v;
}

extern MeshVertex *MESH_VERT_Init(double x, double y, double z) {
  return MESH_VERT_Set(malloc(sizeof(MeshVertex)), x, y, z);
}

extern void MESH_VERT_Print(MeshVertex *v) { VECT_Print(&v->world); }
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

// Pour des points dans le même plan
extern MeshFace **MESH_FACE_FromVertices(MeshVertex **vertices,
                                         unsigned nbVertices, unsigned *nbFaces,
                                         color c) {
  int nbFaceCalc = nbVertices - 2;
  if (nbFaceCalc <= 0)
    return NULL;

  *nbFaces = (unsigned)nbFaceCalc;

  MeshFace **faces = malloc(sizeof(MeshFace *) * *nbFaces);
  for (unsigned i = 0; i < *nbFaces; i++) {
    faces[i] = MESH_FACE_Init(vertices[0], vertices[i + 1], vertices[i + 2], c);
  }
  return faces;
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
  BOX3_Reset(&m->box);
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
  BOX3_AddPoint(&mesh->box, &vertex->world);
  return ARRLISTP_Add(mesh->vertices, vertex);
}

/*
 *  Ajoute une face au mesh
 */
extern MeshFace *MESH_AddFace(Mesh *mesh, MeshFace *face) {
  return ARRLISTP_Add(mesh->faces, face);
}

/*
 * Ajoute plusieurs face au mesh
 */
extern void MESH_AddFaces(Mesh *mesh, MeshFace **faces, unsigned nbFaces) {
  for (unsigned i = 0; i < nbFaces; i++)
    MESH_AddFace(mesh, faces[i]);
}

/* Definit le nom de la mesh */
extern void MESH_SetName(Mesh *mesh, const char *name) {
  mesh->name = realloc(mesh->name, strlen(name) + 1);
  strcpy(mesh->name, name);
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

extern void MESH_CalcVerticesNormales(Mesh *mesh) {
  static MeshVertex *v;
  static MeshFace *f;
  for (size_t iv = 0; iv < MESH_GetNbVertice(mesh); iv++) {
    v = MESH_GetVertex(mesh, iv);
    VECT_Cpy(&v->normal, &VECT_0);
    // On somme les normales des faces qui contiennent v;
    for (size_t i = 0; i < MESH_GetNbFace(mesh); i++) {
      f = MESH_GetFace(mesh, i);
      if (v == f->p0 || v == f->p1 || v == f->p2)
        VECT_Add(&v->normal, &v->normal, &f->normal);
    }
    VECT_Normalise(&v->normal);
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

extern void MESH_FACE_CalcNormaleFace(struct MeshFace *f) {
  Vector s21, s31;
  VECT_Sub(&s21, &f->p1->world, &f->p0->world);
  VECT_Sub(&s31, &f->p2->world, &f->p0->world);
  VECT_CrossProduct(&f->normal, &s21, &s31);
  VECT_Normalise(&f->normal);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

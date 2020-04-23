/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "mesh.h"
#include "color.h"
#include "geo.h"
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
 * Initialise un tétrahedre a partir d'un fichier
 */
struct Mesh *MESH_Init(const char *namefile) {
  struct Mesh *ret = malloc(sizeof(struct Mesh));
  ; // TODO
  return ret;
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
 * Initialise un tetrahedre
 * https://en.wikipedia.org/wiki/Tetrahedron
 */
struct Mesh *MESH_InitTetrahedron(struct Vector *origin) {
  struct Mesh *p = malloc(sizeof(struct Mesh));
  p->nb_vertices = 4;
  p->vertices = malloc(sizeof(struct Vector) * p->nb_vertices);
  VECT_Set(&p->vertices[0], 0, 0, 0);
  VECT_Set(&p->vertices[1], 1, 0, 0);
  VECT_Set(&p->vertices[2], 0, 1, 0);
  VECT_Set(&p->vertices[3], 0, 0, 1);

  for (unsigned int i = 0; i < 4; i++) {
    VECT_Add(&p->vertices[i], &p->vertices[i], origin);
  }

  p->nb_faces = 4;
  p->faces = malloc(sizeof(struct MeshFace) * p->nb_faces);
  MESH_FACE_Set(&p->faces[0], &p->vertices[0], &p->vertices[1], &p->vertices[2],
                CL_rgb(0, 0, 255));
  MESH_FACE_Set(&p->faces[1], &p->vertices[0], &p->vertices[1], &p->vertices[3],
                CL_rgb(0, 255, 0));
  MESH_FACE_Set(&p->faces[2], &p->vertices[0], &p->vertices[2], &p->vertices[3],
                CL_rgb(255, 0, 0));
  MESH_FACE_Set(&p->faces[3], &p->vertices[1], &p->vertices[2], &p->vertices[3],
                CL_rgb(100, 100, 100));
  return p;
}

void MESH_Print(struct Mesh *mesh) {
  printf("NB TR = %d\n", mesh->nb_faces);
  for (unsigned int i_face = 0; i_face < mesh->nb_faces; i_face++) {
    printf("TR[%d]:{", i_face);
    VECT_Print(mesh->faces[i_face].p0);
    VECT_Print(mesh->faces[i_face].p1);
    VECT_Print(mesh->faces[i_face].p2);
    printf("}\n");
  }
}

/*
 * Translate le mesh suivant le vecteur depl
 */
void MESH_Translate(struct Mesh *mesh, struct vecteur *depl) {
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

/*
 * geo.c
 *
 *  Created on: 21/04/2020
 *      Author: RAVENEL Pierre
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "geo.h"
#include <math.h>
#include <stdint.h>
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

const struct Vector VECT_0 = {0, 0, 0};
const struct Vector VECT_X = {1, 0, 0};
const struct Vector VECT_Y = {0, 1, 0};
const struct Vector VECT_Z = {0, 0, 1};

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 * Initialisation d'un vecteur statique
 */
struct Vector *VECT_SetStatic(double x, double y, double z) {
  static struct Vector v;
  v.x = x;
  v.y = y;
  v.z = z;
  return &v;
}

/*
 * Initialisation de vecteur
 */
struct Vector *VECT_Set(struct Vector *v, double x, double y, double z) {
  v->x = x;
  v->y = y;
  v->z = z;
  return v;
}

/*
 * Copie de vecteur
 */
struct Vector *VECT_Cpy(struct Vector *dest, const struct Vector *src) {
  return memcpy(dest, src, sizeof(struct Vector));
}

/*
 * Comparaison de vecteur
 */
char VECT_Eq(const struct Vector *a, const struct Vector *b) {
  return a->x == b->x && a->y == b->y && a->z == b->z;
}

/*
 * Distance au carre entre deux points
 */
float VECT_DistanceSquare(const struct Vector *a, const struct Vector *b) {
  static double d1, d2, d3;
  d1 = (a->x - b->x);
  d2 = (a->y - b->y);
  d3 = (a->z - b->z);
  return d1 * d1 + d2 * d2 + d3 * d3;
}

/*
 * Distance entre deux points
 */
float VECT_Distance(const struct Vector *a, const struct Vector *b) {
  return sqrt(VECT_DistanceSquare(a, b));
}

/*
 * Affichage vecteur
 */
void VECT_Print(struct Vector *a) { printf("{%f, %f, %f}", a->x, a->y, a->z); }

/*
 * Somme vecteur : a := a + b
 */
struct Vector *VECT_Add(struct Vector *dest, const struct Vector *a,
                        const struct Vector *b) {
  dest->x = a->x + b->x;
  dest->y = a->y + b->y;
  dest->z = a->z + b->z;
  return dest;
}

/*
 * Somme vecteur : a := a - b
 */
struct Vector *VECT_Sub(struct Vector *dest, const struct Vector *a,
                        const struct Vector *b) {
  dest->x = a->x - b->x;
  dest->y = a->y - b->y;
  dest->z = a->z - b->z;
  return dest;
}

/*
 * SOustraction vecteur : a := a - b
 */
struct Vector *VECT_MultSca(struct Vector *dest, const struct Vector *a,
                            double lambda) {
  dest->x = a->x * lambda;
  dest->y = a->y * lambda;
  dest->z = a->z * lambda;
  return dest;
}

/*
 * Produit scalaire
 */
double VECT_DotProduct(const struct Vector *a, const struct Vector *b) {
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

/*
 * Produit vectorielle
 */
struct Vector *VECT_CrossProduct(struct Vector *dest, const struct Vector *a,
                                 const struct Vector *b) {
  dest->x = a->y * b->z - a->z * b->y;
  dest->y = a->z * b->x - a->x * b->z;
  dest->z = a->x * b->y - a->y * b->x;
  return dest;
}

/*
 * Norme au carré
 */
double VECT_NormSquare(const struct Vector *v) {
  return v->x * v->x + v->y * v->y + v->z * v->z;
}

/*
 * Vecteur normalisé
 */
struct Vector *VECT_Normalise(struct Vector *v) {
  return VECT_MultSca(v, v, 1 / sqrt(VECT_NormSquare(v)));
}

/*
 * Angle entre deux vecteurs
 */
/*
double VECT_Angle(const struct Vector *a, const struct Vector *b) {
  return 0.f;
}
*/

/*
 * RayIntersectsTriangle
 * https://fr.wikipedia.org/wiki/Algorithme_d%27intersection_de_M%C3%B6ller%E2%80%93Trumbore
 */
bool RayIntersectsTriangle(const struct Vector *rayOrigin,
                           const struct Vector *rayVector,
                           const struct Vector *trpoint0,
                           const struct Vector *trpoint1,
                           const struct Vector *trpoint2,
                           struct Vector *outIntersectionPoint) {
  const double EPSILON = 0.0000001;

  static struct Vector edge1, edge2, h, s, q, rab;
  static double a, f, u, v;

  VECT_Sub(&edge1, trpoint1, trpoint0);
  VECT_Sub(&edge2, trpoint2, trpoint0);
  VECT_CrossProduct(&h, rayVector, &edge2);
  a = VECT_DotProduct(&edge1, &h);

  if (a > -EPSILON && a < EPSILON)
    return false; // Le rayon est parallèle au triangle.

  f = 1.0 / a;
  VECT_Sub(&s, rayOrigin, trpoint0);

  u = f * VECT_DotProduct(&s, &h);
  if (u < 0.0 || u > 1.0)
    return false;

  VECT_CrossProduct(&q, &s, &edge1);
  v = f * VECT_DotProduct(rayVector, &q);
  if (v < 0.0 || u + v > 1.0)
    return false;

  // On calcule t pour savoir ou le point d'intersection se situe sur la ligne.

  float t = f * VECT_DotProduct(&edge2, &q);
  if (t > EPSILON) // Intersection avec le rayon
  {
    VECT_Add(outIntersectionPoint, rayOrigin, VECT_MultSca(&rab, rayVector, t));
    return true;
  } else // On a bien une intersection de droite, mais pas de rayon.
    return false;
}

void VECT_test(void) {
  /*
   p2
    |\
    | \
    |  \
    |___\
    p0   p1
  */

  struct Vector p0 = {0, 0, 0};
  struct Vector p1 = {1, 0, 0};
  struct Vector p2 = {0, 1, 0};

  const struct Vector INTERSECT_POINT = {0.01, 0.2, 0};

  struct Vector cam_pos = {0, 0, 10};
  struct Vector cam_vect;
  VECT_Sub(&cam_vect, &INTERSECT_POINT, &cam_pos);

  struct Vector intersect_point_result = {0, 0, 0};
  bool res = RayIntersectsTriangle(&cam_pos, &cam_vect, &p0, &p1, &p2,
                                   &intersect_point_result);
  VECT_Print(&intersect_point_result);
  printf("RES %d", res);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

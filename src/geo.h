/*
 *  window.h
 *
 *  Created on: 21/04/2020
 *      Author: RAVENEL P
 */

#ifndef _GEO_H_
#define _GEO_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

struct Vector {
  double x;
  double y;
  double z;
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*
 * Initialisation d'un vecteur statique
 */
struct Vector *VECT_SetStatic(double x, double y, double z);

/*
 * Initialisation de vecteur
 */
struct Vector *VECT_Set(struct Vector *v, double x, double y, double z);

/*
 * Copie de vecteur
 */
struct Vector *VECT_Cpy(struct Vector *dest, const struct Vector *src);

/*
 * Comparaison de vecteur
 */
char VECT_Eq(const struct Vector *a, const struct Vector *b);

/*
 * Distance au carre entre deux points
 */
float VECT_DistanceSquare(const struct Vector *a, const struct Vector *b);

/*
 * Distance entre deux points
 */
float VECT_Distance(const struct Vector *a, const struct Vector *b);

/*
 * Affichage vecteur
 */
void VECT_Print(struct Vector *a);
/*
 * Somme vecteur : a := a + b
 */
struct Vector *VECT_Add(struct Vector *dest, const struct Vector *a,
                        const struct Vector *b);

/*
 * Somme vecteur : a := a - b
 */
struct Vector *VECT_Sub(struct Vector *dest, const struct Vector *a,
                        const struct Vector *b);

/*
 * SOustraction vecteur : a := a - b
 */
struct Vector *VECT_MultSca(struct Vector *dest, const struct Vector *a,
                            double lambda);

/*
 * Produit scalaire
 */
double VECT_DotProduct(const struct Vector *a, const struct Vector *b);

/*
 * Produit vectorielle
 */
struct Vector *VECT_CrossProduct(struct Vector *dest, const struct Vector *a,
                                 const struct Vector *b);

/*
 * Norme au carré
 */
double VECT_NormSquare(const struct Vector *v);

/*
 * Vecteur normalisé
 */
struct Vector *VECT_Normalise(struct Vector *v);

/*
 * Angle entre deux vecteurs
 */
double VECT_Angle(const struct Vector *a, const struct Vector *b);
/*
 * RayIntersectsTriangle
 * https://fr.wikipedia.org/wiki/Algorithme_d%27intersection_de_M%C3%B6ller%E2%80%93Trumbore
 */
bool RayIntersectsTriangle(const struct Vector *rayOrigin,
                           const struct Vector *rayVector,
                           const struct Vector *trpoint0,
                           const struct Vector *trpoint1,
                           const struct Vector *trpoint2,
                           struct Vector *outIntersectionPoint);
void VECT_test(void);

#endif /* _GEO_H_ */

#ifndef _VECTOR_H_
#define _VECTOR_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/
struct ArrayList;
typedef struct ArrayList ArrayList;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Cree une liste pour des elements de taille definie */
ArrayList *ARRLIST_Create(size_t elementSize);
/* Libere la liste et les tableau de donnees (le tableau retourne par
 * ARRLIST_GetData est invalide) */
void ARRLIST_Free(ArrayList *list);

/* Ajout d'un element a la liste */
size_t ARRLIST_Add(ArrayList *list, void *element);
/* Acces a un element de la liste via son indice */
void *ARRLIST_Get(const ArrayList *list, size_t index);
/* Retourne le nombre d'elements de la liste */
size_t ARRLIST_GetSize(const ArrayList *list);

/* Redimensionne la liste pour ne pas avoir de memoire superflue */
size_t ARRLIST_Fit(ArrayList *list);
/* Retourne le tableau brute de donnees. La structure ArrayList garde la main
 * sur ce tableau */
void *ARRLIST_GetData(ArrayList *list);
/* Definie la capacite de l'ArrayList manuellement */
void ARRLIST_SetCapacity(ArrayList *list, size_t capacity);

#endif /* _VECTOR_H_ */

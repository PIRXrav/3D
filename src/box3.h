#ifndef _BOX3_H_
#define _BOX3_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "geo.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

struct Box3 {
  Vector min;
  Vector max;
  uint32_t cpt;  // Le nombre de points dans la boite (assure non vide)
  Vector center; // Le centre de la boite
};

typedef struct Box3 Box3;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void BOX3_Reset(Box3 *b);

bool BOX3_IsPointInside(Box3 *b, Vector *point);

void BOX3_AddPoint(Box3 *b, Vector *point);

#endif /* _BOX3_H_ */

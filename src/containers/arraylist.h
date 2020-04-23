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

ArrayList *ARRLIST_Create(size_t elementSize);
void ARRLIST_Free(ArrayList *list);

size_t ARRLIST_Add(ArrayList *list, void *element);
void *ARRLIST_Get(const ArrayList *list, size_t index);
size_t ARRLIST_GetSize(const ArrayList *list);

size_t ARRLIST_Fit(ArrayList *list);
void *ARRLIST_GetData(ArrayList *list);
void ARRLIST_SetCapacity(ArrayList *list, size_t capacity);

#endif /* _VECTOR_H_ */

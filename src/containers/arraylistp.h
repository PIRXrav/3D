#ifndef _ARRAYLISTP_H_
#define _ARRAYLISTP_H_

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

extern ArrayList *ARRLISTP_Create(void);
extern void ARRLISTP_Free(ArrayList *list);
extern void *ARRLISTP_Add(ArrayList *list, void *element);
extern void *ARRLISTP_Get(const ArrayList *list, size_t index);
extern void ARRLISTP_Clear(ArrayList *list);
extern int ARRLISTP_Search(const ArrayList *list, const void *element,
                           int (*eq)(const void *, const void *));
extern size_t ARRLISTP_GetSize(const ArrayList *list);
extern void *ARRLISTP_ToArray(ArrayList *list);

#endif /* _ARRAYLISTP_H_ */

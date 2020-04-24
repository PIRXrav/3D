/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "arraylistp.h"
#include "assert.h"
#include <string.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/
struct ArrayList {
  void **data;
  size_t capacity, number;
  size_t elementSize;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static size_t ARRLISTP_Fit(ArrayList *list);
static void ARRLISTP_SetCapacity(ArrayList *list, size_t capacity);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

extern ArrayList *ARRLISTP_Create(void) {
  ArrayList *list = malloc(sizeof(ArrayList));
  list->number = 0;
  list->data = NULL;
  ARRLISTP_SetCapacity(list, 10);
  return list;
}

extern void ARRLISTP_Free(ArrayList *list) {
  free(list->data);
  free(list);
}

extern void *ARRLISTP_Add(ArrayList *list, void *element) {
  if (list->number == list->capacity)
    ARRLISTP_SetCapacity(list, list->capacity * 2);
  // memcpy(list->data[list->number], element, ) list->data[list->number] =
  //    element;
  list->data[list->number] = element;
  return ARRLISTP_Get(list, list->number++);
}

extern void *ARRLISTP_Get(const ArrayList *list, size_t index) {
  assert(index < list->number);
  return list->data[index];
}

extern void ARRLISTP_Clear(ArrayList *list) { list->number = 0; }

extern int ARRLISTP_Search(const ArrayList *list, const void *element,
                           int (*eq)(const void *, const void *)) {
  for (size_t i = 0; i < list->number; i++) {
    if (eq(element, list->data[i]))
      return i;
  }
  return -1;
}

extern size_t ARRLISTP_GetSize(const ArrayList *list) { return list->number; }
// void *ARRLISTP_GetData(ArrayList *list) { return list->data; }

extern void *ARRLISTP_ToArray(ArrayList *list) {
  ARRLISTP_Fit(list);
  void *data = list->data;
  free(list);
  return data;
}
/*******************************************************************************
 * Internal function
 ******************************************************************************/

static size_t ARRLISTP_Fit(ArrayList *list) {
  ARRLISTP_SetCapacity(list, list->number ? list->number : 1);
  return list->number;
}

static void ARRLISTP_SetCapacity(ArrayList *list, size_t capacity) {
  assert(capacity != 0 && capacity >= list->number);

  void *new = reallocarray(list->data, capacity, sizeof(void *));
  assert(new);

  list->data = new;
  list->capacity = capacity;
}

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "arraylist.h"
#include "assert.h"
#include <string.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/
struct ArrayList {
  void *data;
  unsigned int capacity, number;
  size_t elementSize;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

ArrayList *ARRLIST_Create(size_t elementSize) {
  ArrayList *list = malloc(sizeof(ArrayList));
  list->number = 0;
  list->elementSize = elementSize;

  list->data = NULL;
  ARRLIST_SetCapacity(list, 10);
  return list;
}

void ARRLIST_Free(ArrayList *list) {
  free(list->data);
  free(list);
}

size_t ARRLIST_Fit(ArrayList *list) {
  ARRLIST_SetCapacity(list, list->number ? list->number : 1);
  return list->number;
}

void ARRLIST_SetCapacity(ArrayList *list, size_t capacity) {
  assert(capacity != 0 && capacity >= list->number);

  void *new = reallocarray(list->data, capacity, list->elementSize);
  assert(new);

  list->data = new;
  list->capacity = capacity;
}

size_t ARRLIST_Add(ArrayList *list, void *element) {
  if (list->number == list->capacity)
    ARRLIST_SetCapacity(list, list->capacity * 2);

  memcpy(list->data + list->number * list->elementSize, element,
         list->elementSize);
  return ++list->number;
}

void *ARRLIST_Get(const ArrayList *list, size_t index) {
  assert(index < list->number);
  return list->data + list->elementSize * index;
}
size_t ARRLIST_GetSize(const ArrayList *list) { return list->number; }
void *ARRLIST_GetData(ArrayList *list) { return list->data; }

/*******************************************************************************
 * Internal function
 ******************************************************************************/

#include "containers/arraylist.h"
#include <assert.h>

typedef struct Data {
  int x, y;
} Data;

int main() {
  ArrayList *list = ARRLIST_Create(sizeof(Data));

  for (int i = 0; i < 20; i++) {
    Data x = {i, i};
    ARRLIST_Add(list, &x);
  }
  ARRLIST_SetCapacity(list, 100);
  ARRLIST_Fit(list);

  for (int i = 0; i < ARRLIST_GetSize(list); i++) {
    Data *x = ARRLIST_Get(list, i);
    assert(x->x == i && x->y == i);
  }

  ARRLIST_Clear(list);
  assert(!ARRLIST_GetSize(list));

  ARRLIST_Free(list);

  return 0;
}

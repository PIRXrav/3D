#include "containers/arraylist.h"
#include <assert.h>

typedef struct Data {
  unsigned x, y;
} Data;

int main() {
  ArrayList *list = ARRLIST_Create(sizeof(Data));

  for (int i = 0; i < 20; i++) {
    Data x = {i, i};
    ARRLIST_Add(list, &x);
  }
  ARRLIST_SetCapacity(list, 100);
  ARRLIST_Fit(list);

  for (unsigned i = 0; i < ARRLIST_GetSize(list); i++) {
    Data *x = ARRLIST_Get(list, i);
    assert(x->x == i && x->y == i);
  }

  Data s = {10, 10};

  assert(ARRLIST_Search(list, &s) == 10);
  s.x = s.y = 14;
  assert(ARRLIST_Search(list, &s) == 14);
  s.x = 15;
  assert(ARRLIST_Search(list, &s) == -1);
  s.x = s.y = ARRLIST_GetSize(list);
  assert(ARRLIST_Search(list, &s) == -1);

  ARRLIST_Clear(list);
  assert(!ARRLIST_GetSize(list));

  ARRLIST_Free(list);

  return 0;
}

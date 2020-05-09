/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "matrix.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

/*******************************************************************************
 * Public function
 ******************************************************************************/

Matrix *MATRIX_Init(uint32_t xmax, uint32_t ymax, uint32_t elemsize, char *ss) {
  Matrix *m = malloc(sizeof(Matrix));
  assert(m);
  m->xmax = xmax;
  m->ymax = ymax;
  m->elemsize = elemsize;
  m->data = malloc(elemsize * xmax * ymax);
  assert(m->data);
  m->strtype = ss;
  return m;
}

void *MATRIX_Edit(Matrix *m, uint32_t x, uint32_t y) {
  assert(x < m->xmax && y < m->ymax);
  return m->data + (y * m->xmax + x) * m->elemsize;
}

void MATRIX_Clear(Matrix *m) {
  printf("TODO : %p\n", m);
  exit(1);
}

void MATRIX_Free(Matrix *m) {
  free(m->data);
  free(m);
}

void *MATRIX_Max(Matrix *m, int (*isgreater)(void *, void *)) {
  void *max = m->data;
  for (size_t x = 0; x < m->xmax; x++) {
    for (size_t y = 0; y < m->ymax; y++) {
      void *elem = m->data + (y * m->xmax + x) * m->elemsize;
      if (isgreater(max, elem))
        max = elem;
    }
  }
  return max;
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

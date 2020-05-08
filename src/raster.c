/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "raster.h"
#include "color.h"
#include <assert.h>
#include <stdint.h>
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

void swap_pos(RasterPos **p1, RasterPos **p2);

// TODO: optimiser en ecrivant tout d'un bloc
static void RASTER_DrawHorizontalLine(uint32_t x1, uint32_t x2, uint32_t y,
                                      void (*callbackxy)(uint32_t, uint32_t,
                                                         void *),
                                      void *args);

/**
 * http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html#algo2
 */
static void RASTER_DrawFillBottomFlatTriangle(
    RasterPos *p1, RasterPos *p2, RasterPos *p3,
    void (*callbackxy)(uint32_t, uint32_t, void *), void *args);

static void
RASTER_DrawFillTopFlatTriangle(RasterPos *p1, RasterPos *p2, RasterPos *p3,
                               void (*callbackxy)(uint32_t, uint32_t, void *),
                               void *args);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

void RP_Cpy(RasterPos *p1, RasterPos *p2) {
  p1->x = p2->x;
  p1->y = p2->y;
}

extern struct Raster *RASTER_Init(uint32_t xmax, uint32_t ymax) {
  struct Raster *s = malloc(sizeof(struct Raster));
  assert(s);
  s->screen = malloc(sizeof(color) * xmax * ymax);
  assert(s->screen);
  s->xmax = xmax;
  s->ymax = ymax;

  return s;
}

extern void RASTER_DrawFill(struct Raster *s, color c) {
  for (uint32_t x = 0; x < s->xmax; x++) {
    for (uint32_t y = 0; y < s->ymax; y++) {
      s->screen[y * s->xmax + x] = c;
    }
  }
}

extern void RASTER_DrawPixel(struct Raster *s, RasterPos p, color c) {
  if (p.x < s->xmax && p.y < s->ymax)
    s->screen[p.y * s->xmax + p.x] = c;
}

extern void RASTER_DrawPixelxy(struct Raster *s, uint32_t x, uint32_t y,
                               color c) {
  if (x < s->xmax && y < s->ymax)
    s->screen[y * s->xmax + x] = c;
}

extern color RASTER_GetPixel(struct Raster *s, RasterPos p) {
  assert(p.x < s->xmax && p.y < s->ymax);
  return s->screen[p.y * s->xmax + p.x];
}

extern color RASTER_GetPixelxy(struct Raster *s, uint32_t x, uint32_t y) {
  assert(x < s->xmax && y < s->ymax);
  return s->screen[y * s->xmax + x];
}

/*
 * https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 */
extern void RASTER_DrawLine(struct Raster *s, RasterPos *p0, RasterPos *p1,
                            color c) {
  int32_t x0 = p0->x, x1 = p1->x, y0 = p0->y, y1 = p1->y, dx, dy, sx, sy, err,
          e2;
  dx = abs(x1 - x0);
  sx = x0 < x1 ? 1 : -1;
  dy = -abs(y1 - y0);
  sy = y0 < y1 ? 1 : -1;
  err = dx + dy; /* error value e_xy */
  while (1) {
    RASTER_DrawPixelxy(s, (uint32_t)x0, (uint32_t)y0, c);
    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy; /* e_xy+e_x > 0 */
      x0 += sx;
    }
    if (e2 <= dx) { /* e_xy+e_y < 0 */
      err += dx;
      y0 += sy;
    }
  }
}

extern void RASTER_DrawTriangle(struct Raster *s, RasterPos *p1, RasterPos *p2,
                                RasterPos *p3, color c) {
  RASTER_DrawLine(s, p1, p2, c);
  RASTER_DrawLine(s, p2, p3, c);
  RASTER_DrawLine(s, p3, p1, c);
}

extern void
RASTER_GenerateFillTriangle(RasterPos *p1, RasterPos *p2, RasterPos *p3,
                            void (*callbackxy)(uint32_t, uint32_t, void *),
                            void *args) {
  // On trie p1, p2, p3 par ordre ascendant en ordonnees
  if (p1->y > p2->y) {
    if (p2->y > p3->y)      // p3, p2, p1
      swap_pos(&p1, &p3);   // Ordered
    else if (p1->y > p3->y) // p2, p3, p1
    {
      RasterPos *tmp = p1;
      p1 = p2;
      p2 = p3;
      p3 = tmp;
    } else // p2, p1, p3
    {
      swap_pos(&p1, &p2);
    }
  } else // p2 > p1
  {
    if (p1->y > p3->y) // p3, p1, p2
    {
      RasterPos *tmp = p1;
      p1 = p3;
      p3 = p2;
      p2 = tmp;
    } else if (p2->y > p3->y) // p1, p3, p2
      swap_pos(&p2, &p3);
    // p1, p2, p3
  }

  // On a p1.y <= p2.y <= p3.y
  // Cas triviaux :
  // Triangle plat en bas
  if (p2->y == p3->y)
    RASTER_DrawFillBottomFlatTriangle(p1, p2, p3, callbackxy, args);
  // Triangle plat en haut
  else if (p1->y == p2->y) {
    RASTER_DrawFillTopFlatTriangle(p1, p2, p3, callbackxy, args);
  } else {
    // Cas general n split le triangle en deux triangles plats
    float x1 = p1->x, y1 = p1->y, y2 = p2->y, x3 = p3->x, y3 = p3->y;
    RasterPos intersection = {
        (uint32_t)(x1 + ((y2 - y1) / (y3 - y1)) * (x3 - x1)), (uint32_t)y2};
    RASTER_DrawFillBottomFlatTriangle(p1, p2, &intersection, callbackxy, args);
    RASTER_DrawFillTopFlatTriangle(p2, &intersection, p3, callbackxy, args);
  }
}

extern void RASTER_DrawCircle(struct Raster *s, RasterPos *p, int r, color c) {
  int xc = p->x;
  int yc = p->y;
  int x = 0;
  int y = r;
  int m = 5 - 4 * r;
  while (x <= y) {
    RASTER_DrawPixelxy(s, xc + x, yc + y, c);
    RASTER_DrawPixelxy(s, xc - x, yc + y, c);
    RASTER_DrawPixelxy(s, xc + x, yc - y, c);
    RASTER_DrawPixelxy(s, xc - x, yc - y, c);
    RASTER_DrawPixelxy(s, xc + y, yc + x, c);
    RASTER_DrawPixelxy(s, xc - y, yc + x, c);
    RASTER_DrawPixelxy(s, xc + y, yc - x, c);
    RASTER_DrawPixelxy(s, xc - y, yc - x, c);
    if (m > 0) {
      y--;
      m = m - 8 * y;
    };
    x++;
    m = m + 8 * x + 4;
  }
}
/*
https://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles

int r2 = r * r;
int area = r2 << 2;
int rr = r << 1;

for (int i = 0; i < area; i++)
{
    int tx = (i % rr) - r;
    int ty = (i / rr) - r;

    if (tx * tx + ty * ty <= r2)
        SetPixel(x + tx, y + ty, c);
}
*/

/*******************************************************************************
 * Internal function
 ******************************************************************************/
inline void swap_pos(RasterPos **p1, RasterPos **p2) {
  RasterPos *tmp = *p1;
  *p1 = *p2;
  *p2 = tmp;
}

// TODO: optimiser en ecrivant tout d'un bloc
static void RASTER_DrawHorizontalLine(uint32_t x1, uint32_t x2, uint32_t y,
                                      void (*callbackxy)(uint32_t, uint32_t,
                                                         void *),
                                      void *args) {
  // swap si pas en ordre
  if (x1 > x2) {
    x1 ^= x2;
    x2 ^= x1;
    x1 ^= x2;
  }

  for (uint32_t i = x1; i <= x2; i++) {
    callbackxy(i, y, args);
    // printf("Writing %u %u\n", i, y);
  }
}
/**
 * http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html#algo2
 */
static void RASTER_DrawFillBottomFlatTriangle(
    RasterPos *p1, RasterPos *p2, RasterPos *p3,
    void (*callbackxy)(uint32_t, uint32_t, void *), void *args) {

  float x1 = p1->x, y1 = p1->y, x2 = p2->x, y2 = p2->y, x3 = p3->x, y3 = p3->y;

  float invslope1 = (x2 - x1) / (y2 - y1);
  float invslope2 = (x3 - x1) / (y3 - y1);

  float curx1 = x1;
  float curx2 = x1;

  for (uint32_t scanlineY = y1; scanlineY <= y2; scanlineY++) {
    RASTER_DrawHorizontalLine((int)curx1, (int)curx2, scanlineY, callbackxy,
                              args);
    curx1 += invslope1;
    curx2 += invslope2;
  }
}

static void
RASTER_DrawFillTopFlatTriangle(RasterPos *p1, RasterPos *p2, RasterPos *p3,
                               void (*callbackxy)(uint32_t, uint32_t, void *),
                               void *args) {

  float x1 = p1->x, y1 = p1->y, x2 = p2->x, y2 = p2->y, x3 = p3->x, y3 = p3->y;
  float invslope1 = (x3 - x1) / (y3 - y1);
  float invslope2 = (x3 - x2) / (y3 - y2);

  float curx1 = x3;
  float curx2 = x3;

  for (uint32_t scanlineY = y3; scanlineY >= y1; scanlineY--) {
    RASTER_DrawHorizontalLine((int)curx1, (int)curx2, scanlineY, callbackxy,
                              args);
    curx1 -= invslope1;
    curx2 -= invslope2;
  }
}

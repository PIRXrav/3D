#ifndef _RASTER_H_
#define _RASTER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "color.h"
#include "geo.h"
#include <stdint.h>
#include <stdlib.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

struct Raster {
  uint32_t xmax;
  uint32_t ymax;
  color *screen;
};

struct RasterPos {
  uint32_t x;
  uint32_t y;
};

typedef struct RasterPos RasterPos;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

struct Raster *RASTER_Init(uint32_t xmax, uint32_t ymax);

color RASTER_GetPixel(struct Raster *s, RasterPos p);
color RASTER_GetPixelxy(struct Raster *s, uint32_t x, uint32_t y);

void RASTER_DrawFill(struct Raster *s, color c);

void RASTER_DrawPixel(struct Raster *s, RasterPos p, color c);
void RASTER_DrawPixelxy(struct Raster *s, uint32_t x, uint32_t y, color c);

void RASTER_DrawLine(struct Raster *s, RasterPos *p1, RasterPos *p2, color c);

void RASTER_DrawTriangle(struct Raster *s, RasterPos *p1, RasterPos *p2,
                         RasterPos *p3, color c);

void RASTER_DrawCircle(struct Raster *s, RasterPos *p, int r, color c);

void RASTER_GenerateFillTriangle(RasterPos *p1, RasterPos *p2, RasterPos *p3,
                                 void (*callbackxy)(uint32_t, uint32_t, void *),
                                 void *args);

#endif /* _RASTER_H_ */

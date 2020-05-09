#ifndef _RASTER_H_
#define _RASTER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "color.h"
#include "containers/matrix.h"
#include "geo.h"

#include <stdint.h>
#include <stdlib.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

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

color RASTER_GetPixel(Matrix *s, RasterPos p);
color RASTER_GetPixelxy(Matrix *s, uint32_t x, uint32_t y);

void RASTER_DrawFill(Matrix *s, color c);

void RASTER_DrawPixel(Matrix *s, RasterPos p, color c);
void RASTER_DrawPixelxy(Matrix *s, uint32_t x, uint32_t y, color c);

void RASTER_DrawLine(Matrix *s, RasterPos *p1, RasterPos *p2, color c);

void RASTER_DrawTriangle(Matrix *s, RasterPos *p1, RasterPos *p2, RasterPos *p3,
                         color c);

void RASTER_DrawCircle(Matrix *s, RasterPos *p, int r, color c);

void RASTER_GenerateFillTriangle(RasterPos *p1, RasterPos *p2, RasterPos *p3,
                                 void (*callbackxy)(uint32_t, uint32_t,
                                                    void **),
                                 void **args);

#endif /* _RASTER_H_ */

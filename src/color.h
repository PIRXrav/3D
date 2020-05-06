#ifndef _COLOR_H_
#define _COLOR_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "colornames.h"
#include <stdint.h>
#include <stdlib.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define IS_BIG_ENDIAN 0

#define CL_ALPHA_OPAQUE 0xFF
#define CL_ALPHA_TRANSPARENT 0

/*******************************************************************************
 * Types
 ******************************************************************************/

struct color_rgb {
  uint8_t b, g, r, a;
};

union color {
  uint32_t raw;
  struct color_rgb rgb;
};

typedef union color color;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

color CL_rgb(uint8_t r, uint8_t g, uint8_t b);

color CL_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

color CL_Random();

color CL_Negate(color c);

/**
 * Retourne une couleur interpolee entre c1 et c2 en fonction de factor,
 * retourne c1 pour factor = 0 et c2 pour factor = 1
 */
color CL_Mix(color c1, color c2, float factor);

#endif /* _COLOR_H_ */

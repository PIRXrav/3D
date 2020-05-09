/*
 *  window.h
 *
 *  Created on: 21/04/2020
 *      Author: RAVENEL P
 */

#ifndef _WINDOW_H_
#define _WINDOW_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "color.h"
#include "raster.h"
#include <stdint.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

struct hwindow;

typedef enum {
  EVENT_EMPTY,
  EVENT_KEYBOARD,
  EVENT_MOUSE,
  EVENT_WINDOW
} event_type;

struct event_keyboard_data {
  int code;
  unsigned state;
};

struct event_mouse_data {
  unsigned int x, y;
  unsigned int dx, dy;
  unsigned int released;
  enum mouse_bouton {
    BUTTON_NONE = 0,
    BUTTON_RIGHT = 1,
    BUTTON_LEFT = 2,
    BUTTON_MIDDLE = 4
  } button;
};

struct event_window_data {
  unsigned quit;
};

union event_data {
  struct event_keyboard_data key;
  struct event_mouse_data mouse;
  struct event_window_data window;
};

struct event {
  event_type type;
  union event_data data;
};

typedef void (*event_callback)(const struct event *data);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 *  Initialisation d'une fenetre
 */
struct hwindow *HW_Init(const char *name, Matrix *raster);
/*
 *  Fermeture d'une fenetre
 */
void HW_Close(struct hwindow *hw);

/*
 *  Lancement du loop evenements et affichage
 */
void HW_Loop(struct hwindow *hw, void (*userfunc)(unsigned int));

/*
 * Affichage des informations de debug
 */
void HW_Print(struct hwindow *hw);

/*
 * Ajoute un callback pour un type specifique d'evenements
 */
void HW_SetCallback(struct hwindow *hw, event_type type,
                    event_callback callback);

/*
 *  get xmax
 */
unsigned int HW_GetX(struct hwindow *hw);

/*
 *  get ymax
 */
unsigned int HW_GetY(struct hwindow *hw);

#endif /* _WINDOW_H_ */

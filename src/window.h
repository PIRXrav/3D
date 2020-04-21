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

#include <stdint.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

struct hwindow;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 *  Lancement et Initialisation de la fenetre
 */
void HW_start(const char *name, unsigned int x, unsigned int y,
              void (*userfunc)(struct hwindow *));

/*
 *  Mise a jour d'un pixel dans le buffer
 */
void HW_SetPx(struct hwindow *hw, unsigned int x, unsigned int y, uint8_t r,
              uint8_t g, uint8_t b);

/*
 * Affichage des informations de debug
 */
void HW_Print(struct hwindow *hw);

/*
 *  get xmax
 */
unsigned int HW_GetX(struct hwindow *hw);

/*
 *  get ymax
 */
unsigned int HW_GetY(struct hwindow *hw);

#endif /* _WINDOW_H_ */

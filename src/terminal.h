#ifndef _TERMINAL_H_
#define _TERMINAL_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "raster.h"
#include <stdint.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 *  Initialisation d'un terminal
 */
struct tty *TTY_Init(struct Raster *raster);
/*
 *  Fermeture d'un terminal
 */
void TTY_Close(struct tty *tty);

/*
 *  Lancement du loop d'affichage
 */
void TTY_Loop(struct tty *tty, void (*userfunc)(unsigned int));

/*
 * Affichage des informations de debug
 */
void TTY_Print(struct tty *tty);

/*
 *  get largeur terminal
 */
uint32_t TTY_GetW(struct tty *tty);

/*
 *  get hauteur d'affichage du terminal
 */
uint32_t TTY_GetH(struct tty *tty);

#endif /* _TERMINAL_H */

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
 *  get largeur d'affichage terminal
 */
uint32_t TTY_GetW(struct tty *tty);

/*
 *  get hauteur d'affichage du terminal
 */
uint32_t TTY_GetH(struct tty *tty);

/*
 * Recupere la taille reelle du terminal
 */
void TTY_QuerySize(uint32_t *w, uint32_t *h);

#endif /* _TERMINAL_H */

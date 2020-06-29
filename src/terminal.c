/*
 * window.c
 *
 *  Created on: 21/04/2020
 *      Author: RAVENEL Pierre
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "terminal.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define MIN(x, y) (x > y ? y : x)
#define STDOUT_FILENO 1
// Devrait etre 58 normalement mais bon on voit large
#define PIXELS_SIZE 60

/*******************************************************************************
 * Types
 ******************************************************************************/

/*
 * Structure window
 */
struct tty {
  /* Output buffer */
  char *buffer;
  uint32_t bufferSize;
  /* Raster */
  uint32_t width, height;
  Matrix *raster; // Pointeur vers le raster
  /* State */
  int run;
  uint32_t cpt;
  RenderType renderType;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*
 *  Mise a jour du rendu
 */
static void TTY_Render(struct tty *tty);

/**
 * Permet de render la scene avec la methode TTY_RENDER_PIXELS ou
 * TTY_RENDER_ASCII
 */
static void TTY_DrawPixels_Square(struct tty *tty);
static void TTY_DrawPixels_ASCII(struct tty *tty);

/*
 * Met a jour le buffer de sortie pour la taille du terminal
 * et le le synchronise avec le buffer de stdout
 */
static void TTY_ResizeBuffer(struct tty *tty);

/*
 * Gere les signaux
 */
static void TTY_SignalReceived(int signal);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static int halted = 0;

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 *  Initialisation d'un terminal
 */
extern struct tty *TTY_Init(Matrix *raster, RenderType renderType) {
  struct tty *tty = malloc(sizeof(struct tty));

  tty->buffer = NULL;
  tty->bufferSize = 0;

  tty->raster = raster;
  tty->cpt = 0;
  tty->run = 1;
  TTY_QuerySize(&tty->width, &tty->height);

  tty->renderType = renderType;

  TTY_ResizeBuffer(tty);

  signal(SIGINT, TTY_SignalReceived);

  return tty;
}

/*
 *  Fermeture d'un terminal
 */
extern void TTY_Close(struct tty *tty) {
  free(tty);
  signal(SIGINT, SIG_DFL);
}

/*
 *  Lancement du loop d'affichage
 */
extern void TTY_Loop(struct tty *tty, void (*userfunc)(unsigned int)) {
  TTY_Print(tty);
  printf("\033[?1049h\033[H\033[?25l");
  while (tty->run) {
    if (halted)
      tty->run = 0;
    // Maj de la taille du terminal
    TTY_QuerySize(&tty->width, &tty->height);
    TTY_ResizeBuffer(tty);

    userfunc(tty->cpt);

    TTY_Render(tty);

    tty->cpt++;
  }
  printf("\033[?1049l\033[?25h");
}
/*
 * Affichage des informations de debug
 */
extern void TTY_Print(struct tty *tty) {
  printf("==================================\n");
  printf("TTY (%p):\n", tty);
  printf("%ux%u\n", tty->width, tty->height);
  printf("data addr : %p\n", tty->raster);
  printf("state run : %d\n", tty->run);
}

extern inline uint32_t TTY_GetW(struct tty *tty) { return tty->width; }

extern inline uint32_t TTY_GetH(struct tty *tty) { return tty->height; }

extern void TTY_QuerySize(uint32_t *w, uint32_t *h) {
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  *w = size.ws_col;
  *h = size.ws_row * 2;
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static void TTY_ResizeBuffer(struct tty *tty) {
  uint32_t maxWidth = MIN(tty->width, tty->raster->xmax);
  uint32_t maxHeight = MIN(tty->height, tty->raster->ymax);
  uint32_t newSize =
      maxWidth * ((maxHeight + 1) / 2) * PIXELS_SIZE + maxHeight * 2 + 1;
  if (tty->bufferSize != newSize) {
    tty->bufferSize = newSize;
    tty->buffer = realloc(tty->buffer, tty->bufferSize);
    assert(tty->buffer);
    assert(!setvbuf(stdout, tty->buffer, _IOFBF, tty->bufferSize));
  }
}

/*
 *  Mise a jour du rendu
 */
static void TTY_Render(struct tty *tty) {
  switch (tty->renderType) {
  case TTY_RENDER_ASCII:
    TTY_DrawPixels_ASCII(tty);
    break;
  case TTY_RENDER_PIXELS:
    TTY_DrawPixels_Square(tty);
    break;
  }
}

static void TTY_DrawPixels_ASCII(struct tty *tty) {
  static unsigned int nbChars = 69;
  static const char *chars = ".'`^\",:;Il!i><~+_-?][}{1)(|\\/"
                             "tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
  uint32_t maxWidth = MIN(tty->width, tty->raster->xmax);
  uint32_t maxHeight = MIN(tty->height, tty->raster->ymax);
  for (uint32_t y = 0; y < maxHeight; y++) {
    if (y < maxHeight) {
      for (uint32_t x = 0; x < maxWidth; x++) {

        unsigned lum = CL_Brightness(RASTER_GetPixelxy(tty->raster, x, y));
        unsigned indice = (int)((lum * nbChars) / 255);
        putc(chars[indice], stdout);
        putc(chars[indice], stdout);
      }
    }
    printf("\n");
  }
  fflush(stdout);
}

static void TTY_DrawPixels_Square(struct tty *tty) {
  uint32_t maxWidth = MIN(tty->width, tty->raster->xmax);
  uint32_t maxHeight = MIN(tty->height, tty->raster->ymax);
  for (uint32_t y = 0; y <= tty->height; y += 2) {
    if (y < maxHeight) {
      for (uint32_t x = 0; x < maxWidth; x++) {
        color bg = RASTER_GetPixelxy(tty->raster, x, y);
        color fg = y + 1 < maxHeight ? RASTER_GetPixelxy(tty->raster, x, y + 1)
                                     : CL_BLACK;

        printf("\x1b[48;2;%u;%u;%um\x1b[38;2;%u;%u;%um\u2584\x1b[0m", bg.rgb.r,
               bg.rgb.g, bg.rgb.b, fg.rgb.r, fg.rgb.g, fg.rgb.b);
      }
    }
    printf("\n");
  }
  fflush(stdout);
}

static void TTY_SignalReceived(int signal) {
  if (signal == SIGINT)
    halted = 1;
}

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
  struct Raster *raster; // Pointeur vers le raster
  /* State */
  int run;
  uint32_t cpt;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*
 *  Mise a jour du rendu
 */
static void TTY_Render(struct tty *tty);

/**
 * Permet de remplir buffer avec une paire de pixels (x, y1), (x, y2) (si y2 ==
 * -1, alors (x, y2) sera noir)
 */
static void TTY_RenderPixelPair(char *buffer, struct tty *tty, uint32_t x,
                                uint32_t y1, uint32_t y2);

/*
 * Recupere la taille reelle du terminal
 */
static void TTY_QuerySize(uint32_t *w, uint32_t *h);

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
extern struct tty *TTY_Init(struct Raster *raster) {
  struct tty *tty = malloc(sizeof(struct tty));

  tty->buffer = NULL;
  tty->bufferSize = 0;

  tty->raster = raster;
  tty->cpt = 0;
  tty->run = 1;
  TTY_QuerySize(&tty->width, &tty->height);

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
  printf("\033[?1049h\033[H\033[?25l");
  while (tty->run && tty->cpt < 1000) {
    if (halted)
      tty->run = 0;
    // Maj de la taille du terminal
    TTY_QuerySize(&tty->width, &tty->height);
    TTY_ResizeBuffer(tty);
    // TTY_Print(tty);

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
  uint32_t maxWidth = MIN(tty->width, tty->raster->xmax);
  uint32_t maxHeight = MIN(tty->height, tty->raster->ymax);
  for (uint32_t y = 0; y <= tty->height; y += 2) {
    if (y < maxHeight) {
      for (uint32_t x = 0; x < maxWidth; x++) {
        char buffer[PIXELS_SIZE];
        TTY_RenderPixelPair(buffer, tty, x, y,
                            y + 1 == maxHeight ? (uint32_t)-1 : y + 1);
        printf(buffer);
        /*for (int i = 0; i < 1500; i++)
          buffer[0] += i;*/
      }
    }
    printf("\n");
  }
  fflush(stdout);
}

static void TTY_RenderPixelPair(char *buffer, struct tty *tty, uint32_t x,
                                uint32_t y1, uint32_t y2) {
  color bg = RASTER_GetPixelxy(tty->raster, x, y1);
  color fg =
      y2 != (uint32_t)-1 ? RASTER_GetPixelxy(tty->raster, x, y2) : CL_BLACK;

  sprintf(buffer, "\x1b[48;2;%u;%u;%um\x1b[38;2;%u;%u;%um\u2584\x1b[0m",
          bg.rgb.r, bg.rgb.g, bg.rgb.b, fg.rgb.r, fg.rgb.g, fg.rgb.b);
}

static void TTY_QuerySize(uint32_t *w, uint32_t *h) {
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  *w = size.ws_col;
  *h = size.ws_row * 2;
}

static void TTY_SignalReceived(int signal) {
  if (signal == SIGINT)
    halted = 1;
}

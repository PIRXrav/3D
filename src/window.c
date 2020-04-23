/*
 * window.c
 *
 *  Created on: 21/04/2020
 *      Author: RAVENEL Pierre
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "window.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*
 * PIXEL : bgra
 */
typedef color pixel;

/*
 * Structure window
 */
struct hwindow {
  /* Config */
  const char *name;
  /* SDL */
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  /* Buffer */
  pixel *pixels;
  unsigned int width;
  unsigned int height;
  /* State */
  int run;
  double fps;
  uint32_t framecpt;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*
 *  Initialisation d'une fenetre
 */
static struct hwindow *HW_Init(const char *name, unsigned int width,
                               unsigned int height);

/*
 *  Fermeture d'une fenetre
 */
static void HW_Close(struct hwindow *hw);

/*
 *  Mise a jour du rendu
 */
static void HW_Render(struct hwindow *hw);

/*
 *  Gestion interne de la boucle d'eveneents
 */
static void HW_EventLoop(struct hwindow *hw);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 *  Lancement et Initialisation de la fenetre
 */
extern void HW_start(const char *name, unsigned int x, unsigned int y,
                     void (*userfunc)(struct hwindow *)) {
  Uint64 start;
  Uint64 freq = SDL_GetPerformanceFrequency();
  struct hwindow *hw = HW_Init(name, x, y);

  // HW_Print(hw);
  while (hw->run) {
    HW_Print(hw);
    start = SDL_GetPerformanceCounter();
    HW_EventLoop(hw);
    /******* BEGIN USER CALL  ********/

    userfunc(hw);

    /******** END USE CALL ***********/
    HW_Render(hw);
    Uint64 end = SDL_GetPerformanceCounter();
    const double seconds = (end - start) / (double)freq;
    hw->fps = 1 / seconds;
    hw->framecpt += 1;
  }
  HW_Close(hw);
}

/*
 *  Mise a jour d'un pixel dans le buffer
 */
extern void HW_SetPx(struct hwindow *hw, unsigned int x, unsigned int y,
                     color c) {
  assert(x < hw->width && y < hw->height);
  hw->pixels[(hw->width * y) + x] = c;
}

/*
 * Affichage des informations de debug
 */
extern void HW_Print(struct hwindow *hw) {
  printf("==================================\n");
  printf("HW %s:(%p)\n", hw->name, hw);
  printf("x=%d, y=%d\n", hw->width, hw->height);
  printf("data addr : %p\n", hw->pixels);
  printf("state run : %d\n", hw->run);
  printf("Frame freq: %fhz\n", hw->fps);
}

/*
 *  get xmax
 */
extern inline unsigned int HW_GetX(struct hwindow *hw) { return hw->width; }

/*
 *  get ymax
 */
extern inline unsigned int HW_GetY(struct hwindow *hw) { return hw->height; }

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*
 *  Initialisation d'une fenetre
 */
static struct hwindow *HW_Init(const char *name, unsigned int width,
                               unsigned int height) {
  struct hwindow *ret = malloc(sizeof(struct hwindow));
  assert(ret);

  ret->width = width;
  ret->height = height;
  ret->run = 1;
  ret->name = name;
  ret->framecpt = 0;
  ret->fps = -1;

  SDL_Init(SDL_INIT_EVERYTHING);

  ret->window =
      SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       width, height, SDL_WINDOW_SHOWN);

  ret->renderer = SDL_CreateRenderer(ret->window, -1, SDL_RENDERER_ACCELERATED);

  ret->texture = SDL_CreateTexture(ret->renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, width, height);

  ret->pixels = malloc(width * height * sizeof(color));
  assert(ret->pixels);
  return ret;
}

/*
 *  Fermeture d'une fenetre
 */
static void HW_Close(struct hwindow *hw) {
  SDL_DestroyTexture(hw->texture);
  SDL_DestroyRenderer(hw->renderer);
  SDL_DestroyWindow(hw->window);
  free(hw->pixels);
  free(hw);
  SDL_Quit();
}

/*
 *  Mise a jour du rendu
 */
static void HW_Render(struct hwindow *hw) {
  // SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  // SDL_RenderClear(renderer);
  SDL_UpdateTexture(hw->texture, NULL, hw->pixels, hw->width * 4);
  SDL_RenderCopy(hw->renderer, hw->texture, NULL, NULL);
  SDL_RenderPresent(hw->renderer);
}

/*
 *  Gestion interne de la boucle d'eveneents
 */
static void HW_EventLoop(struct hwindow *hw) {

  static SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if ((SDL_QUIT == event.type) ||
        (SDL_KEYDOWN == event.type &&
         SDL_SCANCODE_ESCAPE == event.key.keysym.scancode)) {
      hw->run = 0;
      break;
    }
  }
}

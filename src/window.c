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
#include "containers/arraylist.h"
#include "raster.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  unsigned int width;
  unsigned int height;
  /* Events */
  ArrayList *eventTypes;
  ArrayList *eventCallbacks;
  /* Buffer */
  struct Raster *raster; // Pointeur vers le raster
  /* State */
  int run;
  double fps;
  uint32_t framecpt;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*
 *  Mise a jour du rendu
 */
static void HW_Render(struct hwindow *hw);

/*
 *  Gestion interne de la boucle d'eveneents
 */
static void HW_EventLoop(struct hwindow *hw);

/*
 * Passe d'un bouton format SDL au format custom
 */
unsigned mouseButtonFromSDL(unsigned long buttonMaskSDL);
/*
 * Appelle les callbacks associe au type d'evenement donne
 */
void HW_SendEvent(struct hwindow *hw, const struct event *event);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 *  Initialisation d'une fenetre
 */
extern struct hwindow *HW_Init(const char *name, struct Raster *raster) {
  struct hwindow *ret = malloc(sizeof(struct hwindow));
  assert(ret);

  ret->width = raster->xmax; // On initialise la fenetre à la taille du raster
  ret->height = raster->ymax;
  ret->run = 1;
  ret->name = name;
  ret->framecpt = 0;
  ret->fps = -1;
  ret->eventTypes = ARRLIST_Create(sizeof(event_type));
  ret->eventCallbacks = ARRLIST_Create(sizeof(event_callback));

  SDL_Init(SDL_INIT_EVERYTHING);

  ret->window =
      SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       ret->width, ret->height, SDL_WINDOW_SHOWN);

  ret->renderer = SDL_CreateRenderer(ret->window, -1, SDL_RENDERER_ACCELERATED);

  ret->texture =
      SDL_CreateTexture(ret->renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, ret->width, ret->height);

  ret->raster = raster;
  assert(ret->raster);
  return ret;
}

/*
 *  Fermeture d'une fenetre
 */
void HW_Close(struct hwindow *hw) {
  SDL_DestroyTexture(hw->texture);
  SDL_DestroyRenderer(hw->renderer);
  SDL_DestroyWindow(hw->window);
  free(hw);
  SDL_Quit();
}

/*
 *  Lancement et Initialisation de la fenetre
 */
extern void HW_Loop(struct hwindow *hw, void (*userfunc)(unsigned int)) {
  Uint64 start;
  Uint64 freq = SDL_GetPerformanceFrequency();

  // HW_Print(hw);
  while (hw->run) {
    HW_Print(hw);
    start = SDL_GetPerformanceCounter();
    HW_EventLoop(hw);
    /******* BEGIN USER CALL  ********/

    userfunc(hw->framecpt);

    /******** END USE CALL ***********/
    HW_Render(hw);
    Uint64 end = SDL_GetPerformanceCounter();
    const double seconds = (end - start) / (double)freq;
    hw->fps = 1 / seconds;
    hw->framecpt += 1;
  }
}

void HW_SetCallback(struct hwindow *hw, event_type type,
                    event_callback callback) {
  ARRLIST_Add(hw->eventTypes, &type);
  ARRLIST_Add(hw->eventCallbacks, &callback);
}

/*
 * Affichage des informations de debug
 */
extern void HW_Print(struct hwindow *hw) {
  // printf("==================================\n");
  // printf("HW %s:(%p)\n", hw->name, hw);
  // printf("x=%d, y=%d\n", hw->width, hw->height);
  // printf("data addr : %p\n", hw->raster);
  // printf("state run : %d\n", hw->run);
  printf("\rFrame[%d] freq: %fhz", hw->framecpt, hw->fps);
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
 *  Mise a jour du rendu
 */
static void HW_Render(struct hwindow *hw) {
  // SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  // SDL_RenderClear(renderer);
  SDL_UpdateTexture(hw->texture, NULL, hw->raster->screen,
                    hw->raster->xmax * 4);
  SDL_RenderCopy(hw->renderer, hw->texture, NULL, NULL);
  SDL_RenderPresent(hw->renderer);
}

/*
 *  Gestion interne de la boucle d'eveneents
 */
static void HW_EventLoop(struct hwindow *hw) {

  static SDL_Event event;
  static struct event customEvent;

  while (SDL_PollEvent(&event)) {
    memset(&customEvent, 0, sizeof(customEvent));
    switch (event.type) {
    case SDL_QUIT:
      customEvent.type = EVENT_WINDOW;
      customEvent.data.window.quit = 1;
      hw->run = 0;
      break;
    case SDL_KEYDOWN:
      customEvent.data.key.state = 1;
      // fallthrough
    case SDL_KEYUP:
      customEvent.type = EVENT_KEYBOARD;
      customEvent.data.key.code = event.key.keysym.sym;
      break;
    case SDL_MOUSEMOTION:
      customEvent.type = EVENT_MOUSE;
      customEvent.data.mouse.dx = event.motion.xrel;
      customEvent.data.mouse.dy = event.motion.yrel;
      customEvent.data.mouse.x = event.motion.x;
      customEvent.data.mouse.y = event.motion.y;
      customEvent.data.mouse.button = mouseButtonFromSDL(event.motion.state);
      break;
    case SDL_MOUSEBUTTONUP:
      customEvent.data.mouse.released = 1;
      // fallthrough
    case SDL_MOUSEBUTTONDOWN:
      customEvent.type = EVENT_MOUSE;
      customEvent.data.mouse.x = event.button.x;
      customEvent.data.mouse.y = event.button.y;
      customEvent.data.mouse.button = mouseButtonFromSDL(event.button.state);
      break;
    }
    if (customEvent.type != EVENT_EMPTY)
      HW_SendEvent(hw, &customEvent);
  }
}

/*
 * Passe d'un bouton format SDL au format custom
 */
unsigned mouseButtonFromSDL(unsigned long buttonMaskSDL) {
  unsigned ret = BUTTON_NONE;
  if (buttonMaskSDL & SDL_BUTTON_LEFT)
    ret |= BUTTON_LEFT;
  if (buttonMaskSDL & SDL_BUTTON_RIGHT)
    ret |= BUTTON_RIGHT;
  if (buttonMaskSDL & SDL_BUTTON_MIDDLE)
    ret |= BUTTON_MIDDLE;
  return ret;
}

/*
 * Appelle les callbacks associe au type d'evenement donne
 */
void HW_SendEvent(struct hwindow *hw, const struct event *event) {
  int index = 0;
  while ((index = ARRLIST_SearchFrom(hw->eventTypes, index, &event->type)) !=
         -1) {
    event_callback cb =
        *(event_callback *)ARRLIST_Get(hw->eventCallbacks, index);
    cb(event);
    index++;
  }
}

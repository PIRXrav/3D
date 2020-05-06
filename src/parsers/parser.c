/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "parser.h"
#include "parser_obj.h"
#include <libgen.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define NB_PARSERS 1

/*******************************************************************************
 * Types
 ******************************************************************************/
typedef struct Mesh **(*Parser)(FILE *, unsigned *, char *);

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/
Parser getParser(const char *extension);

/*******************************************************************************
 * Variables
 ******************************************************************************/
const char *PARSERS_EXTS[] = {".obj"};
const Parser PARSERS_FUNCS[] = {OBJ_Parse};

/*******************************************************************************
 * Public function
 ******************************************************************************/
struct Mesh **PARSER_Load(const char *filename, unsigned *nbMeshes) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    perror("[PARSER_Load] Impossible d'ouvrir le fichier");
    return NULL;
  }
  char *extension = strrchr(filename, '.');
  const Parser parse = getParser(extension);
  if (!parse) {
    fprintf(stderr, "[PARSER_Load] Unknown file format : %s\n", extension);
    return NULL;
  }

  char *filenameCpy = malloc(strlen(filename) + 1);
  strcpy(filenameCpy, filename);
  char *fileDir = dirname(filenameCpy);

  struct Mesh **meshes = parse(file, nbMeshes, fileDir);

  free(filenameCpy);
  return meshes;
}
/*******************************************************************************
 * Internal function
 ******************************************************************************/
Parser getParser(const char *extension) {
  for (unsigned p = 0; p < NB_PARSERS; p++) {
    if (!strcmp(PARSERS_EXTS[p], extension))
      return PARSERS_FUNCS[p];
  }
  return NULL;
}

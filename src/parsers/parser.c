/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "parser.h"
#include "parser_obj.h"
#include <errno.h>
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
  *nbMeshes = 0;

  char *extension = strrchr(filename, '.');
  const Parser parse = getParser(extension);
  if (!parse) {
    char formats[512] = {0};
    for (unsigned i = 0; i < NB_PARSERS; i++) {
      strcat(formats, PARSERS_EXTS[i]);
      if (i != NB_PARSERS - 1)
        strcat(formats, ", ");
    }
    fprintf(
        stderr,
        "[PARSER_Load] Unknown file format : %s, supported formats are [%s]\n",
        extension, formats);
    return NULL;
  }

  FILE *file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "[PARSER_Load] Impossible d'ouvrir le fichier %s : %s\n",
            filename, strerror(errno));
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

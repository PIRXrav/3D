/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "parser_obj.h"
#include "color.h"
#include "containers/arraylistp.h"
#include "geo.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define MAX_VERTICES_PER_FACE 3

/*******************************************************************************
 * Types
 ******************************************************************************/

/* Only types currently suported by parser are listed */
typedef enum entity_type {
  UNSUPPORTED = -1,
  BLANK = 0,
  COMMENT = 1,
  VERTEX = 2,
  FACE = 3,
  OBJECT = 4
} entity_type;

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/* Reads entity and move on to next tokens */
entity_type getEntityType(char *line);
/* Get next line skipping blank lines, comment and unsupported operands */
entity_type getNextEntity(char *buffer, unsigned max, FILE *file);

/* Parses a line containing face information */
void parseFace(char *line, unsigned *indexes, unsigned *nbVertices);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

struct Mesh **OBJ_Parse(FILE *file, unsigned *nbMeshes) {
  char buffer[256];

  entity_type entity;
  struct Mesh *currentMesh = NULL;
  ArrayList *meshes = ARRLISTP_Create();

  *nbMeshes = 0;
  int faceIndex = 0;
  int verticesIndexOffset = 0;

  while ((entity = getNextEntity(buffer, 256, file)) != BLANK) {

    switch (entity) {
    case VERTEX: {
      struct Vector v;
      sscanf(buffer, "v %lf %lf %lf", &v.x, &v.y, &v.z);
      MESH_AddVertex(currentMesh, MESH_VERT_Init(v.x, v.y, v.z));
    } break;
    case FACE:
      if (!currentMesh) {
        fprintf(stderr, "[OBJ_Parse] Warning : no object definition before "
                        "face definitions, creating one\n");
        currentMesh = MESH_Init();
      }

      unsigned verticesIndex[MAX_VERTICES_PER_FACE];
      unsigned nbVertices = MAX_VERTICES_PER_FACE;
      parseFace(buffer, verticesIndex, &nbVertices);

      // TODO: check if vertices exists (plusieurs meshs avec les meme sommets)
      MESH_AddFace(
          currentMesh,
          MESH_FACE_Init(
              MESH_GetVertex(currentMesh,
                             verticesIndex[0] - verticesIndexOffset),
              MESH_GetVertex(currentMesh,
                             verticesIndex[1] - verticesIndexOffset),
              MESH_GetVertex(currentMesh,
                             verticesIndex[2] - verticesIndexOffset),
              CL_rgb(50 + faceIndex * 2, faceIndex * 2, 50 + faceIndex)));
      faceIndex++;
      break;
    case OBJECT:
      // Nouvelle mesh : on ajoute la precedente a la liste et on travaille sur
      // une nouvelle
      if (currentMesh) {
        ARRLISTP_Add(meshes, currentMesh);
        verticesIndexOffset += MESH_GetNbVertice(currentMesh);
      }
      currentMesh = MESH_Init();
      strtok(buffer, " ");
      char *name = strtok(NULL, " ");
      MESH_SetName(currentMesh, name);
      faceIndex = 0;

      break;
    default:
      fprintf(stderr, "[OBJ_Parse] Warning : unsupported entity\n");
    }
  }

  // On ajoute la derniere mesh
  ARRLISTP_Add(meshes, currentMesh);

  *nbMeshes = ARRLISTP_GetSize(meshes);

  return ARRLISTP_ToArray(meshes);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

void parseFace(char *line, unsigned *indexes, unsigned *nbVertices) {
  unsigned currentNb = 0;
  char *token = strtok(line, " ");
  while ((token = strtok(NULL, " ")) && currentNb <= *nbVertices) {
    sscanf(token, "%u", &indexes[currentNb]);
    // On passe d'un indice base 1 du format obj à un indice base 0
    indexes[currentNb]--;
    currentNb++;
  }

  if (currentNb > *nbVertices)
    fprintf(stderr, "[OBJ_Parse] Warning : face is not a triangle, ignoring "
                    "other vertices\n");
  *nbVertices = currentNb;
}

entity_type getEntityType(char *line) {
  unsigned i = 0;
  while (line[i] != '\0' && isspace(line[i])) {
    i++;
  }
  unsigned length = strchr(line, ' ') - line;
  if (!strncmp(line, "", length))
    return BLANK;
  else if (!strncmp(line, "#", length))
    return COMMENT;
  else if (!strncmp(line, "v", length))
    return VERTEX;
  else if (!strncmp(line, "f", length))
    return FACE;
  else if (!strncmp(line, "o", length))
    return OBJECT;
  else
    return UNSUPPORTED;
}

entity_type getNextEntity(char *buffer, unsigned max, FILE *file) {
  entity_type entity;
  do {
    if (!fgets(buffer, max, file))
      return BLANK;
    entity = getEntityType(buffer);
  } while (entity == BLANK || entity == COMMENT || entity == UNSUPPORTED);
  return entity;
}

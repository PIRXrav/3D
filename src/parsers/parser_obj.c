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

#define MAX_VERTICES_PER_FACE 32

#define NB_ENTITY 10

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
  OBJECT = 4,
  MATERIAL_LIB = 5,
  MATERIAL = 6,

  MTL_DECLARATION = 10,
  MTL_AMBIENT = 11,
  MTL_DIFFUSE = 12,
} entity_type;

struct material {
  color color;
  char name[50];
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/* Reads entity and move on to next tokens */
entity_type getEntityType(char *line);
/* Get next line skipping blank lines, comment and unsupported operands */
entity_type getNextEntity(char *buffer, unsigned max, FILE *file);

/* Parses a line containing face information */
void parseFace(char *line, unsigned *indexes, unsigned *nbVertices);

/* Parses MTL */
ArrayList *MTL_Parse(char *mtllib);

/* Finds material by name in material list */
void find_material(const ArrayList *materialList, const char *name,
                   struct material *res);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/
// TODO: utiliser strtok_r pour rendre tous ca thread-safe
struct Mesh **OBJ_Parse(FILE *file, unsigned *nbMeshes, char *dir) {
  char buffer[256];

  entity_type entity;
  struct Mesh *currentMesh = NULL;
  ArrayList *meshes = ARRLISTP_Create();

  ArrayList *materials = NULL;
  struct material currentMaterial = {CL_GRAY, ""};

  int verticesIndexOffset = 0;

  while ((entity = getNextEntity(buffer, 256, file)) != BLANK) {
    // Si on ne declare pas l'objet alors qu'on en a besoin, on en cree un
    if (!currentMesh && (entity == VERTEX || entity == FACE)) {
      fprintf(stderr, "[OBJ_Parse] Warning : no object definition before "
                      "face definitions, creating one\n");
      currentMesh = MESH_Init();
    }

    switch (entity) {
    case VERTEX: {
      struct Vector v;
      sscanf(buffer, "v %lf %lf %lf", &v.x, &v.y, &v.z);
      MESH_AddVertex(currentMesh, MESH_VERT_Init(v.x, v.y, v.z));
    } break;
    case FACE: {
      unsigned verticesIndex[MAX_VERTICES_PER_FACE];
      unsigned nbVertices = MAX_VERTICES_PER_FACE;
      parseFace(buffer, verticesIndex, &nbVertices);

      MeshVertex **vertices = malloc(sizeof(MeshVertex *) * nbVertices);
      for (unsigned i = 0; i < nbVertices; i++)
        vertices[i] =
            MESH_GetVertex(currentMesh, verticesIndex[i] - verticesIndexOffset);

      // TODO: check if vertices exists (si plusieurs meshs avec les meme
      // sommets)
      unsigned nbFaces = 0;
      struct MeshFace **faces = MESH_FACE_FromVertices(
          vertices, nbVertices, &nbFaces, currentMaterial.color);
      printf("Face of %u vertices decomposed in %u triangles\n", nbVertices,
             nbFaces);
      MESH_AddFaces(currentMesh, faces, nbFaces);

      free(faces);
      free(vertices);
    } break;
    case OBJECT:
      // Nouvelle mesh : on ajoute la precedente a la liste et on travaille sur
      // une nouvelle
      if (currentMesh) {
        printf("Added a mesh\n");
        ARRLISTP_Add(meshes, currentMesh);
        verticesIndexOffset += MESH_GetNbVertice(currentMesh);
      }
      currentMesh = MESH_Init();
      strtok(buffer, " ");
      char *name = strtok(NULL, " ");
      MESH_SetName(currentMesh, name);

      break;
    case MATERIAL_LIB:
      strtok(buffer, " ");
      char *mtllib = strtok(NULL, " ");
      char filename[256];
      sprintf(filename, "%s/%s", dir, mtllib);
      materials = MTL_Parse(filename);
      break;
    case MATERIAL:
      if (!materials) {
        fprintf(stderr, "[OBJ_Parse] Warning : using materials without "
                        "declaring a material library first\n");
      } else {
        strtok(buffer, " ");
        char *materialName = strtok(NULL, " ");
        find_material(materials, materialName, &currentMaterial);
      }
      break;
    default:
      fprintf(stderr, "[OBJ_Parse] Warning : unsupported entity\n");
    }
  }

  if (materials)
    ARRLIST_Free(materials);

  // On ajoute la derniere mesh
  ARRLISTP_Add(meshes, currentMesh);

  *nbMeshes = ARRLISTP_GetSize(meshes);

  return ARRLISTP_ToArray(meshes);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

ArrayList *MTL_Parse(char *mtllib) {

  FILE *file = fopen(mtllib, "rb");
  if (!file) {
    fprintf(stderr, "[MTL_Parse] Error : cannot open '%s'\n", mtllib);
    return NULL;
  }

  char buffer[256];

  ArrayList *materials = ARRLIST_Create(sizeof(struct material));
  struct material current;
  color ambient, diffuse;

  entity_type entity;
  int noCurrentMaterial = 1;
  while ((entity = getNextEntity(buffer, 256, file)) != BLANK) {

    switch (entity) {
    case MTL_DECLARATION:
      if (!noCurrentMaterial) {
        current.color = CL_Mix(ambient, diffuse, .5);
        ARRLIST_Add(materials, &current);
      }

      strtok(buffer, " ");
      char *name = strtok(NULL, " ");
      strcpy(current.name, name);
      noCurrentMaterial = 0;
      break;
    case MTL_AMBIENT: {
      float r, g, b;
      sscanf(buffer, "Ka %f %f %f", &r, &g, &b);
      ambient =
          CL_rgb((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255));
    } break;
    case MTL_DIFFUSE: {
      float r, g, b;
      sscanf(buffer, "Kd %f %f %f", &r, &g, &b);
      diffuse =
          CL_rgb((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255));
    } break;
    default:
      fprintf(stderr, "[MTL_Parse] Warning : unsupported entity\n");
    }
  }

  // On ajoute le dernier material parse, si on a en a parse au moins un
  if (!noCurrentMaterial) {
    current.color = CL_Mix(ambient, diffuse, .5);
    ARRLIST_Add(materials, &current);
  }

  ARRLIST_Fit(materials);
  return materials;
}

void find_material(const ArrayList *materialList, const char *name,
                   struct material *res) {

  for (uint32_t i = 0; i < ARRLIST_GetSize(materialList); i++) {
    struct material *mat = ARRLIST_Get(materialList, i);
    if (!strcmp(name, mat->name)) {
      *res = *mat;
      return;
    }
  }

  fprintf(stderr, "[OBJ_Parse] Warning : unknown material '%s'\n", name);
}

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

  char *rulesDirectors[NB_ENTITY] = {"",       "#",      "v",      "f",
                                     "o",      "mtllib", "usemtl",

                                     "newmtl", "Ka",     "Kd"};
  entity_type rulesTokens[NB_ENTITY] = {
      BLANK,           COMMENT,      VERTEX,     FACE,
      OBJECT,          MATERIAL_LIB, MATERIAL,

      MTL_DECLARATION, MTL_AMBIENT,  MTL_DIFFUSE};

  for (uint32_t i = 0; i < NB_ENTITY; i++) {
    if (!strncmp(line, rulesDirectors[i], length))
      return rulesTokens[i];
  }
  return UNSUPPORTED;
}

entity_type getNextEntity(char *buffer, unsigned max, FILE *file) {
  entity_type entity;
  do {
    if (!fgets(buffer, max, file))
      return BLANK;

    char *newlinePos = strchr(buffer, '\n');
    if (newlinePos)
      *newlinePos = 0;

    entity = getEntityType(buffer);
  } while (entity == BLANK || entity == COMMENT || entity == UNSUPPORTED);
  return entity;
}

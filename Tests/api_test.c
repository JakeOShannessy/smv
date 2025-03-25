#include "smv.h"

int main(int argc, char **argv) {
  if(argc < 2) {
    fprintf(stderr, "error: insufficient arguments");
  }
  char *filepath = argv[1];
  // Initialize the customized memory allocator that smokeview uses
  initMALLOC();
  // Create and initialize an smv_case struct
  smv_case *scase = ScaseCreate();
  // Parse a file at the given path into scase
  int result = ScaseParseFromPath(filepath, scase);
  // Error handling on parse failure
  if(result) {
    fprintf(stderr, "failed to read smv file: %s\n", filepath);
    return result;
  }
  // For each mesh print the number of cells
  for(int i = 0; i < scase->meshescoll.nmeshes; i++) {
    meshdata *mesh = &scase->meshescoll.meshinfo[i];
    printf("MESH: %s: %d cells\n", mesh->label,
           mesh->ibar * mesh->jbar * mesh->kbar);
  }
  // Deconstruct and free scase
  ScaseDestroy(scase);
  return 0;
}

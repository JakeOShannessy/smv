#include "options.h"

#include "smv.h"

void InitScase(smv_case *scase);
smv_case global_scase;

int main(int argc, char **argv) {
  if(argc < 2) {
    fprintf(stderr, "error: insufficient arguments");
  }
  char *filepath = argv[1];
  // Initialize the customized memory allocator that smokeview uses
  initMALLOC();
  // Initialize the global_scase struct
  InitScase(&global_scase);
  // Parse a file at the given path into scase
  int result = ScaseParseFromPath(filepath, &global_scase);
  // Error handling on parse failure
  if(result) {
    fprintf(stderr, "failed to read smv file: %s\n", filepath);
    return result;
  }
  // For each mesh print the number of cells
  for(int i = 0; i < global_scase.meshescoll.nmeshes; i++) {
    meshdata *mesh = &global_scase.meshescoll.meshinfo[i];
    printf("MESH: %s: %d cells\n", mesh->label,
           mesh->ibar * mesh->jbar * mesh->kbar);
  }
  // Deconstruct global_scase
  ScaseClear(&global_scase);
  return 0;
}

#include "options.h"

#include "getdata.h"

#include "dmalloc.h"

#include "string_util.h"
#include <stdlib.h>

#include "readobject.h"

int main(int argc, char **argv) {
  initMALLOC();
  if (argc > 1) {
    const char *file_path = argv[1];
    // Create an object collection, read in object definitions, then free it.
    object_collection *objectscoll = CreateObjectCollection();
#if DEBUG
    int result = ReadObjectDefs(objectscoll, file_path);
    fprintf(stderr, "result: %i\n", result);
    // Two of the objects in this bad file are still parsable so we should
    // parse 2 object definitions.
    assert(result > 0);
#else
    ReadObjectDefs(objectscoll, file_path);
#endif
    FreeObjectCollection(objectscoll);
    return 0;
  }
  else {
    return 1;
  }
}

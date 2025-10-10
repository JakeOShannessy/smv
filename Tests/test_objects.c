#include "dmalloc.h"
#include "getdata.h"
#include "readobject.h"
#include "smv.h"
#include <stdlib.h>

int main(int argc, char **argv) {
  initMALLOC();
  {
    // Create an objects collection and then immediately free it.
    object_collection *objectscoll = CreateObjectCollection();
    // There should be no objects
    assert(objectscoll->nobject_defs == 0);
    FreeObjectCollection(objectscoll);
  }
  {
    // Create an object collection, read in object definitions, then free it.
    object_collection *objectscoll = CreateObjectCollection();
    // There should be no objects to begin with
    assert(objectscoll->nobject_defs == 0);
    ReadDefaultObjectCollection(objectscoll, NULL, 0);
    FreeObjectCollection(objectscoll);
  }
  {
    // Create an object collection, read in object definitions, then free it.
    object_collection *objectscoll = CreateObjectCollection();
    ReadDefaultObjectCollection(objectscoll, NULL, 0);
    FreeObjectCollection(objectscoll);
  }
  return 0;
}

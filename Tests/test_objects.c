#include "options.h"

#include "getdata.h"

#include "MALLOCC.h"

#include "string_util.h"
#include <stdlib.h>

#include "readobject.h"

int show_help;
int hash_option;
int show_version;
char append_string[1024];

int main(int argc, char **argv) {
  initMALLOC();
  {
    // Create an objects collection and then immediately free it.
    object_collection *objectscollx = CreateObjectCollection();
    FreeObjectCollection(objectscollx);
  }
  {
    // Create an object collection, read in object definitions, then free it.
    object_collection *objectscollx = CreateObjectCollection();
    const char *smokeview_bindir = "";
    const char *fdsprefix = "room_fire";
    int setbw = 0;
    int isZoneFireModel = 0;
    ReadObjectCollection(objectscollx, smokeview_bindir, fdsprefix, setbw,
                         isZoneFireModel);
    FreeObjectCollection(objectscollx);
  }
  return 0;
}

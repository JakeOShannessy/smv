#include "dmalloc.h"
#include "getdata.h"
#include "smv.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  int mode = FILE_MODE;
  initMALLOC();
  int t2 = GetFileListSize("Utilities/Scripts", "*", mode);
  filelistdata *l2 = NULL;
  MakeFileList("Utilities/Scripts", "*", t2, NO, &l2, mode);
  return 0;
}

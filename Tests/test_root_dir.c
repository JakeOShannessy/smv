#include "options.h"

#include "getdata.h"

#include "dmalloc.h"

#include "file_util.h"
#include "string_util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  initMALLOC();
  {
#if DEBUG
    char *root = GetSmvRootDir();
    int root_len = strlen(root);
    fprintf(stderr, "root_len: %i\n", root_len);
    assert(root_len > 0);
    assert(root[root_len - 1] == dirseparator[0]);
#endif
  }
  return 0;
}

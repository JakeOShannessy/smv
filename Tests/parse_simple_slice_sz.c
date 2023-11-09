#include "options.h"
#include "MALLOCC.h"
#include "getdata.h"
#include <assert.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  SizeFileInfo sz_info = {0};
  int result = ParseSizeFileInfo(argv[1], COMPRESSED_ZLIB, 1, 0, 0, 0, 0, &sz_info);
  assert(!result);
  assert(sz_info.nsteps == 2001);
  return result;
}

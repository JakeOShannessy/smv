#include "options.h"

// TODO: sort out imports
#include "MALLOCC.h"
#include "getdata.h"
#include <stdlib.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "MALLOCC.h"
#include "datadefs.h"
#include "histogram.h"
#include "isodefs.h"
#include "string_util.h"

#include "file_util.h"
#include "stdio_buffer.h"

#include "readhvac.h"

int show_help;
int hash_option;
int show_version;
char append_string[1024];

int main(int argc, char **argv) {
  initMALLOC();
  FILE *file;
  int i1, i2, j1, j2, k1, k2 = 0;
  int error = 0;

  hvacdatacollection hvaccoll = {0};
  FILE_SIZE file_size;
  ReadHVACData0(&hvaccoll, 0, &file_size);
  return 0;
}

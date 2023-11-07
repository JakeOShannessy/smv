#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"

int main(int argc, char **argv) {
  int error = 0;
  if (argc < 2) return 2;
  SmvFile smvfile = {0};
  FILE *stream = fopen(argv[1], "r");
  error = ReadSMV(&smvfile, stream);
  if (error) return error;
  printf("Surfaces[%zi]\n", smvfile.nsurfinfo);
  for (size_t i = 0; i < smvfile.nsurfinfo; i++) {
    printf("  \"%s\"\n", smvfile.surfinfo[i].surfacelabel);
  }
  return error;
}

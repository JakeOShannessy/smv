#include "getdata.h"
#include "dmalloc.h"
#include <stdlib.h>

int main(int argc, char **argv) {
  FILE *file;
  int i1, i2, j1, j2, k1, k2 = 0;
  int error = 0;

  openslice(argv[1], &file, &i1, &i2, &j1, &j2, &k1, &k2, &error);
  if (error) return 1;

  size_t nx = i2 + 1 - i1;
  size_t ny = j2 + 1 - j1;
  size_t nz = k2 + 1 - k1;

  if (argc < 3) return 2;

  size_t expected = atol(argv[2]);

  size_t nqframe = nx * ny * nz;
  float *qframe = malloc(nqframe * sizeof(float));
  float time;
  size_t count = 0;
  while (1) {
    getsliceframe(file, i1, i2, j1, j2, k1, k2, &time, qframe, 0, &error);
    if (error != 0) break;
    count++;
  }
  printf("count: %zu, expected: %zu\n", count, expected);
  if (count != expected) {
    return 1;
  }
  free(qframe);
  closefortranfile(file);
  return 0;
}

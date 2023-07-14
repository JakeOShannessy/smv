#include "getdata.h"
#include "dmalloc.h"
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 3) return 2;
  int error = 0;
  char *filename = argv[1];
  size_t n_expected_frames = atol(argv[2]);
  int boundary_version = 1;

  FILE *file;
  int npatches = 0;
  int headersize = 0;
  int npatchsize = 0;
  int framesize = 0;
  getpatchsizes1(&file, filename, &npatches, &headersize, &error);

  int *i1 = malloc(npatches * sizeof(int));
  int *i2 = malloc(npatches * sizeof(int));
  int *j1 = malloc(npatches * sizeof(int));
  int *j2 = malloc(npatches * sizeof(int));
  int *k1 = malloc(npatches * sizeof(int));
  int *k2 = malloc(npatches * sizeof(int));
  int *patchdir = malloc(npatches * sizeof(int));

  getpatchsizes2(file, boundary_version, npatches, &npatchsize, i1, i2, j1, j2,
                 k1, k2, patchdir, &headersize, &framesize);
  fclose(file);

  if (error) return error;
  file = openboundary(filename, boundary_version, &error);
  if (error) return error;
  float patchtime = 0.0;
  int file_size = 0;
  int npatchdata = 0;
  int nsize = 0;
  for (int i = 0; i < npatches; i++) {
    int size = (i2[i] - i1[i] + 1) * (j2[i] - j1[i] + 1) * (k2[i] - k1[i] + 1);
    printf("patch[%d] %d-%d %d-%d %d-%d (%d)\n", i, i1[i], i2[i], j1[i], j2[i],
           k1[i], k2[i], size);
    nsize += size;
  }

  int n_parse_frames = 0;
  if (nsize > 0) {
    // TODO: we need information about the patches first, including
    //  1. Number of patches.
    //  2. IJK values.
    float *patchdata = malloc(nsize * sizeof(float));

    while (1) {
      getpatchdata(file, npatches, i1, i2, j1, j2, k1, k2, &patchtime,
                   patchdata, &npatchdata, &file_size, &error);
      if (error) break;
      n_parse_frames++;
    }
    free(patchdata);
  }

  free(i1);
  free(i2);
  free(j1);
  free(j2);
  free(k1);
  free(k2);
  if (n_parse_frames != n_expected_frames) {
    fprintf(stderr, "incorrect number of frames");
    return 1;
  }
  closefortranfile(file);
  return 0;
}

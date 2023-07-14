#include "getdata.h"
#include "dmalloc.h"
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 3) return 2;
  int error = 0;
  char *filename = argv[1];
  size_t n_expected_frames = atol(argv[2]);
  FILE *file = openpart(filename, &error);

  int size = 0;
  int fdsversion = 0;
  int nclasses = 0;
  getpartheader1(file, &nclasses, &fdsversion, &size);

  int *nquantities = malloc(nclasses * sizeof(int));
  int *npoints = malloc(nclasses * sizeof(int));

  const size_t BUFFER_SIZE = 1000000;

  float *pdata = malloc(BUFFER_SIZE * sizeof(float));
  int *tagdata = malloc(BUFFER_SIZE * sizeof(int));

  getpartheader2(file, nclasses, nquantities, &size);
  int i = 0;
  while (1) {
    float time;
    getpartdataframe(file, nclasses, nquantities, npoints, &time, tagdata,
                     pdata, &size, &error);
    if (error != 0) break;
    i++;
  }

  closefortranfile(file);

  free(nquantities);
  free(npoints);
  free(pdata);
  free(tagdata);
  if (i != n_expected_frames) {
    return 1;
  }
  return 0;
}

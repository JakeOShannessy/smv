#include <stddef.h>
#include <stdio.h>

#include "datadefs.h"

/* ------------------ WriteFileBounds ------------------------ */

int bnd_write(const char *file, float valmin, float valmax) {
  FILE *stream = fopen(file, "w");
  if (stream == NULL) return 1;
  fprintf(stream, " %f %f %f", 0.0, valmin, valmax);
  fclose(stream);
  return 0;
}

/* ------------------ GetFileBounds ------------------------ */

int bnd_read(const char *file, float *valmin, float *valmax) {
  char buffer[255];
  float t, vmin, vmax;

  FILE *stream = fopen(file, "r");
  if (stream == NULL || fgets(buffer, 255, stream) == NULL) {
    if (stream != NULL) fclose(stream);
    return 1;
  }
  sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
  *valmin = vmin;
  *valmax = vmax;
  while (fgets(buffer, 255, stream) != NULL) {
    sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
    *valmin = MIN(*valmin, vmin);
    *valmax = MAX(*valmax, vmax);
  }
  fclose(stream);
  return 0;
}

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"

int main(int argc, char **argv) {
  int error = 0;
  if (argc < 3) return 2;
  size_t expected = atol(argv[2]);

  Compression compression_type = COMP_NONE;
  if (argc >= 4) {
    if (strcmp(argv[3], "rle") == 0) {
      compression_type = COMP_RLE;
    } else if (strcmp(argv[3], "zlib") == 0) {
      compression_type = COMP_ZLIB;
    }
  }

  SlcfR slice_file;
  error = slcfr_open(argv[1], compression_type, &slice_file);
  if (error) {
    fprintf(stderr, "could not open file %s\n", argv[1]);
    return error;
  }

  size_t frame_size = slcfr_frame_size(&slice_file);
  float *frame_data = malloc(frame_size * sizeof(float));
  float time;
  size_t count = 0;
  while (1) {
    fprintf(stderr, "reading: %u\n", slice_file.current_frame_index);
    error = slcfr_read_next_frame(&slice_file, &time, frame_data);
    if (error != 0) break;
    count++;
  }
  fprintf(stderr, "count: %zu, expected: %zu\n", count, expected);
  if (count != expected) {
    return 1;
  }
  free(frame_data);
  error = slcfr_close(&slice_file);
  return error;
}

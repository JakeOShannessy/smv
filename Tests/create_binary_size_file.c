#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "two arguments are required, the text size file and a path "
                    "to a new binary size file");
    return 2;
  };

  return create_binary_size_file(argv[1], argv[2]);
}

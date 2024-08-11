#include "MALLOCC.h"

int show_help;
int hash_option;
int show_version;
char append_string[1024];

int main(int argc, char **argv) {
  initMALLOC();
  int *cb;
  NEWMEMORY(cb, 256*sizeof(int));
  FREEMEMORY(cb);
  return 0;
}

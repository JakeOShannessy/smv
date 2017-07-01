
typedef struct {
  char name[256];
  char units[256];
  size_t nvalues;
  float *values;
} vector;

typedef struct {
  vector *x;
  vector *y;
} dvector;

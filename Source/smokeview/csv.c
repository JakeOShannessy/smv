#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "csv.h"

#define BUFFERSIZE 4096
#define INIT_VEC_SIZE 2048

// Return the original string if it cannot be unquoted
// Does not account for backslash escaped chars
// only accouts for strings starting and ending with double quotes
char *simpleunquote(char *string){

  char *c;
  size_t len = strlen(string);
  char *stringStart = string;
  char *stringEnd = string+strlen(string)-1;

  if(string==NULL)return string;
  if(len==0)return string;
  if(*stringStart=='"' && *stringEnd=='"') {
    string = stringStart+1;
    *stringEnd = '\0';
    return string;
  }
  return string;
}

// This is built for FDS CSV file in paticular.
size_t readcsv(char *file, dvector **dvectors){
  FILE *stream;

  if((stream = fopen(file, "r")) == NULL)return 0;
  char buffer[BUFFERSIZE];

  // Read data into buffer.
  if(fgets(buffer, BUFFERSIZE, stream) == NULL)return 0;
  char bufferdup[BUFFERSIZE];
  strcpy(bufferdup, buffer);
  // Trim buffer to a single line.
  // TODO: what about windows, fortran is usually just LF, but check
  buffer[strcspn(buffer, "\n")] = 0;
  // The first line is units, count the number of units first.
  char *bufptr;
  size_t x = 0;
  bufptr = strtok(buffer, ",\n");
  while(bufptr != NULL) {
    x++;
    bufptr = strtok(NULL, ",\n");
  }
  size_t n = x;
  // Now we know how many vectors we have. Assume the first vector is our
  // x vector
  // Initialise memory for each
  vector *vectors = malloc(n*sizeof(vector));

  // Read through the units line again and set the unit name values.
  // Assume the first vector is the x-vector (usually time)
  bufptr = strtok(bufferdup, ",\n");
  size_t j = 0;
  while(bufptr != NULL) {
    strcpy(vectors[j].units, bufptr);
    bufptr = strtok(NULL, ",\n");
    j++;
  }

  // Read data into buffer.
  if(fgets(buffer, BUFFERSIZE, stream) == NULL)return 0;
  // Trim buffer to a single line.
  // TODO: what about windows, fortran is usually just LF, but check
  buffer[strcspn(buffer, "\n")] = 0;
  // Assume the first vector is the x-vector (usually time)
  bufptr = strtok(buffer, ",\n");
  j = 0;
  while(bufptr != NULL) {
    strcpy(vectors[j].name, simpleunquote(bufptr));
    bufptr = strtok(NULL, ",\n");
    j++;
  }

  // Initialise the value arrays with an initial assumption of INIT_VEC_SIZE,
  // setting nvalues to zero.
  for (size_t k = 0; k < n; ++k) {
    // Initialise memory.
    float *vec = malloc(INIT_VEC_SIZE*sizeof(float));
    // Assign to vector.
    vectors[k].values = vec;
    // Set nvalues to zero;
    vectors[k].nvalues = 0;
  }

  size_t l = 0;
  size_t current_vec_size = INIT_VEC_SIZE;
  while(!feof(stream)){
    if(l>=current_vec_size){
      // Allocate larger arrays
      current_vec_size = current_vec_size + INIT_VEC_SIZE;
      for (size_t k = 0; k < n; k++) {
        vectors[k].values = realloc(vectors[k].values,
                                    current_vec_size*sizeof(float));
        if(vectors[k].values==NULL) {
          fprintf(stderr, "reallocation failed\n");
          exit(1);
        }
      }
    }
    // Read data into buffer.
    if(fgets(buffer, BUFFERSIZE, stream) == NULL)break;
    bufptr = strtok(buffer, ",\n");
    size_t k = 0;
    while(bufptr != NULL) {
      sscanf(bufptr,"%f", &vectors[k].values[l]);
      vectors[k].nvalues = l+1;
      bufptr = strtok(NULL, ",\n");
      k++;
    }
    l++;
  }
  *dvectors = malloc((n-1)*sizeof(dvector));
  for (size_t i = 0; i < n-1; ++i) {
    (*dvectors)[i].x = &vectors[0];
    (*dvectors)[i].y = &vectors[i+1];
  }
  fclose(stream);
  return n-1;
}

int main(int argc, char const *argv[]) {
  dvector *dvectors;
  size_t n = readcsv("testcsv.csv", &dvectors);
  for (size_t k = 0; k < n; ++k) {
    printf("Vector:\n");
    printf("  xname: %s\n", dvectors[k].x->name);
    printf("  xunits: %s\n", dvectors[k].x->units);
    printf("  yname: %s\n", dvectors[k].y->name);
    printf("  yunits: %s\n", dvectors[k].y->units);
    printf("  nvalues: %zu\n", dvectors[k].x->nvalues);
  }
  return 0;
}
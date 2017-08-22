#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "csv.h"

#define BUFFERSIZE 4096
#define INIT_VEC_SIZE 2048

char* strsep(char** stringp, const char* delim) {
  char* start = *stringp;
  char* p;

  p = (start != NULL) ? strpbrk(start, delim) : NULL;

  if (p == NULL)
  {
    *stringp = NULL;
  }
  else
  {
    *p = '\0';
    *stringp = p + 1;
  }

  return start;
}

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
  fprintf(stderr, "reading: %s\n", file);
  if((stream = fopen(file, "r")) == NULL)return 0;
  // char buffer[BUFFERSIZE];
  char *buffer = malloc(BUFFERSIZE*sizeof(char));

  // Read data into buffer.
  if(fgets(buffer, BUFFERSIZE, stream) == NULL)return 0;
  // char bufferdup[BUFFERSIZE];
  char *bufferdup = malloc(BUFFERSIZE*sizeof(char));
  strcpy(bufferdup, buffer);
  // Trim buffer to a single line.
  // TODO: what about windows, fortran is usually just LF, but check
  buffer[strcspn(buffer, "\n")] = 0;
  bufferdup[strcspn(buffer, "\n")] = 0;
  fprintf(stderr, "buffer: >%s<\n", buffer);
  fprintf(stderr, "bufferdup: >%s<\n", bufferdup);
  // The first line is units, count the number of units first.
  char *bufptr;
  size_t x = 0;
  bufptr = strsep(&buffer, ",\n");
  while(bufptr != NULL) {
    fprintf(stderr, "unit[%zu]: %s\n", x, bufptr);
    x++;
    bufptr = strsep(&buffer, ",\n");
  }
  size_t n = x;
  // Now we know how many vectors we have. Assume the first vector is our
  // x vector
  // Initialise memory for each
  vector *vectors = malloc(n*sizeof(vector));

  // Read through the units line again and set the unit name values.
  // Assume the first vector is the x-vector (usually time)
  bufptr = strsep(&bufferdup, ",\n");
  size_t j = 0;
  while(bufptr != NULL) {
    fprintf(stderr, "unitsbufptr[%zu]: %s\n", j, bufptr);
    strcpy(vectors[j].units, bufptr);
    bufptr = strsep(&bufferdup, ",\n");
    j++;
  }
  free(buffer);
  buffer = malloc(BUFFERSIZE*sizeof(char));
  // Read data into buffer for names.
  if(fgets(buffer, BUFFERSIZE, stream) == NULL){
    fprintf(stderr, "fgets failed\n");
    return 0;
  }
  // Trim buffer to a single line.
  // TODO: what about windows, fortran is usually just LF, but check
  buffer[strcspn(buffer, "\n")] = 0;
  // Assume the first vector is the x-vector (usually time)
  bufptr = strsep(&buffer, ",\n");
  j = 0;
  while(bufptr != NULL) {
    fprintf(stderr, "name[%zu]: %s\n", j, bufptr);
    strcpy(vectors[j].name, simpleunquote(bufptr));
    bufptr = strsep(&buffer, ",\n");
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
    free(buffer);
    buffer = malloc(BUFFERSIZE*sizeof(char));
    if(fgets(buffer, BUFFERSIZE, stream) == NULL)break;
    bufptr = strsep(&buffer, ",\n");
    size_t k = 0;
    while(bufptr != NULL) {
      sscanf(bufptr,"%f", &vectors[k].values[l]);
      vectors[k].nvalues = l+1;
      bufptr = strsep(&buffer, ",\n");
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

#ifndef IOCOMMON_H_DEFINED
#define IOCOMMON_H_DEFINED
#include <stdio.h>

int cread(void *ptr, size_t size, size_t count, FILE *file);
int fortread(void *ptr, size_t size, size_t count, FILE *file);
int fortwrite(void *ptr, size_t size, size_t count, FILE *file);
int fortseek(FILE *file, size_t size, size_t count, int whence);
size_t fort_record_size(size_t size, size_t count);
FILE *FOPEN(const char *file, const char *mode);
typedef struct {
  char *longlabel, *shortlabel, *unit;
} flowlabels;
void FreeLabels(flowlabels *flowlabel);
flowlabels NewLabels();

const char *TrimFrontConst_(const char *line);
void TrimBack_(char *line);
#endif

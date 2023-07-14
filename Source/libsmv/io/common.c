#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <share.h>
#endif
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#if __STDC_VERSION__ >= 201112L
#include <limits.h>
#endif
#include "common.h"

#define LABEL_SIZE 31

// As with the Fortran code preceding this, it is assumed that a float is 4
// bytes for serialization/deserialization purposes. With C11 we can check this
// using static assertions.
#if __STDC_VERSION__ >= 201112L
_Static_assert(CHAR_BIT == 8, "io/common.c assumes that CHAR_BIT == 8");
_Static_assert(sizeof(float) == 4, "io/common.c assumes that float is 4 bytes");
#endif

#ifdef WIN32
FILE *FOPEN(const char *file, const char *mode) {
  return _fsopen(file, mode, _SH_DENYNO);
}
#else
FILE *FOPEN(const char *file, const char *mode) { return fopen(file, mode); }
#endif

/// TrimFrontConst duplicated here due to dependency problems.
const char *TrimFrontConst_(const char *line) {
  for (const char *c = line; c <= line + strlen(line) - 1; c++) {
    if (!isspace((unsigned char)(*c))) return c;
  }
  return line;
}

/// TrimBack duplicated here due to dependency problems.
void TrimBack_(char *line) {
  if (line == NULL) return;
  size_t len = strlen(line);
  if (len == 0) return;
  for (char *c = line + len - 1; c >= line; c--) {
    if (isspace((unsigned char)(*c))) continue;
    *(c + 1) = '\0';
    return;
  }
  *line = '\0';
}

/// @brief Read a number of elements using fread. Returns an error code if an
/// insufficient number of elements are read. Has the same interface a fortread.
/// @param[out] ptr The location to store read data.
/// @param[in] size The size of each element in bytes.
/// @param[in] count The number of elements.
/// @param[in,out] file The FILE to read from.
/// @return 0 on success.
/// @return -3 if the correct number of elements could not be read.
int cread(void *ptr, size_t size, size_t count, FILE *file) {
  size_t n_elments_read = fread(ptr, size, count, file);
  if (n_elments_read == count) {
    return 0;
  } else {
    return -3;
  }
}

/// @brief Read a number of elements formatted in a Fortran style, i.e., with a
/// header and a trailer indicating the length of the record. Returns an error
/// code if the wrong number of elements are read, or if the header/trailer
/// combination is incorrect.
/// @param[out] ptr The location to store read data.
/// @param[in] size The size of each element in bytes.
/// @param[in] count The number of elements.
/// @param[in,out] file The FILE to read from.
/// @return 0 on success.
/// @return -1 if the file ended unexpectedly while reading.
/// @return -2 if the header of the record doesn't match the expected number of
/// bytes based on size and count.
/// @return -3 if the correct number of elements could not be read.
/// @return -4 if the trailer could not be read.
/// @return -5 if the trailer does not hold the correct value.
int fortread(void *ptr, size_t size, size_t count, FILE *file) {
  // TODO: check endianess, currently little-endian is assumed
  // Read record header. Smokeview data files use 32-bit ints and floats.
  uint32_t header;
  size_t header_read = fread(&header, sizeof(header), 1, file);
  if (header_read == 0) {
    if (feof(file)) {
      return -1;
    } else {
      fprintf(stderr, "Value of errno: %d\n", errno);
      fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
    }
  }
  // The value in header is the number of bytes and should match size * count.
  if (header != (size * count)) {
    fprintf(stderr, "Expected record of %zu bytes, found one of %u bytes\n",
            size * count, header);
    return -2;
  }

  size_t data_read = fread(ptr, size, count, file);
  if (data_read != count) return -3;

  // Read record trailer
  uint32_t trailer = 0;
  size_t trailer_read = fread(&trailer, sizeof(trailer), 1, file);
  if (trailer_read != 1) return -4;
  if (trailer != (size * count)) return -5;
  return 0;
}

int fortwrite(void *ptr, size_t size, size_t count, FILE *file) {
  // TODO: check endianess
  // Read record header
  uint32_t header = size * count;
  size_t header_written = fwrite(&header, sizeof(header), 1, file);
  if (header_written == 0) {
    fprintf(stderr, "Value of errno: %d\n", errno);
    fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
  }
  if (header_written != 1) return 1;

  size_t data_written = fwrite(ptr, size, count, file);
  if (data_written != count) return 3;

  // Write record trailer
  uint32_t trailer = size * count;
  size_t trailer_written = fwrite(&trailer, sizeof(trailer), 1, file);
  if (trailer_written != 1) return 4;
  assert(header == (size * count));
  assert(data_written == count);
  assert(trailer == (size * count));
  return 0;
}

/// @brief Seek a Fortran-style record. Critically this assumes that the
/// file handle is already aligned with the records in a file. Note that this
/// can only seek a single record at a time. Return values are as per fseek.
/// @param file The file handle to seek in.
/// @param size The size of each element in the record.
/// @param count The number of elements in the record.
/// @param whence The initial position.
/// @return 0 on success
/// @return nonzero on failure
int fortseek(FILE *file, size_t size, size_t count, int whence) {
  return fseek(file, sizeof(uint32_t) + size * count + sizeof(uint32_t),
               whence);
}

/// @brief Free labels.
/// @param flowlabel Pointer the the flowlabels struct to free.
void FreeLabels(flowlabels *flowlabel) {
  free(flowlabel->longlabel);
  free(flowlabel->shortlabel);
  free(flowlabel->unit);
}

/// @brief Create a new label struct.
/// @return flowlabels struct with strings allocated.
flowlabels NewLabels() {
  flowlabels flowlabel;
  flowlabel.longlabel = malloc(LABEL_SIZE);
  flowlabel.shortlabel = malloc(LABEL_SIZE);
  flowlabel.unit = malloc(LABEL_SIZE);
  return flowlabel;
}

/// @brief Given a specification of the a Fortran-style record, return the total
/// number of bytes of that record (including header and trailer).
/// @param size The size of each element.
/// @param count The number of elements.
/// @return The total size of this record in bytes.
size_t fort_record_size(size_t size, size_t count) {
  return sizeof(uint32_t) + size * count + sizeof(uint32_t);
}

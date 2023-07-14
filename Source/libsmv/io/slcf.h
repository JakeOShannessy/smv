#ifndef IOSLCF_H_DEFINED
#define IOSLCF_H_DEFINED

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

typedef enum {
  COMP_NONE,
  COMP_RLE,
  COMP_ZLIB,
} Compression;

typedef enum { X = 1, Y = 2, Z = 3, ThreeD } SliceDirection;

/// @brief An opened slice file with parsed header information.
///
/// Does not parse any frame data.
typedef struct {
  /// @brief The file which was opened. Should never be null.
  FILE *file;
  /// @brief The bounds of the slice.
  uint32_t ijk[6];
  /// @brief  The labels associated with the slice.
  flowlabels labels;
  /// @brief The frame index the file handle is currently pointed towards.
  uint32_t current_frame_index;
  /// @brief The plane on which the slice lies, or 3d if it is 3d.
  SliceDirection direction;
  /// @brief The type of compression used by the file.
  Compression compression_type;
  /// @brief The global min and max of the slice file. Only valid if
  /// compression_type is COMP_RLE or COMP_ZLIB.
  float minmax[2];
} SlcfR;

size_t slcfr_frame_size(SlcfR *slice_file);
int slcfr_open(const char *path, Compression compression_type,
               SlcfR *slice_file);
int slcfr_close(SlcfR *slice_file);
int slcfr_read_next_frame(SlcfR *slice_file, float *time,
                          float *frame_datatestslice);
int slcfr_make_size_file_text(SlcfR *slice_file, FILE *sizestream);
int slcfr_reset(SlcfR *slice_file);

/// @brief An opened slice file for writing.
typedef struct {
  /// @brief The file which was opened. Should never be null.
  FILE *file;
  /// @brief The bounds of the slice.
  uint32_t ijk[6];
  /// @brief The labels associated with the slice.
  flowlabels labels;
  /// @brief The frame index the file handle is currently pointed towards.
  uint32_t current_frame_index;
  /// @brief  The plane on which the slice lies, or 3d if it is 3d.
  SliceDirection direction;
} SlcfW;

size_t slcfw_frame_size(SlcfW *slice_file);
int slcfw_open(const char *path, uint32_t ijk[6], flowlabels labels,
               SlcfW *slice_file);
int slcfw_close(SlcfW *slice_file);
int slcfw_write_next_frame(SlcfW *slice_file, float time, float *frame_data);

SliceDirection direction(uint32_t ijk[6]);
int create_binary_size_file(const char *text_size_path,
                            const char *bimary_size_path);
#endif

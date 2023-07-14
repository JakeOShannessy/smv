#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "common.h"
#include "slcf.h"

/// @brief Determine the size of a single frame of slice data.
///
/// @return The size of the uncompressed frame in bytes, excluding the time
/// record or any record headers and trailers.
size_t slcfr_frame_size(SlcfR *slice_file) {
  size_t nxsp = slice_file->ijk[1] + 1 - slice_file->ijk[0];
  size_t nysp = slice_file->ijk[3] + 1 - slice_file->ijk[2];
  size_t nzsp = slice_file->ijk[5] + 1 - slice_file->ijk[4];
  return sizeof(float) * (nxsp * nysp * nzsp);
}

size_t slcfw_frame_size(SlcfW *slice_file) {
  size_t nxsp = slice_file->ijk[1] + 1 - slice_file->ijk[0];
  size_t nysp = slice_file->ijk[3] + 1 - slice_file->ijk[2];
  size_t nzsp = slice_file->ijk[5] + 1 - slice_file->ijk[4];
  return sizeof(float) * (nxsp * nysp * nzsp);
}

/// @brief Determine the plane upon which this slice lies (or 3d if it is 3d).
SliceDirection direction(uint32_t ijk[6]) {

  size_t nxsp = ijk[1] + 1 - ijk[0];
  size_t nysp = ijk[3] + 1 - ijk[2];
  size_t nzsp = ijk[5] + 1 - ijk[4];

  bool x_thin = nxsp == 1;
  bool y_thin = nysp == 1;
  bool z_thin = nzsp == 1;

  // The number of dimensions along which this slice is thin.
  int n_thin = 0;
  if (x_thin) n_thin++;
  if (y_thin) n_thin++;
  if (z_thin) n_thin++;

  // If the slice if thin on many dimensions (an unusual case) we treat it as
  // 3d.
  if (n_thin > 1) return ThreeD;

  // Otherwise we return whichever dimension is thin, or if it is not thin in
  // any dimsions, it is obviously 3d.
  if (x_thin) {
    return X;
  } else if (y_thin) {
    return Y;
  } else if (z_thin) {
    return Z;
  } else {
    return ThreeD;
  }
}

/// @brief Open a slice value for reading and parse its header.
/// @param[in] path The path to read from.
/// @param[in] compression_type The compression method to use.
/// @param[out] slice_file
/// @return 0 on success
/// @return 1 if there is an error which sets errno.
/// @return other non-zero value on failure.
int slcfr_open(const char *path, Compression compression_type,
               SlcfR *slice_file) {
  int error = 0;
  slice_file->compression_type = compression_type;
  slice_file->current_frame_index = 0;
  uint32_t endianness;
  uint32_t completion;
  uint32_t fileversion;
  uint32_t version_local;

  slice_file->labels = NewLabels();

  slice_file->file = FOPEN(path, "rb");
  if (slice_file->file == NULL) {
    // The file could not be opened, it is the responsibility of the caller to
    // look at errno if desired.
    error = 1;
    return error;
  }

  switch (compression_type) {
  case COMP_NONE:
    error = fortread(slice_file->labels.longlabel, 30, 1, slice_file->file);
    if (error) return error;
    error = fortread(slice_file->labels.shortlabel, 30, 1, slice_file->file);
    if (error) return error;
    error = fortread(slice_file->labels.unit, 30, 1, slice_file->file);
    if (error) return error;
    error = fortread(slice_file->ijk, sizeof(*(slice_file->ijk)), 6,
                     slice_file->file);
    break;
  case COMP_RLE:
    // A compressed header is as follows:
    // endian (4-bytes)
    // completion (0/1) (4-bytes)
    // fileversion (compressed format) (4-bytes)
    // version_local  (slicef version) (4-bytes)
    // global min max (used to perform conversion) (4-bytes+4-bytes)
    // i1,i2,j1,j2,k1,k2 6*4-bytes
    error = fortread(&endianness, 30, 1, slice_file->file);
    if (error) return error;
    error = fortread(&completion, 30, 1, slice_file->file);
    if (error) return error;
    error = fortread(&fileversion, 30, 1, slice_file->file);
    if (error) return error;
    error = fortread(&slice_file->minmax, 4, 2, slice_file->file);
    if (error) return error;
    error = fortread(slice_file->ijk, sizeof(*(slice_file->ijk)), 6,
                     slice_file->file);
    break;
  case COMP_ZLIB:
    // A compressed header is as follows:
    // endian (4-bytes)
    // completion (0/1) (4-bytes)
    // fileversion (compressed format) (4-bytes)
    // version_local  (slicef version) (4-bytes)
    // global min max (used to perform conversion) (4-bytes+4-bytes)
    // i1,i2,j1,j2,k1,k2 6*4-bytes
    error = cread(&endianness, 4, 1, slice_file->file);
    if (error) return error;
    error = cread(&completion, 4, 1, slice_file->file);
    if (error) return error;
    error = cread(&fileversion, 4, 1, slice_file->file);
    if (error) return error;
    error = cread(&version_local, 4, 1, slice_file->file);
    if (error) return error;
    error = cread(&slice_file->minmax, 4, 2, slice_file->file);
    if (error) return error;
    error =
        cread(slice_file->ijk, sizeof(*(slice_file->ijk)), 6, slice_file->file);
    break;
  }
  if (error) return error;

  slice_file->direction = direction(slice_file->ijk);

  return error;
}

// header size in bytes
size_t slcfr_header_size(SlcfR *slice_file) {
  switch (slice_file->compression_type) {
  case COMP_NONE:
    return fort_record_size(4, 1) + fort_record_size(4, 2) +
           fort_record_size(4, 2) +
           fort_record_size(sizeof(*(slice_file->ijk)), 6);
  case COMP_RLE:
    return fort_record_size(30, 1) + fort_record_size(30, 1) +
           fort_record_size(30, 1) + fort_record_size(4, 2) +
           fort_record_size(sizeof(*(slice_file->ijk)), 6);
  case COMP_ZLIB:
    return 4 + 4 + 4 + 4 + 4 * 2 + 4 * 6;
  }
  return 0;
}

/// @brief Rest the slice file to frame 0.
/// @param slice_file The slice file to reset
/// @return 0 on success
/// @return nonzero on failure (e.g., if there isn't a complete header).
int slcfr_reset(SlcfR *slice_file) {
  int error = 0;
  size_t header_size = slcfr_header_size(slice_file);
  error = fseek(slice_file->file, header_size, SEEK_SET);
  slice_file->current_frame_index = 0;
  return error;
}

// TODO: this is duplicated from compress.c
#define MARK 255
unsigned int UnCompressRLE(unsigned char *buffer_in, int nchars_in,
                           unsigned char *buffer_out) {
  int nrepeats, nn;
  unsigned char thischar, *buffer_in_end;

  nn = 0;
  buffer_in_end = buffer_in + nchars_in;

  while (buffer_in < buffer_in_end) {
    if (*buffer_in == MARK) {
      if (buffer_in + 2 >= buffer_in_end) break;
      buffer_in++;
      thischar = *buffer_in++;
      nrepeats = *buffer_in++;
      nn += nrepeats;
      memset(buffer_out, thischar, nrepeats);
      buffer_out += nrepeats;
    } else {
      *buffer_out++ = *buffer_in++;
      nn++;
    }
  }
  return nn;
}

// Seek forward one frame
int slcfr_seek_next(SlcfR *slice_file) {
  int error = 0;
  uint32_t frame_size[2]; // [original,compressed]
  uint32_t compressed_size;
  switch (slice_file->compression_type) {
  case COMP_NONE:
    error = fortseek(slice_file->file, sizeof(float), 1, SEEK_CUR);
    if (error != 0) return error;
    error = fortseek(slice_file->file, sizeof(float),
                     slcfr_frame_size(slice_file), SEEK_CUR);
    if (error != 0) return error;
    break;
  case COMP_RLE:
    //  time
    //  original frame size, compressed frame size (rle)
    //  compressed buffer
    // Read the time of the frame
    if (fortseek(slice_file->file, 4, 1, SEEK_CUR)) return 3;
    // Read the original and compressed sizes of the frame
    if (fortseek(slice_file->file, 4, 2, SEEK_CUR)) return 3;
    if (fortread(frame_size, 4, 2, slice_file->file)) return 3;
    compressed_size = frame_size[1];
    if (fortseek(slice_file->file, 1, compressed_size, SEEK_CUR)) return 5;
    break;
  case COMP_ZLIB:
    //  time
    //  compressed frame size (zlib)
    //  compressed buffer
    if (!fseek(slice_file->file, 4 * 1, SEEK_CUR)) return 3;
    // Read the compressed size (in bytes)
    if (!fread(&compressed_size, 4, 1, slice_file->file)) return 4;
    if (!fseek(slice_file->file, 1 * compressed_size, SEEK_CUR)) return 5;
    break;
  }
  (slice_file->current_frame_index)++;
  return error;
}

/// @brief Seek to a specific frame number.
/// @param slice_file The SlcfR to seek within.
/// @param frame_number The frame number to seek to.
/// @return 0 if the frame was succesfully seeked to.
/// @return -1 if seeking failed in general.
/// @return -2 if seeking failed due to the frame number being too high.
int slcfr_seek_frame(SlcfR *slice_file, size_t frame_number) {
  int error = 0;
  size_t offset;
  switch (slice_file->compression_type) {
  case COMP_NONE:
    // When the file is uncompressed we can seek directly.
    // TODO: how do we handle partial frames?
    offset = slcfr_header_size(slice_file) +
             slcfr_frame_size(slice_file) * frame_number;
    error = fseek(slice_file->file, offset, SEEK_SET);
    if (error != 0) return error;
    // TODO: we shouldn't set this frame number if the seek fails
    (slice_file->current_frame_index) = frame_number;
    break;
  case COMP_RLE:
  case COMP_ZLIB:
    // For all compression types we simply need to iterate through, first we
    // rest the file. TODO: we should be able to avoid this with size files.
    error = slcfr_reset(slice_file);
    if (error != 0) return error;
    // Then we iterate
    while (slice_file->current_frame_index < frame_number) {
      if (!slcfr_seek_next(slice_file)) break;
    }
    break;
  }
  return error;
}

/// @brief Parse the next frame of slice data.
///
/// @param[in,out] slice_file The slice file from which to read from.
/// @param[out] time The frame time.
/// @param[out] frame_data A pre-allocated array to store frame data. TODO: need
/// to check that when data is uncompressed it still fits into this buffer.
///
/// @return 0 on success.
/// @return 1 if there is an error which sets errno.
/// @return other non-zero value on failure.
int slcfr_read_next_frame(SlcfR *slice_file, float *time, float *frame_data) {

  size_t nxsp = slice_file->ijk[1] + 1 - slice_file->ijk[0];
  size_t nysp = slice_file->ijk[3] + 1 - slice_file->ijk[2];
  size_t nzsp = slice_file->ijk[5] + 1 - slice_file->ijk[4];

  void *compressed_data;
  uint32_t frame_size[2]; // [original,compressed]
  uint32_t compressed_size;
  uLongf countout;

  int error = 0;

  switch (slice_file->compression_type) {
  case COMP_NONE:
    error = fortread(time, sizeof(*time), 1, slice_file->file);
    if (error != 0) return error;
    error = fortread(frame_data, sizeof(*frame_data), nxsp * nysp * nzsp,
                     slice_file->file);
    if (error != 0) return error;
    break;
  case COMP_RLE:
    //  time
    //  original frame size, compressed frame size (rle)
    //  compressed buffer
    // Read the time of the frame
    if (fortread(time, 4, 1, slice_file->file)) return 3;
    // Read the original and compressed sizes of the frame
    if (fortread(frame_size, 4, 2, slice_file->file)) return 3;
    compressed_size = frame_size[1];
    // Read the compressed data
    compressed_data = malloc(compressed_size);
    if (fortread(compressed_data, 1, compressed_size, slice_file->file))
      return 5;
    // Uncompress the data
    countout = UnCompressRLE(compressed_data, compressed_size,
                             (unsigned char *)frame_data);
    free(compressed_data);
    break;
  case COMP_ZLIB:
    //  time
    //  compressed frame size (zlib)
    //  compressed buffer
    if (!fread(time, 4, 1, slice_file->file)) return 3;
    // Read the compressed size (in bytes)
    if (!fread(&compressed_size, 4, 1, slice_file->file)) return 4;
    // Read the compressed data
    compressed_data = malloc(compressed_size);
    if (!fread(compressed_data, 1, compressed_size, slice_file->file)) return 5;
    // Uncompress the data
    countout = slcfr_frame_size(slice_file) * sizeof(float);
    uncompress((unsigned char *)frame_data, &countout, compressed_data,
               compressed_size * sizeof(float));
    free(compressed_data);
    break;
  }
  (slice_file->current_frame_index)++;
  return error;
}

/// @brief Close a slice file being read, freeing up any relevant memory.
/// @param[in,out] slice_file
///
/// @return 0 on success.
/// @return otherwise whichever errorcode came from fclose.
int slcfr_close(SlcfR *slice_file) {
  FreeLabels(&slice_file->labels);
  return fclose(slice_file->file);
}

/// @brief Open a slice file for writing and write its header.
/// @param[in] path The path to write to.
/// @param[in] compression_type The compression method to use.
/// @param[in] ijk The bounds of the data.
/// @param[in] labels The labels of the slice.
/// @param[out] slice_file
/// @return 0 on success
/// @return 1 if there is an error which sets errno.
/// @return other non-zero value on failure.
int slcfw_open(const char *path, uint32_t ijk[6], flowlabels labels,
               SlcfW *slice_file) {
  int error = 0;

  slice_file->file = FOPEN(path, "wb");
  if (slice_file->file == NULL) {
    // The file could not be opened, it is the responsibility of the caller to
    // look at errno if desired.
    error = 1;
    return error;
  }
  slice_file->labels = labels;
  slice_file->current_frame_index = 0;
  slice_file->direction = direction(ijk);
  slice_file->ijk[0] = ijk[0];
  slice_file->ijk[1] = ijk[1];
  slice_file->ijk[2] = ijk[2];
  slice_file->ijk[3] = ijk[3];
  slice_file->ijk[4] = ijk[4];
  slice_file->ijk[5] = ijk[5];

  error = fortwrite(slice_file->labels.longlabel, 30, 1, slice_file->file);
  if (error) return error;
  error = fortwrite(slice_file->labels.shortlabel, 30, 1, slice_file->file);
  if (error) return error;
  error = fortwrite(slice_file->labels.unit, 30, 1, slice_file->file);
  if (error) return error;

  error = fortwrite(slice_file->ijk, sizeof(*(slice_file->ijk)), 6,
                    slice_file->file);
  if (error) return error;

  return error;
}

/// @brief Write the next frame of slice data.
/// @param[in,out] slice_file
/// @param[in] time
/// @param[in] frame_data
/// @return
int slcfw_write_next_frame(SlcfW *slice_file, float time, float *frame_data) {
  size_t nxsp = slice_file->ijk[1] + 1 - slice_file->ijk[0];
  size_t nysp = slice_file->ijk[3] + 1 - slice_file->ijk[2];
  size_t nzsp = slice_file->ijk[5] + 1 - slice_file->ijk[4];
  int error = 0;

  error = fortwrite(&time, sizeof(time), 1, slice_file->file);
  if (error != 0) return error;
  // TODO: check that this is in the write format
  error = fortwrite(frame_data, sizeof(*frame_data), nxsp * nysp * nzsp,
                    slice_file->file);
  return error;
}

/// @brief Close a slice file being written, freeing up any relevant memory.
/// @param[in] slice_file
///
/// @return 0 on success.
/// @return otherwise whichever errorcode came from fclose.
int slcfw_close(SlcfW *slice_file) {
  FreeLabels(&slice_file->labels);
  return fclose(slice_file->file);
}

int slcfr_make_size_file_text(SlcfR *slice_file, FILE *sizestream) {
  if (slice_file->compression_type == COMP_NONE) {
    fprintf(stderr, "Cannot create size file of uncompressed data");
    return -1;
  }
  int error = 0;
  int line_count = 0;
  // Reset cursor to zero
  error = slcfr_reset(slice_file);
  if (error) return error;
  fprintf(sizestream, "%i %i %i %i %i %i\n", slice_file->ijk[0],
          slice_file->ijk[1], slice_file->ijk[2], slice_file->ijk[3],
          slice_file->ijk[4], slice_file->ijk[5]);
  fprintf(sizestream, "%f %f\n", slice_file->minmax[0], slice_file->minmax[1]);
  line_count = 2;
  float time;
  uint32_t compressed_size;
  uint32_t frame_size[2];
  do {
    switch (slice_file->compression_type) {
    case COMP_NONE:
      return -1;
    case COMP_RLE:
      if (fortread(&time, 4, 1, slice_file->file)) return -1;
      // Read the original and compressed sizes of the frame
      if (fortread(frame_size, 4, 2, slice_file->file)) return -1;
      compressed_size = frame_size[1];
      // Seek over the compressed data
      if (fortseek(slice_file->file, 1, frame_size[1], SEEK_CUR)) return 5;
      fprintf(sizestream, "%f %i %i\n", time, frame_size[0], frame_size[1]);
    case COMP_ZLIB:
      if (!fread(&time, 4, 1, slice_file->file)) return -1;
      // Read the compressed size (in bytes)
      if (!fread(&compressed_size, 4, 1, slice_file->file)) return -1;
      // Seek over the compressed data
      if (!fseek(slice_file->file, compressed_size, SEEK_CUR)) return -1;
      fprintf(sizestream, "%f %i %i\n", time, compressed_size, 0);
      break;
    }
    line_count++;

  } while (slcfr_seek_next(slice_file));
  return line_count;
}

// size file stores compressed data offsets. This adds a size files to the
// SlcfR. Decisions need to be made on how we store this.
// int slcfr_add_size_file(SlcfR *slice_file, const char *size_file) {
//   uint32_t ijkbar[6];
//   float time_local;
//   int ncompressed;
//   int count;
//   char buffer[256];
//   int ncompressed_rle, ncompressed_zlib;

//   FILE *stream = FOPEN(size_file, "rb");
//   if (stream == NULL) return 1;

//   if (fgets(buffer, 255, stream) == NULL) {
//     fclose(stream);
//     return 0;
//   }
//   if (sscanf(buffer, "%u %u %u %u %u %u", &ijkbar[0], &ijkbar[1], &ijkbar[2],
//              &ijkbar[3], &ijkbar[4], &ijkbar[5]))
//     return -1;
//   if (fgets(buffer, 255, stream) == NULL) {
//     fclose(stream);
//     return 0;
//   }
//   if (sscanf(buffer, "%f %f", valmin, valmax) < 2) return -1;

//   count = 0;
//   *nsteps = 0;
//   *ntotal = 0;
//   while (!feof(stream)) {
//     if (fgets(buffer, 255, stream) == NULL) break;
//     if (sscanf(buffer, "%f %i %i", &time_local, &ncompressed_zlib,
//                &ncompressed_rle))
//       return -1;
//     if (slice_file->compression_type == COMP_ZLIB) {
//       ncompressed = ncompressed_zlib;
//     } else {
//       ncompressed = ncompressed_rle;
//     }
//     if (count++ % framestep != 0) continue;
//     if (set_tmin == 1 && time_local < tmin_local) continue;
//     if (set_tmax == 1 && time_local > tmax_local) continue;
//     (*nsteps)++;
//     *ntotal += ncompressed;
//   }
//   fclose(stream);
//   return 2 + *nsteps;
// }

// size file stores compressed data offsets. This adds a size files to the
// SlcfR. Decisions need to be made on how we store this.

/// @brief Given a text size file, create a binary size file.
/// @param text_size_path The path to the text size file.
/// @param binary_size_path The path to the binary size file.
/// @return 0 on success
/// @return nonzero on failure.
int create_binary_size_file(const char *text_size_path,
                            const char *binary_size_path) {
  uint32_t ijkbar[6];
  float minmax[2];
  float time_local;
  char buffer[256];
  int error = 0;
  uint32_t ncompressed_rle, ncompressed_zlib;
  FILE *text_stream = FOPEN(text_size_path, "r");
  if (text_stream == NULL) return 1;
  FILE *binary_stream = FOPEN(binary_size_path, "wb");
  if (binary_stream == NULL) {
    // If we get an error while opening the binary file, we must remember to
    // close the text stream before returning.
    fclose(text_stream);
    return 1;
  }
  // From this point on, both file handles are open. Therefore, on error, we
  // jump to the 'error_cleanup' label to do cleanup (including closing the file
  // handles).
  if (fgets(buffer, 255, text_stream) == NULL) {
    error = 3;
    goto error_cleanup;
  }
  int read = sscanf(buffer, "%u %u %u %u %u %u", &ijkbar[0], &ijkbar[1],
                    &ijkbar[2], &ijkbar[3], &ijkbar[4], &ijkbar[5]);
  if (read == 0) {
    error = -1;
    goto error_cleanup;
  }
  fwrite(ijkbar, sizeof(uint32_t), 6, binary_stream);
  if (fgets(buffer, 255, text_stream) == NULL) {
    error = 3;
    goto error_cleanup;
  }
  if (sscanf(buffer, "%f %f", &minmax[0], &minmax[1]) < 2) {
    error = -1;
    goto error_cleanup;
  }
  fwrite(minmax, sizeof(float), 2, binary_stream);

  while (!feof(text_stream)) {
    if (fgets(buffer, 255, text_stream) == NULL) break;
    if (!sscanf(buffer, "%f %i, %i", &time_local, &ncompressed_zlib,
                &ncompressed_rle)) {
      error = -1;
      goto error_cleanup;
    }
    fwrite(&time_local, sizeof(float), 1, binary_stream);
    fwrite(&ncompressed_zlib, sizeof(uint32_t), 1, binary_stream);
    fwrite(&ncompressed_rle, sizeof(uint32_t), 1, binary_stream);
  }
error_cleanup:
  // Cleanup file handles and return error code
  fclose(text_stream);
  fclose(binary_stream);
  return error;
}

#include "dmalloc.h"
#include "options.h"
#include "smokeviewvars.h"
#include "structures.h"
#include <assert.h>
#include <stdio.h>

#include "io.h"

/* ------------------ OutSlicefile ------------------------ */

void OutSlicefile(slicedata *sd) {
  writeslicedata(sd->file, sd->is1, sd->is2, sd->js1, sd->js2, sd->ks1, sd->ks2,
                 sd->qslicedata, sd->times, sd->ntimes, redirect);
}

//*** header
// endian
// completion (0/1)
// fileversion (compressed format)
// version  (slicef version)
// global min max (used to perform conversion)
// i1,i2,j1,j2,k1,k2

//*** frame
// time, compressed frame size                        for each frame
// compressed buffer

/* ------------------ MakeSliceSizefile ------------------------ */

int MakeSliceSizefile(char *file, char *sizefile, int compression_type) {
  FILE *stream, *sizestream;
  int count;

  stream = FOPEN(file, "rb");
  if (stream == NULL) return 0;

  sizestream = fopen(sizefile, "w");
  if (sizestream == NULL) {
    fclose(stream);
    return 0;
  }
  count = 0;
  if (compression_type != UNCOMPRESSED) {
    float minmax[2];
    int ijkbar[6];

    // endian
    // completion (0/1)
    // fileversion (compressed format)
    // version_local  (slicef version)
    // global min max (used to perform conversion)
    // i1,i2,j1,j2,k1,k2

#define FSKIP                                                                  \
  if (compression_type == COMPRESSED_RLE) FSEEK(stream, 4, SEEK_CUR)

    FSKIP;
    FSEEK(stream, 4, SEEK_CUR);
    FSKIP;
    FSKIP;
    FSEEK(stream, 12, SEEK_CUR);
    FSKIP;
    FSKIP;
    fread(minmax, 4, 2, stream);
    FSKIP;
    FSKIP;
    fread(ijkbar, 4, 6, stream);
    FSKIP;

    fprintf(sizestream, "%i %i %i %i %i %i\n", ijkbar[0], ijkbar[1], ijkbar[2],
            ijkbar[3], ijkbar[4], ijkbar[5]);
    fprintf(sizestream, "%f %f\n", minmax[0], minmax[1]);
    count = 2;

    while (!feof(stream)) {
      float time_local;
      int ncompressed, noriginal;

      //  time
      //  original frame size, compressed frame size (rle)
      //  compressed frame size                      (zlib)
      //  compressed buffer
      FSKIP;
      if (fread(&time_local, 4, 1, stream) != 1) break;
      FSKIP;
      if (compression_type == COMPRESSED_RLE) {
        FSEEK(stream, 4, SEEK_CUR);
        fread(&noriginal, 4, 1, stream);
        fread(&ncompressed, 4, 1, stream);
        FSEEK(stream, 4, SEEK_CUR);
      }
      else {
        fread(&ncompressed, 4, 1, stream);
      }
      if (compression_type == COMPRESSED_ZLIB)
        fprintf(sizestream, "%f %i %i\n", time_local, ncompressed, 0);
      if (compression_type == COMPRESSED_RLE)
        fprintf(sizestream, "%f %i %i\n", time_local, noriginal, ncompressed);
      count++;

      FSKIP;
      FSEEK(stream, ncompressed, SEEK_CUR);
      FSKIP;
    }
  }
  fclose(stream);
  fclose(sizestream);
  return count;
}

/* ------------------ GetSliceHeader0 ------------------------ */

int GetSliceHeader0(char *comp_file, char *size_file, int compression_type,
                    int *i1, int *i2, int *jj1, int *j2, int *k1, int *k2,
                    int *slice3d) {
  FILE *stream;
  char buffer[255];

  stream = FOPEN(size_file, "r");
  if (stream == NULL) {
    if (MakeSliceSizefile(comp_file, size_file, compression_type) == 0)
      return 0;
    stream = FOPEN(size_file, "r");
    if (stream == NULL) return 0;
  }

  if (fgets(buffer, 255, stream) == NULL) {
    fclose(stream);
    return 0;
  }
  sscanf(buffer, "%i %i %i %i %i %i", i1, i2, jj1, j2, k1, k2);
  if (*i1 == *i2 || *jj1 == *j2 || *k1 == *k2) {
    *slice3d = 0;
  }
  else {
    *slice3d = 1;
  }
  fclose(stream);
  return 1;
}

/* ------------------ GetSliceHeader ------------------------ */

int GetSliceHeader(char *comp_file, char *size_file, int compression_type,
                   int framestep, int set_tmin, int set_tmax, float tmin_local,
                   float tmax_local, int *nx, int *ny, int *nz, int *nsteps,
                   int *ntotal, float *valmin, float *valmax) {
  FILE *stream;
  int i1, i2, jj1, j2, k1, k2;
  float time_local;
  int ncompressed;
  int count;
  char buffer[256];
  int ncompressed_rle, ncompressed_zlib;

  stream = FOPEN(size_file, "r");
  if (stream == NULL) {
    if (MakeSliceSizefile(comp_file, size_file, compression_type) == 0)
      return 0;
    stream = fopen(size_file, "r");
    if (stream == NULL) return 0;
  }

  if (fgets(buffer, 255, stream) == NULL) {
    fclose(stream);
    return 0;
  }
  sscanf(buffer, "%i %i %i %i %i %i", &i1, &i2, &jj1, &j2, &k1, &k2);
  *nx = i2 + 1 - i1;
  *ny = j2 + 1 - jj1;
  *nz = k2 + 1 - k1;
  if (fgets(buffer, 255, stream) == NULL) {
    fclose(stream);
    return 0;
  }
  sscanf(buffer, "%f %f", valmin, valmax);

  count = 0;
  *nsteps = 0;
  *ntotal = 0;
  while (!feof(stream)) {

    if (fgets(buffer, 255, stream) == NULL) break;
    sscanf(buffer, "%f %i %i", &time_local, &ncompressed_zlib,
           &ncompressed_rle);
    if (compression_type == COMPRESSED_ZLIB) {
      ncompressed = ncompressed_zlib;
    }
    else {
      ncompressed = ncompressed_rle;
    }
    if (count++ % framestep != 0) continue;
    if (set_tmin == 1 && time_local < tmin_local) continue;
    if (set_tmax == 1 && time_local > tmax_local) continue;
    (*nsteps)++;
    *ntotal += ncompressed;
  }
  fclose(stream);
  return 2 + *nsteps;
}

/* ------------------ CReadSlice_frame ------------------------ */

int CReadSlice_frame(int frame_index_local, int sd_index, int flag) {
  slicedata *sd;
  int headersize, framesize;
  int frame_size;
  long int skip_local;
  FILEBUFFER *SLICEFILE = NULL;
  float *time_local, *slicevals;
  int error;
  int returncode = 0;

  sd = sliceinfo + sd_index;
  if (sd->loaded == 1)
    ReadSlice(sd->file, sd_index, ALL_FRAMES, NULL, UNLOAD, SET_SLICECOLOR,
              &error);
  if (flag == UNLOAD) {
    FREEMEMORY(sd->qslicedata);
    FREEMEMORY(sd->times);
    return 0;
  }
  if (frame_index_local == first_frame_index) {
    if (sd->compression_type == UNCOMPRESSED) {

      GetSliceSizes(sd->file, ALL_FRAMES, &sd->nslicei, &sd->nslicej,
                    &sd->nslicek, &sd->ntimes, tload_step, &error,
                    use_tload_begin, use_tload_end, tload_begin, tload_end,
                    &headersize, &framesize);
    }
    else if (sd->compression_type != UNCOMPRESSED) {
      if (GetSliceHeader(sd->comp_file, sd->size_file, sd->compression_type,
                         tload_step, use_tload_begin, use_tload_end,
                         tload_begin, tload_end, &sd->nslicei, &sd->nslicej,
                         &sd->nslicek, &sd->ntimes, &sd->ncompressed,
                         &sd->valmin_slice, &sd->valmax_slice) == 0) {
        ReadSlice("", sd_index, ALL_FRAMES, NULL, UNLOAD, SET_SLICECOLOR,
                  &error);
        return -1;
      }
    }
  }
  skip_local = (HEADER_SIZE + 30 + TRAILER_SIZE);  // long label
  skip_local += (HEADER_SIZE + 30 + TRAILER_SIZE); // short label
  skip_local += (HEADER_SIZE + 30 + TRAILER_SIZE); // unit label
  skip_local +=
      (HEADER_SIZE + 6 * 4 + TRAILER_SIZE); // is1, is2, js1, js2, ks1, ks2

  frame_size = sd->nslicei * sd->nslicej * sd->nslicek;
  skip_local += frame_index_local * (HEADER_SIZE + 4 + TRAILER_SIZE); //
  skip_local +=
      frame_index_local * (HEADER_SIZE + frame_size * 4 + TRAILER_SIZE); //

  if (SLICEFILE == NULL) {
    SLICEFILE = FOPEN_SLICE(sd->file, "rb");
  }
  if (SLICEFILE == NULL) {
    return -1;
  }

  FSEEK_SLICE(SLICEFILE, skip_local, SEEK_SET); // skip from beginning of file

  if (frame_index_local == first_frame_index) {
    if (NewMemory((void **)&sd->qslicedata, 2 * frame_size * sizeof(float)) ==
            0 ||
        NewMemory((void **)&sd->times, sizeof(float)) == 0) {
      FCLOSE_SLICE(SLICEFILE);
      return -1;
    }
  }
  slicevals = sd->qslicedata;
  if (frame_index_local % 2 != 0) {
    slicevals += frame_size;
  }
  time_local = sd->times;

  FORT_SLICEREAD(time_local, 1, SLICEFILE);
  FORT_SLICEREAD(slicevals, frame_size, SLICEFILE);
  FCLOSE_SLICE(SLICEFILE);
  return 0;
}

/* ------------------ OutputFedCSV ------------------------ */

void OutputFedCSV(void) {
  FILE *AREA_STREAM = NULL;
  char *fed_area_file = NULL, fed_area_file_base[1024];

  if (fed_areas == NULL) return;
  sprintf(fed_area_file_base, "%s_s%04i_fedarea.csv", fdsprefix, fed_seqnum++);
  fed_area_file = fed_area_file_base;
  AREA_STREAM = fopen(fed_area_file, "w");
  if (AREA_STREAM == NULL) return;

  fprintf(AREA_STREAM,
          "\"step\",\"0.0->0.3\",\"0.3->1.0\",\"1.0->3.0\",\"3.0->\"\n");
  fclose(AREA_STREAM);
}

/* ------------------ ReadFed ------------------------ */

void ReadFed(int file_index, int time_frame, float *time_value, int flag,
             int file_type, int *errorcode) {
  feddata *fedi;
  slicedata *fed_slice, *o2, *co2, *co;
  isodata *fed_iso;
  int error_local;
  meshdata *meshi;
  int nx, ny;
  int nxy;
  int nxdata, nydata;
  int ibar, jbar, kbar;
  slicedata *slicei = NULL;

#define FEDCO(CO)                                                              \
  ((2.764 / 100000.0) * pow(1000000.0 * CLAMP(CO, 0.0, 0.1), 1.036) / 60.0)
#define FEDO2(O2)                                                              \
  (exp(-(8.13 - 0.54 * (20.9 - 100.0 * CLAMP(O2, 0.0, 0.2)))) / 60.0)
#define HVCO2(CO2) (exp(0.1930 * CLAMP(CO2, 0.0, 0.1) * 100.0 + 2.0004) / 7.1)

  assert(fedinfo != NULL);
  assert(file_index >= 0);
  if (file_type == FED_SLICE) {

    assert(file_index < nsliceinfo);
    slicei = sliceinfo + file_index;
    fedi = slicei->fedptr;
  }
  else if (file_type == FED_ISO) {
    isodata *isoi;

    assert(file_index < nisoinfo);
    isoi = isoinfo + file_index;
    fedi = isoi->fedptr;
  }
  else {
    return;
  }
  o2 = fedi->o2;
  co2 = fedi->co2;
  co = fedi->co;
  fed_slice = fedi->fed_slice;
  fed_iso = fedi->fed_iso;
  meshi = meshinfo + fed_slice->blocknumber;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  kbar = meshi->kbar;

  nx = meshi->ibar + 1;
  ny = meshi->jbar + 1;
  nxy = nx * ny;

  switch (fed_slice->idir) {
  case XDIR:
    nxdata = co->js2 + 1 - co->js1;
    nydata = co->ks2 + 1 - co->ks1;
    break;
  case YDIR:
    nxdata = co->is2 + 1 - co->is1;
    nydata = co->ks2 + 1 - co->ks1;
    break;
  case ZDIR:
    nxdata = co->is2 + 1 - co->is1;
    nydata = co->js2 + 1 - co->js1;
    break;
  default:
    assert(FFALSE);
    break;
  }

  if (file_type == FED_SLICE) {
    ReadSlice(fed_slice->file, fedi->fed_index, ALL_FRAMES, NULL, UNLOAD,
              SET_SLICECOLOR, &error_local);
  }
  else if (file_type == FED_ISO) {
    ReadIsoOrig(fed_iso->file, file_index, UNLOAD, &error_local);
  }

  if (flag == UNLOAD) {
    return;
  }

  // regenerate if either the FED slice or isosurface file does not exist or is
  // older than either the CO, CO2 or O2 slice files

#ifndef pp_BOUNDS
  if (file_type == FED_SLICE) {
    FILE *stream;

    stream = fopen(slicei->bound_file, "r");
    if (stream != NULL) {
      slicei->have_bound_file = 1;
      fclose(stream);
      UpdateGlobalFEDSliceBounds();
    }
    else {
      slicei->have_bound_file = 0;
      regenerate_fed = 1;
    }
  }
#endif
  if (regenerate_fed == 1 ||
      (file_type == FED_SLICE &&
       (IsFileNewer(fed_slice->file, o2->file) != 1 ||
        IsFileNewer(fed_slice->file, co2->file) != 1 ||
        IsFileNewer(fed_slice->file, co->file) != 1)) ||
      (file_type == FED_ISO && (IsFileNewer(fed_iso->file, o2->file) != 1 ||
                                IsFileNewer(fed_iso->file, co2->file) != 1 ||
                                IsFileNewer(fed_iso->file, co->file) != 1))) {
    int i, j, k;
    int frame_size;
    float *fed_frame, *fed_framem1;
    float *o2_frame1, *o2_frame2;
    float *co2_frame1, *co2_frame2;
    float *co_frame1, *co_frame2;
    float *times;

    char *iblank;

    NewMemory((void **)&iblank, nxdata * nydata * sizeof(char));
    for (j = 0; j < nxdata - 1; j++) {
      for (k = 0; k < nydata - 1; k++) {
        iblank[j + k * (nxdata - 1)] = 0;
      }
    }
    switch (fed_slice->idir) {
    case XDIR:
      if (meshi->c_iblank_x != NULL) {
        for (j = 0; j < nxdata - 1; j++) {
          for (k = 0; k < nydata - 1; k++) {
            iblank[j + k * (nxdata - 1)] = meshi->c_iblank_x[IJKNODE(
                fed_slice->is1, fed_slice->js1 + j, fed_slice->ks1 + k)];
          }
        }
      }
      break;
    case YDIR:
      if (meshi->c_iblank_y != NULL) {
        for (i = 0; i < nxdata - 1; i++) {
          for (k = 0; k < nydata - 1; k++) {
            iblank[i + k * (nxdata - 1)] = meshi->c_iblank_y[IJKNODE(
                fed_slice->is1 + i, fed_slice->js1, fed_slice->ks1 + k)];
          }
        }
      }
      break;
    case ZDIR:
      if (meshi->c_iblank_z != NULL) {
        for (i = 0; i < nxdata - 1; i++) {
          for (j = 0; j < nydata - 1; j++) {
            iblank[i + j * (nxdata - 1)] = meshi->c_iblank_z[IJKNODE(
                fed_slice->is1 + i, fed_slice->js1 + j, fed_slice->ks1)];
          }
        }
      }
      break;
    default:
      assert(FFALSE);
      break;
    }
    PRINTF("\n");
    PRINTF("generating FED slice data\n");
    if (CReadSlice_frame(0, fedi->o2_index, LOAD) < 0 ||
        CReadSlice_frame(0, fedi->co2_index, LOAD) < 0 ||
        CReadSlice_frame(0, fedi->co_index, LOAD) < 0) {

      ReadFed(file_index, time_frame, time_value, UNLOAD, file_type, errorcode);
      return;
    }
    fed_slice->is1 = co->is1;
    fed_slice->is2 = co->is2;
    if (fed_slice->is1 != fed_slice->is2 && fed_slice->is1 == 1)
      fed_slice->is1 = 0;
    fed_slice->js1 = co->js1;
    fed_slice->js2 = co->js2;
    if (fed_slice->js1 != fed_slice->js2 && fed_slice->js1 == 1)
      fed_slice->js1 = 0;
    fed_slice->ks1 = co->ks1;
    fed_slice->ks2 = co->ks2;
    if (fed_slice->ks1 != fed_slice->ks2 && fed_slice->ks1 == 1)
      fed_slice->ks1 = 0;
    fed_slice->nslicei = co->nslicei;
    fed_slice->nslicej = co->nslicej;
    fed_slice->nslicek = co->nslicek;
    fed_slice->volslice = co->volslice;
    fed_slice->iis1 = co->iis1;
    fed_slice->iis2 = co->iis2;
    fed_slice->jjs1 = co->jjs1;
    fed_slice->jjs2 = co->jjs2;
    fed_slice->kks1 = co->kks1;
    fed_slice->kks2 = co->kks2;
    fed_slice->plotx = co->plotx;
    fed_slice->ploty = co->ploty;
    fed_slice->plotz = co->plotz;
    fed_slice->ijk_min[0] = co->is1;
    fed_slice->ijk_min[1] = co->js1;
    fed_slice->ijk_min[2] = co->ks1;
    fed_slice->ijk_max[0] = co->is2;
    fed_slice->ijk_max[1] = co->js2;
    fed_slice->ijk_max[2] = co->ks2;
    if (fed_slice->volslice == 1) {
      if (fed_slice->nslicei != fed_slice->is2 + 1 - fed_slice->is1)
        fed_slice->is2 = fed_slice->nslicei + fed_slice->is1 - 1;
      if (fed_slice->nslicej != fed_slice->js2 + 1 - fed_slice->js1)
        fed_slice->js2 = fed_slice->nslicej + fed_slice->js1 - 1;
      if (fed_slice->nslicek != fed_slice->ks2 + 1 - fed_slice->ks1)
        fed_slice->ks2 = fed_slice->nslicek + fed_slice->ks1 - 1;
    }
    fed_slice->ntimes = MIN(co->ntimes, co2->ntimes);
    fed_slice->ntimes = MIN(fed_slice->ntimes, o2->ntimes);
    frame_size = fed_slice->nslicei * fed_slice->nslicej * fed_slice->nslicek;
    fed_slice->nslicetotal = frame_size * fed_slice->ntimes;

    if (NewMemory((void **)&fed_slice->qslicedata,
                  sizeof(float) * frame_size * fed_slice->ntimes) == 0 ||
        NewMemory((void **)&fed_slice->times,
                  sizeof(float) * fed_slice->ntimes) == 0 ||
        NewResizeMemory(fed_slice->times_map, fed_slice->ntimes) == 0) {
      ReadFed(file_index, time_frame, NULL, UNLOAD, file_type, errorcode);
      *errorcode = -1;
    }
    times = fed_slice->times;
    fed_frame = fed_slice->qslicedata;

    o2_frame1 = o2->qslicedata;
    o2_frame2 = o2_frame1 + frame_size;

    co2_frame1 = co2->qslicedata;
    co2_frame2 = co2_frame1 + frame_size;

    co_frame1 = co->qslicedata;
    co_frame2 = co_frame1 + frame_size;

    times[0] = co2->times[0];
    for (i = 0; i < frame_size; i++) {
      fed_frame[i] = 0.0;
    }
    float fed_valmin = 1.0, fed_valmax = 0.0;
    fed_valmin = 0.0;
    fed_valmax = 0.0;
    for (i = 1; i < fed_slice->ntimes; i++) {
      int jj;
      float dt;

      if (CReadSlice_frame(i, fedi->o2_index, LOAD) < 0 ||
          CReadSlice_frame(i, fedi->co2_index, LOAD) < 0 ||
          CReadSlice_frame(i, fedi->co_index, LOAD)) {
        ReadFed(file_index, time_frame, NULL, UNLOAD, file_type, errorcode);
        return;
      }

      times[i] = co2->times[0];
      PRINTF("generating FED time=%.2f\n", times[i]);
      dt = (times[i] - times[i - 1]);

      fed_framem1 = fed_frame;
      fed_frame += frame_size;
      for (jj = 0; jj < frame_size; jj++) {
        float val1, val2;
        float fed_co_val, fed_o2_val;

        val1 = FEDCO(co_frame1[jj]) * HVCO2(co2_frame1[jj]);
        val2 = FEDCO(co_frame2[jj]) * HVCO2(co2_frame2[jj]);
        fed_co_val = (val1 + val2) * dt / 2.0;

        val1 = FEDO2(o2_frame1[jj]);
        val2 = FEDO2(o2_frame2[jj]);
        fed_o2_val = (val1 + val2) * dt / 2.0;

        fed_frame[jj] = fed_framem1[jj] + fed_co_val + fed_o2_val;
        fed_valmin = MIN(fed_valmin, fed_frame[jj]);
        fed_valmax = MAX(fed_valmax, fed_frame[jj]);
      }
    }
    FREEMEMORY(iblank);
    if (file_type == FED_SLICE) {
      FILE *stream = NULL;

      stream = fopen(slicei->bound_file, "w");
      if (stream != NULL) {
        fprintf(stream, "0.0 %f %f\n", fed_valmin, fed_valmax);
        fclose(stream);
        slicei->have_bound_file = 1;
        UpdateGlobalFEDSliceBounds();
      }
      else {
        slicei->have_bound_file = 0;
      }
    }
    OutSlicefile(fed_slice);
    if (fed_slice->volslice == 1) {
      float *xplt, *yplt, *zplt;
      char *iblank_cell;
      char longlabel[50], shortlabel[50], unitlabel[50];
      char *isofile;
      int error_local2;
      int reduce_triangles = 1;
      int nz;

      strcpy(longlabel, "Fractional effective dose");
      strcpy(shortlabel, "FED");
      strcpy(unitlabel, " ");

      xplt = meshi->xplt;
      yplt = meshi->yplt;
      zplt = meshi->zplt;
      ibar = meshi->ibar;
      jbar = meshi->jbar;
      kbar = meshi->kbar;
      nx = ibar + 1;
      ny = jbar + 1;
      nz = kbar + 1;
      isofile = fed_iso->file;

      iblank_cell = meshi->c_iblank_cell;

      CCIsoHeader(isofile, longlabel, shortlabel, unitlabel, fed_iso->levels,
                  &fed_iso->nlevels, &error_local2);
      PRINTF("generating FED isosurface\n");
      for (i = 0; i < fed_slice->ntimes; i++) {
        float *vals;

        vals = fed_slice->qslicedata + i * frame_size;
        PRINTF("outputting isotime time=%.2f\n", times[i]);

        //    C_val(i,j,k) = i*nj*nk + j*nk + k
        // Fort_val(i,j,k) = i + j*ni + k*ni*nj

        CCIsoSurface2File(isofile, times + i, vals, iblank_cell,
                          fed_iso->levels, &fed_iso->nlevels, xplt, &nx, yplt,
                          &ny, zplt, &nz, &reduce_triangles, &error_local2);
      }
    }
    FREEMEMORY(fed_slice->qslicedata);
    FREEMEMORY(fed_slice->times);
    FREEMEMORY(fed_slice->times_map);
    CReadSlice_frame(0, fedi->o2_index, UNLOAD);
    CReadSlice_frame(0, fedi->co2_index, UNLOAD);
    CReadSlice_frame(0, fedi->co_index, UNLOAD);
  }
  if (file_type == FED_SLICE) {
    ReadSlice(fed_slice->file, fedi->fed_index, ALL_FRAMES, NULL, flag,
              SET_SLICECOLOR, &error_local);
  }
  else {
    ReadIsoOrig(fed_iso->file, file_index, flag, &error_local);
  }
#ifdef pp_BOUNDS
  UpdateGlobalFEDSliceBounds();
#endif
  {
    colorbardata *cb;

    cb = GetColorbar(default_fed_colorbar);
    if (cb != NULL) {
      colorbartype = cb - colorbarinfo;
      GLUISetColorbarListEdit(colorbartype);
      GLUISliceBoundCB(COLORBAR_LIST2);
      UpdateCurrentColorbar(cb);
    }
  }
}

/* ------------------ ReadVSlice ------------------------ */

FILE_SIZE ReadVSlice(int ivslice, int time_frame, float *time_value, int flag,
                     int set_slicecolor, int *errorcode) {
  vslicedata *vd;
  float valmin, valmax;
  int display;
  int i;
  FILE_SIZE return_filesize = 0;
  int finalize = 0;

  valmin = 1000000000.0;
  valmax = -valmin;
  vd = vsliceinfo + ivslice;
  vd->u = NULL;
  vd->v = NULL;
  vd->w = NULL;
  vd->val = NULL;

  if (vd->iu != -1) sliceinfo[vd->iu].uvw = 1;
  if (vd->iv != -1) sliceinfo[vd->iv].uvw = 1;
  if (vd->iw != -1) sliceinfo[vd->iw].uvw = 1;
  if (flag == UNLOAD) {
    if (vd->loaded == 0) return 0;
    if (vd->iu != -1) {
      slicedata *u = NULL;

      u = sliceinfo + vd->iu;
      display = u->display;
      if (u->loaded == 1) {
        if (u->slice_filetype == SLICE_GEOM) {
          return_filesize = ReadGeomData(u->patchgeom, u, UNLOAD, time_frame,
                                         time_value, 0, errorcode);
        }
        else {
          return_filesize += ReadSlice(u->file, vd->iu, time_frame, NULL,
                                       UNLOAD, DEFER_SLICECOLOR, errorcode);
        }
      }
      u->display = display;
      u->vloaded = 0;
    }
    if (vd->iv != -1) {
      slicedata *v = NULL;

      v = sliceinfo + vd->iv;
      display = v->display;
      if (v->loaded == 1) {
        if (v->slice_filetype == SLICE_GEOM) {
          return_filesize = ReadGeomData(v->patchgeom, v, UNLOAD, time_frame,
                                         time_value, 0, errorcode);
        }
        else {
          return_filesize += ReadSlice(v->file, vd->iv, time_frame, NULL,
                                       UNLOAD, DEFER_SLICECOLOR, errorcode);
        }
      }
      v->display = display;
      v->vloaded = 0;
    }
    if (vd->iw != -1) {
      slicedata *w = NULL;

      w = sliceinfo + vd->iw;
      display = w->display;
      if (w->loaded == 1) {
        if (w->slice_filetype == SLICE_GEOM) {
          return_filesize = ReadGeomData(w->patchgeom, w, UNLOAD, time_frame,
                                         time_value, 0, errorcode);
        }
        else {
          return_filesize += ReadSlice(w->file, vd->iw, time_frame, NULL,
                                       UNLOAD, DEFER_SLICECOLOR, errorcode);
        }
      }
      w->display = display;
      w->vloaded = 0;
    }
    if (vd->ival != -1) {
      slicedata *val = NULL;

      val = sliceinfo + vd->ival;
      display = val->display;
      if (val->loaded == 1) {
        if (val->slice_filetype == SLICE_GEOM) {
          return_filesize = ReadGeomData(val->patchgeom, val, UNLOAD,
                                         time_frame, time_value, 0, errorcode);
        }
        else {
          return_filesize += ReadSlice(val->file, vd->ival, time_frame, NULL,
                                       UNLOAD, set_slicecolor, errorcode);
        }
      }
      val->display = display;
      val->vloaded = 0;
    }
    RemoveSliceLoadstack(&vslice_loadstack, ivslice);
    vd->loaded = 0;
    vd->display = 0;
    showvslice = 0;
    updatemenu = 1;
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    update_vectorskip = 1;
    return return_filesize;
  }
  if (vd->finalize == 0) set_slicecolor = DEFER_SLICECOLOR;
#ifdef pp_BOUNDS

  int set_valmin_save, set_valmax_save;
  float qmin_save, qmax_save;
  if (vd->finalize == 1 && vd->ival != -1) {
    slicedata *sd = NULL;

    sd = sliceinfo + vd->ival;
    GLUIGetMinMax(BOUND_SLICE, sd->label.shortlabel, &set_valmin_save,
                  &qmin_save, &set_valmax_save, &qmax_save);
  }
#endif
  if (vd->iu != -1) {
    slicedata *u = NULL;

    u = sliceinfo + vd->iu;
    u->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->u = u;
    if (scriptoutstream == NULL || script_defer_loading == 0) {
      if (u->slice_filetype == SLICE_GEOM) {
        return_filesize += ReadGeomData(u->patchgeom, u, LOAD, time_frame,
                                        time_value, 0, errorcode);
      }
      else {
        return_filesize += ReadSlice(u->file, vd->iu, time_frame, time_value,
                                     flag, set_slicecolor, errorcode);
      }
      if (*errorcode != 0) {
        vd->loaded = 1;
        fprintf(stderr,
                "*** Error: unable to load U velocity vector components in %s "
                ". Vector load aborted\n",
                u->file);
        ReadVSlice(ivslice, time_frame, time_value, UNLOAD, DEFER_SLICECOLOR,
                   errorcode);
        *errorcode = 1;
        return 0;
      }
      if (u->valmin_slice < valmin) valmin = u->valmin_slice;
      if (u->valmax_slice > valmax) valmax = u->valmax_slice;
      u->display = 0;
      u->vloaded = 1;
    }
  }
  if (vd->iv != -1) {
    slicedata *v = NULL;

    v = sliceinfo + vd->iv;
    v->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->v = v;
    if (scriptoutstream == NULL || script_defer_loading == 0) {
      if (v->slice_filetype == SLICE_GEOM) {
        return_filesize += ReadGeomData(v->patchgeom, v, LOAD, time_frame,
                                        time_value, 0, errorcode);
      }
      else {
        return_filesize += ReadSlice(v->file, vd->iv, time_frame, time_value,
                                     flag, set_slicecolor, errorcode);
      }
      if (*errorcode != 0) {
        fprintf(stderr,
                "*** Error: unable to load V velocity vector components in %s "
                ". Vector load aborted\n",
                v->file);
        vd->loaded = 1;
        ReadVSlice(ivslice, time_frame, time_value, UNLOAD, DEFER_SLICECOLOR,
                   errorcode);
        *errorcode = 1;
        return 0;
      }

      if (v->valmin_slice < valmin) valmin = v->valmin_slice;
      if (v->valmax_slice > valmax) valmax = v->valmax_slice;
      v->display = 0;
      v->vloaded = 1;
    }
  }
  if (vd->iw != -1) {
    slicedata *w = NULL;

    w = sliceinfo + vd->iw;
    w->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->w = w;
    if (scriptoutstream == NULL || script_defer_loading == 0) {
      if (w->slice_filetype == SLICE_GEOM) {
        return_filesize += ReadGeomData(w->patchgeom, w, LOAD, time_frame,
                                        time_value, 0, errorcode);
      }
      else {
        return_filesize += ReadSlice(w->file, vd->iw, time_frame, time_value,
                                     flag, set_slicecolor, errorcode);
      }
      if (*errorcode != 0) {
        fprintf(stderr,
                "*** Error: unable to load W velocity vector components in %s "
                ". Vector load aborted\n",
                w->file);
        vd->loaded = 1;
        ReadVSlice(ivslice, time_frame, time_value, UNLOAD, DEFER_SLICECOLOR,
                   errorcode);
        *errorcode = 1;
        return 0;
      }

      if (w->valmin_slice < valmin) valmin = w->valmin_slice;
      if (w->valmax_slice > valmax) valmax = w->valmax_slice;
      w->display = 0;
      w->vloaded = 1;
    }
  }
  vd->vslicefile_labelindex = -1;
  if (vd->ival != -1) {
    slicedata *val = NULL;

    val = sliceinfo + vd->ival;
    val->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->val = val;
    if (scriptoutstream == NULL || script_defer_loading == 0) {
      if (val->slice_filetype == SLICE_GEOM) {
        return_filesize += ReadGeomData(val->patchgeom, val, LOAD, time_frame,
                                        time_value, 0, errorcode);
      }
      else {
        return_filesize +=
            ReadSlice(val->file, vd->ival, time_frame, time_value, flag,
                      set_slicecolor, errorcode);
      }
      if (*errorcode != 0) {
        fprintf(stderr,
                "*** Error: unable to load vector values in %s . Vector load "
                "aborted\n",
                val->file);
        vd->loaded = 1;
        ReadVSlice(ivslice, time_frame, time_value, UNLOAD, DEFER_SLICECOLOR,
                   errorcode);
        *errorcode = 1;
        return 0;
      }
      slicefile_labelindex = GetSliceBoundsIndex(val);
      vd->vslicefile_labelindex = val->slicefile_labelindex;
      vd->valmin = valmin;
      vd->valmax = valmax;
      val->display = 0;
      val->vloaded = 1;
    }
  }
  vd->display = 1;
  vd->loaded = 1;
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  updatemenu = 1;
  if (finalize == 1) {
    UpdateTimes();

    valmax = -100000.0;
    valmin = 100000.0;
    for (i = 0; i < nvsliceinfo; i++) {
      vslicedata *vslicei;

      vslicei = vsliceinfo + i;
      if (vslicei->loaded == 0) continue;
      if (vslicei->iu != -1) {
        slicedata *u = NULL;

        u = sliceinfo + vslicei->iu;
        valmin = MIN(u->valmin_slice, valmin);
        valmax = MAX(u->valmax_slice, valmax);
      }
      if (vslicei->iv != -1) {
        slicedata *v = NULL;

        v = sliceinfo + vslicei->iv;
        valmin = MIN(v->valmin_slice, valmin);
        valmax = MAX(v->valmax_slice, valmax);
      }
      if (vslicei->iw != -1) {
        slicedata *w = NULL;

        w = sliceinfo + vslicei->iw;
        valmin = MIN(w->valmin_slice, valmin);
        valmax = MAX(w->valmax_slice, valmax);
      }
    }
    max_velocity = MAX(ABS(valmax), ABS(valmin));
#ifdef pp_BOUNDS
    if (vd->ival != -1) {
      slicedata *sd = NULL;

      sd = sliceinfo + vd->ival;
      if (set_valmin_save == 0) {
        SetSliceMin(set_valmin_save, qmin_save, sd->label.shortlabel);
      }
      if (set_valmax_save == 0) {
        SetSliceMax(set_valmax_save, qmax_save, sd->label.shortlabel);
      }
      if (set_valmin_save == 0 || set_valmax_save == 0) {
        float cbvals[256];

        for (i = 0; i < 256; i++) {
          cbvals[i] =
              (qmin_save * (float)(255 - i) + qmax_save * (float)i) / 255.0;
        }
        for (i = 0; i < nvsliceinfo; i++) {
          vslicedata *vslicei;
          slicedata *slicei;

          vslicei = vsliceinfo + i;
          if (vslicei->loaded == 0 || vslicei->display == 0 ||
              vslicei->ival == -1)
            continue;
          slicei = sliceinfo + vslicei->ival;
          if (slicei->loaded == 0 ||
              strcmp(sd->label.shortlabel, slicei->label.shortlabel) != 0)
            continue;
          slicei->valmin_slice = qmin_save;
          slicei->valmax_slice = qmax_save;
          memcpy(slicei->qval256, cbvals, 256 * sizeof(float));
          SetSliceColors(qmin_save, qmax_save, slicei, 0, errorcode);
        }
      }
    }
#endif
  }
  PushSliceLoadstack(&vslice_loadstack, ivslice);

  PrintMemoryInfo;
  if (finalize == 1) {
    updatemenu = 1;
    ForceIdle();
  }
  update_vectorskip = 1;
  return return_filesize;
}

/* ------------------ UncompressSliceDataFrame ------------------------ */

void UncompressSliceDataFrame(slicedata *sd, int iframe_local) {
  unsigned int countin;
  uLongf countout;
  unsigned char *compressed_data;

  compressed_data =
      sd->qslicedata_compressed + sd->compindex[iframe_local].offset;
  countin = sd->compindex[iframe_local].size;
  countout = sd->nsliceijk;

  if (sd->compression_type == COMPRESSED_ZLIB) {
    UnCompressZLIB(sd->slicecomplevel, &countout, compressed_data, countin);
  }
  else if (sd->compression_type == COMPRESSED_RLE) {
    countout = UnCompressRLE(compressed_data, countin, sd->slicecomplevel);
  }
  CheckMemory;
}

/* ------------------ MergeLoadedSliceHist ------------------------ */

void MergeLoadedSliceHist(char *label, histogramdata **histptr) {
  histogramdata *hist;
  int i;

  hist = *histptr;
  if (*histptr != NULL) FreeHistogram(*histptr);
  NewMemory((void **)&hist, sizeof(histogramdata));
  *histptr = hist;

  InitHistogram(hist, NHIST_BUCKETS, NULL, NULL);
  for (i = 0; i < nsliceinfo; i++) {
    slicedata *slicei;

    slicei = sliceinfo + i;
    if (slicei->loaded == 0 || strcmp(slicei->label.shortlabel, label) != 0)
      continue;
    MergeHistogram(hist, slicei->histograms, MERGE_BOUNDS);
  }
}

/* ------------------ GetSliceHists ------------------------ */

void GetSliceHists(slicedata *sd, int use_bounds, float valmin, float valmax) {
  int ndata;
  int n, i;
  int nframe;
  float *pdata0;

  int istep;
  int nx, ny, nxy, ibar, jbar;
  int ntimes;
  char *iblank_node, *iblank_cell, *slice_mask0;
  meshdata *meshi;
  float *slice_weight0;
  float *xplt, *yplt, *zplt;

  if (sd->histograms != NULL) return;
  meshi = meshinfo + sd->blocknumber;
  iblank_node = meshi->c_iblank_node;
  iblank_cell = meshi->c_iblank_cell;
  xplt = meshi->xplt_orig;
  yplt = meshi->yplt_orig;
  zplt = meshi->zplt_orig;

  ibar = meshi->ibar;
  jbar = meshi->jbar;
  nx = ibar + 1;
  ny = jbar + 1;
  nxy = nx * ny;

  ndata = sd->nslicetotal;

  nframe = sd->nslicei * sd->nslicej * sd->nslicek;
  NewMemory((void **)&slice_mask0, sd->nslicei * sd->nslicej * sd->nslicek);
  NewMemory((void **)&slice_weight0,
            sd->nslicei * sd->nslicej * sd->nslicek * sizeof(float));
  n = -1;
  for (i = 0; i < sd->nslicei; i++) {
    int j;
    float dx;
    int i1, i1p1;

    i1 = MIN(sd->is1 + i, sd->is2 - 2);
    i1p1 = i1 + 1;
    dx = xplt[i1p1] - xplt[i1];
    if (dx <= 0.0) dx = 1.0;

    for (j = 0; j < sd->nslicej; j++) {
      int k;
      float dy;
      int j1, j1p1;

      j1 = MIN(sd->js1 + j, sd->js2 - 2);
      j1p1 = j1 + 1;
      dy = yplt[j1p1] - yplt[j1];
      if (dy <= 0.0) dy = 1.0;

      for (k = 0; k < sd->nslicek; k++) {
        float dz;
        int k1, k1p1;

        k1 = MIN(sd->ks1 + k, sd->ks2 - 2);
        k1p1 = k1 + 1;
        dz = zplt[k1p1] - zplt[k1];
        if (dz <= 0.0) dz = 1.0;

        n++;
        slice_mask0[n] = 0;
        slice_weight0[n] = dx * dy * dz;
        if (sd->slice_filetype == SLICE_CELL_CENTER &&
            ((k == 0 && sd->nslicek != 1) || (j == 0 && sd->nslicej != 1) ||
             (i == 0 && sd->nslicei != 1)))
          continue;
        if (show_slice_in_obst == ONLY_IN_GAS) {
          if (sd->slice_filetype != SLICE_CELL_CENTER && iblank_node != NULL &&
              iblank_node[IJKNODE(sd->is1 + i, sd->js1 + j, sd->ks1 + k)] ==
                  SOLID)
            continue;
          if (sd->slice_filetype == SLICE_CELL_CENTER && iblank_cell != NULL &&
              iblank_cell[IJKCELL(sd->is1 + i - 1, sd->js1 + j - 1,
                                  sd->ks1 + k - 1)] == EMBED_YES)
            continue;
        }
        slice_mask0[n] = 1;
      }
    }
  }

  ntimes = ndata / sd->nsliceijk;

  // initialize histograms

  sd->nhistograms = ntimes + 1;
  NewMemory((void **)&sd->histograms, sd->nhistograms * sizeof(histogramdata));
  NewMemory((void **)&pdata0,
            sd->nslicei * sd->nslicej * sd->nslicek * sizeof(float));
  for (i = 0; i < sd->nhistograms; i++) {
    InitHistogram(sd->histograms + i, NHIST_BUCKETS, NULL, NULL);
  }

  n = 0;
  int skip;
  skip = MAX(ntimes / histogram_nframes, 1);
  for (istep = 0; istep < ntimes; istep += skip) {
    histogramdata *histi, *histall;
    int nn;

    if (sd->compression_type != UNCOMPRESSED) {
      UncompressSliceDataFrame(sd, istep);
    }
    for (nn = 0; nn < sd->nslicei * sd->nslicej * sd->nslicek; nn++) {
      if (sd->compression_type != UNCOMPRESSED) {
        pdata0[nn] = sd->qval256[sd->slicecomplevel[nn]];
      }
      else {
        pdata0[nn] = sd->qslicedata[n + nn];
      }
    }
    n += skip * sd->nslicei * sd->nslicej * sd->nslicek;

    // compute histogram for each timestep, histi and all time steps, histall

    histi = sd->histograms + istep + 1;
    histall = sd->histograms;
    CopyVals2Histogram(pdata0, slice_mask0, slice_weight0, nframe, histi,
                       use_bounds, valmin, valmax);
    MergeHistogram(histall, histi, MERGE_BOUNDS);
  }
  FREEMEMORY(pdata0);
  FREEMEMORY(slice_mask0);
  FREEMEMORY(slice_weight0);
}

/* ------------------ GetHistogramValProc ------------------------ */

void GetHistogramValProc(histogramdata *histogram, float cdf, float *val) {
  *val = GetHistogramVal(histogram, cdf);
}

/* ------------------ GetSliceGeomHists ------------------------ */

void GetSliceGeomHists(slicedata *sd, int use_bounds, float valmin,
                       float valmax) {
  if (sd->histograms != NULL) return;

  // initialize histograms

  sd->nhistograms = 1;
  NewMemory((void **)&sd->histograms, sizeof(histogramdata));
  InitHistogram(sd->histograms, NHIST_BUCKETS, NULL, NULL);

  // compute histogram for each timestep, histi and all time steps, histall

  CopyVals2Histogram(sd->patchgeom->geom_vals, NULL, NULL,
                     sd->patchgeom->geom_nvals, sd->histograms, use_bounds,
                     valmin, valmax);
}

/* ------------------ ComputeLoadedSliceHist ------------------------ */

void ComputeLoadedSliceHist(char *label, float valmin, float valmax) {
  int i;

  for (i = 0; i < nsliceinfo; i++) {
    slicedata *slicei;

    slicei = sliceinfo + i;
    if (slicei->loaded == 0) continue;
    if (label != NULL && strcmp(slicei->label.shortlabel, label) != 0) continue;
    if (slicei->histograms == NULL) {
      if (slicei->slice_filetype == SLICE_GEOM) {
        int use_bounds = 1;
        GetSliceGeomHists(slicei, use_bounds, valmin, valmax);
      }
      else {
        int use_bounds = 1;
        GetSliceHists(slicei, use_bounds, valmin, valmax);
      }
    }
  }
}

/* ------------------ SetSliceLabels ------------------------ */

void SetSliceLabels(float smin, float smax, slicedata *sd, patchdata *pd,
                    int *errorcode) {
  int slicetype = -1;

  assert((sd != NULL && pd == NULL) || (sd == NULL && pd != NULL));
  if (sd != NULL)
    slicetype = GetSliceBoundsIndexFromLabel(sd->label.shortlabel);
  if (pd != NULL)
    slicetype = GetSliceBoundsIndexFromLabel(pd->label.shortlabel);
  if (slicetype != -1) {
    boundsdata *sb;

    sb = slicebounds + slicetype;
    if (sd != NULL) sb->label = &(sd->label);
    if (pd != NULL) sb->label = &(pd->label);

    *errorcode = 0;
    GetColorbarLabels(smin, smax, nrgb, sb->colorlabels, sb->levels256);
  }
}

/* ------------------ SliceCompare ------------------------ */

int SliceCompare(const void *arg1, const void *arg2) {
  slicedata *slicei, *slicej;

  slicei = sliceinfo + *(int *)arg1;
  slicej = sliceinfo + *(int *)arg2;

  if (strcmp(slicei->label.longlabel, slicej->label.longlabel) < 0) return -1;
  if (strcmp(slicei->label.longlabel, slicej->label.longlabel) > 0) return 1;
  if (slicei->volslice == 1 && slicej->volslice == 0) return -1;
  if (slicei->volslice == 0 && slicej->volslice == 1) return 1;
  if (slicei->idir < slicej->idir) return -1;
  if (slicei->idir > slicej->idir) return 1;
  if (slicei->position_orig < slicej->position_orig) return -1;
  if (slicei->position_orig > slicej->position_orig) return 1;
  if (slicei->slice_filetype < slicej->slice_filetype) return -1;
  if (slicei->slice_filetype > slicej->slice_filetype) return 1;
  if (slicei->slcf_index < slicej->slcf_index) return -1;
  if (slicei->slcf_index > slicej->slcf_index) return 1;
  return 0;
}

/* ------------------ VSliceCompare ------------------------ */

int VSliceCompare(const void *arg1, const void *arg2) {
  vslicedata *vslicei, *vslicej;

  vslicei = vsliceinfo + *(int *)arg1;
  vslicej = vsliceinfo + *(int *)arg2;
  return SliceCompare(&(vslicei->ival), &(vslicej->ival));
}

#ifndef READSMOKE_H_DEFINED
#define READSMOKE_H_DEFINED
#include "options.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MALLOCC.h"
#include "contourdefs.h"
#include "histogram.h"
#include "isodefs.h"
#include "shared_structures.h"
#include "smokeviewdefs.h"
#include "string_util.h"

#include "readobject.h"

/* --------------------------  smokedata ------------------------------------ */

typedef struct _smokedata {
  int ncomp_total;
  int *nchars_compressed, *nchars_compressed_full;
  unsigned char *frame_in, *frame_out, *view_tmp, *comp_all, **frame_comp_list;
} smokedata;

/* --------------------------  smokestatedata
 * ------------------------------------ */

typedef struct {
  int loaded, index;
  unsigned char *color;
} smokestatedata;

/* --------------------------  smoke3ddata ------------------------------------
 */

typedef struct _smoke3ddata {
  int seq_id, autoload;
  char *file;
  char *comp_file, *reg_file;
#ifdef pp_SMOKE16
  char *s16_file;
#endif
  int filetype;
  int skip_smoke, skip_fire;
  int is_smoke, is_fire;
  int loaded, request_load, finalize, display, primary_file;
  int is_zlib;
#ifdef pp_SMOKE16
  int is_s16;
#endif
  smokestatedata *smokestate;
  int blocknumber;
  int type;
  int is1, is2, js1, js2, ks1, ks2;
  int compression_type, compression_type_temp;
  flowlabels label;
  char menulabel[128];
  float *times;
  unsigned char *times_map;
  int *use_smokeframe;
  int *smokeframe_loaded;
  float extinct, valmin, valmax;
  char cextinct[32];
#define ALPHA_X 0
#define ALPHA_Y 1
#define ALPHA_Z 2
#define ALPHA_XY 3
#define ALPHA_YZ 4
#define ALPHA_XZ 5
  unsigned char *alphas_dir[6];
  int fire_alpha, co2_alpha;
  float fire_alphas[256], co2_alphas[256];
  int *timeslist;
  int ntimes, ntimes_old, ismoke3d_time, lastiframe, ntimes_full;
  int nchars_uncompressed;

  int ncomp_smoke_total;
  int *nchars_compressed_smoke, *nchars_compressed_smoke_full;
#ifdef pp_SMOKE16
  unsigned short *val16s;
  float *val16_mins, *val16_maxs, *times16;
#endif
  float maxval;
  unsigned char *smokeframe_in, *smokeframe_out, **smokeframe_comp_list;
  unsigned char *smokeview_tmp;
#ifndef pp_SMOKEFRAME
  unsigned char *smoke_comp_all;
#endif
  unsigned char *frame_all_zeros;
  FILE_SIZE file_size;
  float *smoke_boxmin, *smoke_boxmax;
  smokedata smoke;
  int dir;
#ifdef pp_SMOKEFRAME
  framedata *frameinfo;
#endif
} smoke3ddata;

/* --------------------------  smoke3dtypedata
 * ------------------------------------ */

typedef struct _smoke3dtypedata {
  char *shortlabel, *longlabel;
  int type; // color based or opacity based
  int menu_id;
  smoke3ddata *smoke3d;
  float extinction, valmin, valmax;
} smoke3dtypedata;

typedef struct {
  int nsmoke3dinfo;
  smoke3ddata *smoke3dinfo;
  smoke3ddata **smoke3dinfo_sorted;
} smoke3d_collection;

void FreeSmoke3D(smoke3ddata *smoke3di);
#endif

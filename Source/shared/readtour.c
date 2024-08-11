#include "options.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #if defined(WIN32)
// #include <windows.h>
// #endif
// #include GLU_H

// #include "datadefs.h"
// #include "smokeviewvars.h"
// #include "IOobjects.h"
// #include "getdata.h"
#include "MALLOCC.h"
#include "contourdefs.h"
#include "histogram.h"
#include "isodefs.h"
#include "smokeviewdefs.h"
#include "string_util.h"
#include "structures.h"

#include "readobject.h"
#include "readpart.h"

void ReallocTourMemory(int ntourinfo, tourdata *tourinfo, int tour_ntimes,
                       float *tour_t, float *tour_t2, float *tour_dist,
                       float *tour_dist2, float *tour_dist3) {
  int i;
  tourdata *touri;

  if (tour_ntimes > 0) {
    for (i = 0; i < ntourinfo; i++) {
      touri = tourinfo + i;
      FREEMEMORY(touri->path_times);
      NewMemory((void **)&touri->path_times, tour_ntimes * sizeof(float));
      touri->ntimes = tour_ntimes;
    }
    FREEMEMORY(tour_t);
    FREEMEMORY(tour_t2);
    FREEMEMORY(tour_dist);
    FREEMEMORY(tour_dist2);
    FREEMEMORY(tour_dist3);
    NewMemory((void **)&tour_t, tour_ntimes * sizeof(float));
    NewMemory((void **)&tour_t2, tour_ntimes * sizeof(float));
    NewMemory((void **)&tour_dist, tour_ntimes * sizeof(float));
    NewMemory((void **)&tour_dist2, tour_ntimes * sizeof(float));
    NewMemory((void **)&tour_dist3, (tour_ntimes + 10) * sizeof(float));
  }
}

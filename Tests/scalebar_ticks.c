#include "options.h"

#include "getdata.h"

#include "dmalloc.h"

#include "string_util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readlabel.h"

int MakeUniformScalebarTicks(scalebarticks *sbt, float start, float end,
                             int n_ticks, int ndecimals, int fixed_point);
int MakeNonUniformScalebarTicks(scalebarticks *sbt, float *ticks, int n_ticks,
                                int ndecimals, int fixed_point);
void FreeScalebarTicks(scalebarticks *sbt);

int main(int argc, char **argv) {
  initMALLOC();
  SetStdOut(stdout);
  {
    scalebarticks sbt = {0};
    assert(!MakeUniformScalebarTicks(&sbt, 5, 10, 3, 3, 0));
    assert(sbt.n_ticks == 3);
    for(int i = 0; i < sbt.n_ticks; i++) {
      fprintf(stderr, "[%d]: %f: %s\n", i, sbt.ticks[i], sbt.tick_labels[i]);
    }
    FreeScalebarTicks(&sbt);
  }
  {
    scalebarticks sbt = {0};
    float ticks[3] = {5.0, 9.0, 10.0};
    assert(!MakeNonUniformScalebarTicks(&sbt, ticks, 3, 3, 0));
    assert(sbt.n_ticks == 3);
    for(int i = 0; i < sbt.n_ticks; i++) {
      fprintf(stderr, "[%d]: %f: %s\n", i, sbt.ticks[i], sbt.tick_labels[i]);
    }
    FreeScalebarTicks(&sbt);
  }
  {
    scalebarticks sbt = {0};
    // Monotonic decreasing is ok.
    float ticks[3] = {10.0, 9.0, 5.0};
    assert(!MakeNonUniformScalebarTicks(&sbt, ticks, 3, 3, 0));
    assert(sbt.n_ticks == 3);
    for(int i = 0; i < sbt.n_ticks; i++) {
      fprintf(stderr, "[%d]: %f: %s\n", i, sbt.ticks[i], sbt.tick_labels[i]);
    }
    FreeScalebarTicks(&sbt);
  }
  {
    scalebarticks sbt = {0};
    float ticks[3] = {5.0, 5.0, 10.0};
    int ret = MakeNonUniformScalebarTicks(&sbt, ticks, 3, 3, 0);
    // We've specified ticks where two values are the same, this should be
    // invalid. The return code should be 2.
    assert(ret == 2);
    // No ticks should be assigned and entries should be null
    assert(sbt.n_ticks == 0);
    assert(sbt.ticks == NULL);
    assert(sbt.tick_labels == NULL);
    FreeScalebarTicks(&sbt);
  }
  {
    scalebarticks sbt = {0};
    float ticks[3] = {5.0, 4.0, 10.0};
    int ret = MakeNonUniformScalebarTicks(&sbt, ticks, 3, 3, 0);
    // We've specified ticks which are not monotonic, this should be
    // invalid. The return code should be 2.
    assert(ret == 2);
    // No ticks should be assigned and entries should be null
    assert(sbt.n_ticks == 0);
    assert(sbt.ticks == NULL);
    assert(sbt.tick_labels == NULL);
    FreeScalebarTicks(&sbt);
  }
  return 0;
}

#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include "smokeviewdefs.h"
#include "string_util.h"
#include "isodefs.h"
#include "structures.h"

#include "readobject.h"
#include "readpart.h"

void UpdatePartClassDepend(partclassdata *partclassi){
  int i;

  if(partclassi->prop!=NULL){
    sv_object_frame *obj_frame;
    int nvar;

    obj_frame=partclassi->prop->smv_object->obj_frames[0];
    for(i=0;i<partclassi->nvars_dep-3;i++){
      char *var;

      var=partclassi->vars_dep[i];
      partclassi->vars_dep_index[i]= GetObjectFrameTokenLoc(var,obj_frame);
    }
    nvar = partclassi->nvars_dep;
    partclassi->vars_dep_index[nvar-3]= GetObjectFrameTokenLoc("R",obj_frame);
    partclassi->vars_dep_index[nvar-2]= GetObjectFrameTokenLoc("G",obj_frame);
    partclassi->vars_dep_index[nvar-1]= GetObjectFrameTokenLoc("B",obj_frame);
  }
}

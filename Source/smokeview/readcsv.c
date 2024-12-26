
#include "options.h"
#include "glew.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "stdio_buffer.h"
#include "glui_bounds.h"
#include "shared_structures.h"
#include "readimage.h"
#include "readhvac.h"
#include "readgeom.h"
#include "readslice.h"
#include "readobject.h"
#include "readlabel.h"
#include "readsmoke.h"
#include "IOobjects.h"
#include "IOscript.h"

/* ------------------ ReadAllCSVFiles ------------------------ */

FILE_SIZE ReadAllCSVFiles(smv_case *scase, int flag){
  int i;
  FILE_SIZE file_size=0;

  if(scase->csvcoll.ncsvfileinfo == 0)return 0;
#define GENPLOT_REM_ALL_PLOTS       136
  GLUIGenPlotCB(GENPLOT_REM_ALL_PLOTS);
  for(i = 0; i < scase->csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvfi;

    csvfi = scase->csvcoll.csvfileinfo + i;
    ReadCSVFile(csvfi, UNLOAD);
  }
  if(flag == UNLOAD)return 0;
  for(i = 0; i < scase->csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvfi;

    csvfi = scase->csvcoll.csvfileinfo + i;
    if(csvfi->defined == CSV_DEFINING|| csvfi->defined == CSV_DEFINED){
      continue;
    }
    csvfi->defined = CSV_DEFINING;
    file_size += ReadCSVFile(csvfi, flag);
    plot2d_max_columns = MAX(plot2d_max_columns, csvfi->ncsvinfo);
    csvfi->defined = CSV_DEFINED;
    UpdateCSVFileTypes(scase);
  }
  for(i = 0; i < scase->csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvfi;

    csvfi = scase->csvcoll.csvfileinfo + i;
    if(csvfi->defined != CSV_DEFINED){
      break;
    }
  }
  return file_size;
}


/* ------------------ Compress ------------------------ */

void *Compress(void *arg){
  char shellcommand[1024];
  smv_case *scase = (smv_case *)arg;

  PRINTF("Compressing...\n");
  GLUICompressOnOff(OFF);

  WriteIni(scase, LOCAL_INI, NULL);

  // surround smokezip path name with "'s so that the system call can handle embedded blanks

  strcpy(shellcommand, "\"");
  strcat(shellcommand, smokezippath);
  strcat(shellcommand, "\" ");
  if(overwrite_all == 1){
    strcat(shellcommand, " -f ");
  }
  if(erase_all == 1){
    strcat(shellcommand, " -c ");
  }
  if(compress_autoloaded == 1){
    strcat(shellcommand, " -auto ");
  }
  strcat(shellcommand, " ");
  strcat(shellcommand, smv_filename);

  PRINTF("Executing shell command: %s\n", shellcommand);
  system(shellcommand);
  UpdateSmoke3dMenuLabels();
  UpdateBoundaryMenuLabels();
  GLUICompressOnOff(ON);
  updatemenu = 1;
  PRINTF("Compression completed\n");
  THREAD_EXIT(compress_threads);
}

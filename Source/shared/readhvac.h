#ifndef READHVAC_H_DEFINED
#define READHVAC_H_DEFINED
#include "options.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// TODO: we'd need pointers if we need to share with global.
typedef struct {
  hvacnodedata *hvacnodeinfo;
  int nhvacnodeinfo;
  hvacductdata *hvacductinfo;
  int nhvacductinfo;
  hvacconnectdata *hvacconnectinfo;
  int nhvacconnectinfo;
  hvacdata *hvacinfo;
  int nhvacinfo;

  hvacvalsdata *hvacductvalsinfo;
  int hvacductvar_index;
  hvacvalsdata *hvacnodevalsinfo;
  int hvacnodevar_index;

  int hvac_maxcells;
  int hvac_n_ducts;

} hvacdatacollection;

int hvacval(hvacdatacollection *hvaccoll, int itime, int iduct, int icell);

hvacductdata *GetHVACDuctID(hvacdatacollection *hvaccoll, char *duct_name);
int GetHVACDuctValIndex(hvacdatacollection *hvaccoll, char *shortlabel);
int GetHVACNodeValIndex(hvacdatacollection *hvaccoll, char *shortlabel);
hvacnodedata *GetHVACNodeID(hvacdatacollection *hvaccoll, char *node_name);

void InitHvacData(hvacvaldata *hi);
int CompareHvacConnect(const void *arg1, const void *arg2);
int IsHVACVisible(hvacdatacollection *hvaccoll);
int HaveHVACConnect(int val, hvacconnectdata *vals, int nvals);
void GetHVACPathXYZ(float fraction, float *xyzs, int n, float *xyz);
void GetCellXYZs(float *xyz, int nxyz, int ncells, float **xyz_cellptr,
                 int *nxyz_cell, int **cell_indptr);
void SetDuctLabelSymbolXYZ(hvacductdata *ducti);
void SetHVACInfo(hvacdatacollection *hvaccoll);
void ReadHVACData0(hvacdatacollection *hvaccoll, int flag,
                   FILE_SIZE *file_size);
#endif

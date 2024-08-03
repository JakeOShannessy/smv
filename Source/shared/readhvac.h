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

  int nhvacfilters;

  int nhvaccomponents;

  hvacvalsdata *hvacductvalsinfo;
  int hvacductvar_index;
  hvacvalsdata *hvacnodevalsinfo;
  int hvacnodevar_index;

  int hvac_maxcells;
  int hvac_n_ducts;

} hvacdatacollection;

hvacductdata *GetHVACDuctID(hvacdatacollection *hvaccoll, char *duct_name);
int GetHVACDuctValIndex(hvacdatacollection *hvaccoll, char *shortlabel);

hvacnodedata *GetHVACNodeID(hvacdatacollection *hvaccoll, char *node_name);
int GetHVACNodeValIndex(hvacdatacollection *hvaccoll, char *shortlabel);

void InitHvacData(hvacvaldata *hi);
/**
 * @brief Are any of the hvac items visible (i.e., have display set to true)?
 *
 * @param hvaccoll The HVAC collection.
 * @return 1 if ANY HVAC item is visible. 0 if NO HVAC item is visible.
 */
int IsHVACVisible(hvacdatacollection *hvaccoll);

/**
 * @brief Parse the definition of HVAC nodes etc. from an *.smv file.
 *
 * @param hvaccoll The HVAC collection.
 * @param stream The stream that is currently being parsed. The position of this
 * stream should be just after the "HVAC" keyword.
 * @param hvac_node_color Default HVAC node color
 * @param hvac_duct_color Default HVAC duct color
 * @return 0 on success, 1 to break parsing loop, 2 to continue to continue
 * parsing loop.
 */
int ParseHVACEntry(hvacdatacollection *hvaccoll, bufferstreamdata *stream,
                   int hvac_node_color[3], int hvac_duct_color[3]);
#endif

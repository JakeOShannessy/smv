#ifndef READLABEL_H_DEFINED
#define READLABEL_H_DEFINED

#include "options_common.h"
#include "shared_structures.h"

#ifndef TYPE_SMV
#define TYPE_SMV 0
#endif
#ifndef TYPE_INI
#define TYPE_INI 1
#endif

EXTERNCPP labeldata *LabelGet(labels_collection *labelscoll, char *name);
EXTERNCPP labeldata *LabelInsert(labels_collection *labelscoll,
                                 labeldata *labeltemp);

EXTERNCPP int LabelGetNUserLabels(labels_collection *labelscoll);
EXTERNCPP labeldata *LabelPrevious(labels_collection *labelscoll,
                                   labeldata *label);
EXTERNCPP labeldata *LabelNext(labels_collection *labelscoll, labeldata *label);
EXTERNCPP void LabelCopy(labeldata *label_to, labeldata *label_from);
EXTERNCPP void LabelDelete(labeldata *label);
labels_collection *CreateLabelsCollection();
EXTERNCPP void InitLabelsCollection(labels_collection *labelscoll);
EXTERNCPP void FreeLabelsCollection(labels_collection *labelscoll);
EXTERNCPP void LabelPrint(labels_collection *labelscoll);
EXTERNCPP float *GetColorPtr(smv_case *scase, float *color);
static inline int FileExistsCaseDir(smv_case *scase, char *filename) {
  return FileExists(filename, scase->filelist_coll.filelist_casename,
                    scase->filelist_coll.nfilelist_casename,
                    scase->filelist_coll.filelist_casedir,
                    scase->filelist_coll.nfilelist_casedir);
}
// extern CCC float foregroundcolor[4];
// extern CCC float *block_ambient2;
// extern CCC float *ventcolor;
// extern CCC GLfloat block_shininess;
// extern CCC float *mat_ambient2;

extern CCC parse_options parse_opts;
#endif

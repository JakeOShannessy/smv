#ifndef READLABEL_H_DEFINED
#define READLABEL_H_DEFINED

EXTERNCPP labeldata *LabelGet(labels_collection *labelscoll, char *name);
EXTERNCPP labeldata *LabelInsert(labels_collection *labelscoll,
                                 labeldata *labeltemp);

EXTERNCPP int LabelGetNUserLabels(labels_collection *labelscoll);
EXTERNCPP labeldata *LabelPrevious(labels_collection *labelscoll,
                                   labeldata *label);
EXTERNCPP labeldata *LabelNext(labels_collection *labelscoll, labeldata *label);
EXTERNCPP void LabelCopy(labeldata *label_to, labeldata *label_from);
EXTERNCPP void LabelDelete(labeldata *label);
#endif

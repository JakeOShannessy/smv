#ifndef READLABEL_H_DEFINED
#define READLABEL_H_DEFINED

EXTERNCPP labeldata *LabelGet(labeldata *label_first_ptr, char *name);

void LabelInsertBefore(labeldata *listlabel, labeldata *label);
void LabelInsertAfter(labeldata *listlabel, labeldata *label);

EXTERNCPP labeldata *LabelInsert(labeldata label_first, labeldata label_last,
                       labeldata *label_first_ptr, labeldata *label_last_ptr,
                       labeldata *labeltemp);
#endif

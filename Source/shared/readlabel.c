#include "options.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmalloc.h"
#include "histogram.h"
#include "isobox.h"
#include "readlabel.h"
#include "shared_structures.h"
#include "string_util.h"
#include "datadefs.h"

/* ------------------ LabelGet ------------------------ */

labeldata *LabelGet(labels_collection *labelscoll, char *name) {
  labeldata *thislabel;

  if(name == NULL) return NULL;
  for(thislabel = labelscoll->label_first_ptr->next; thislabel->next != NULL;
      thislabel = thislabel->next) {
    if(strcmp(thislabel->name, name) == 0) return thislabel;
  }
  return NULL;
}

/* ------------------ LabelInsertBefore ------------------------ */

void LabelInsertBefore(labeldata *listlabel, labeldata *label) {
  labeldata *prev, *next;

  prev = listlabel->prev;
  next = listlabel;
  prev->next = label;
  next->prev = label;
  label->prev = prev;
  label->next = next;
}

/* ------------------ LabelInsertAfter ------------------------ */

void LabelInsertAfter(labeldata *listlabel, labeldata *label) {
  labeldata *prev, *next;

  prev = listlabel;
  next = listlabel->next;
  prev->next = label;
  if(next!=NULL)next->prev = label;
  label->prev = prev;
  label->next = next;
}

/* ------------------ LabelInsert ------------------------ */

labeldata *LabelInsert(labels_collection *labelscoll, labeldata *labeltemp) {
  labeldata *newlabel, *thislabel;
  labeldata *firstuserptr, *lastuserptr;

  NewMemory((void **)&newlabel, sizeof(labeldata));
  memcpy(newlabel, labeltemp, sizeof(labeldata));

  thislabel = LabelGet(labelscoll, newlabel->name);
  if(thislabel != NULL) {
    LabelInsertAfter(thislabel->prev, newlabel);
    return newlabel;
  }

  firstuserptr = labelscoll->label_first_ptr->next;
  if(firstuserptr == labelscoll->label_last_ptr) firstuserptr = NULL;

  lastuserptr = labelscoll->label_last_ptr->prev;
  if(lastuserptr == labelscoll->label_first_ptr) lastuserptr = NULL;

  if(firstuserptr != NULL && strcmp(newlabel->name, firstuserptr->name) < 0) {
    LabelInsertBefore(firstuserptr, newlabel);
    return newlabel;
  }
  if(lastuserptr != NULL && strcmp(newlabel->name, lastuserptr->name) > 0) {
    LabelInsertAfter(lastuserptr, newlabel);
    return newlabel;
  }
  if(firstuserptr == NULL && lastuserptr == NULL) {
    LabelInsertAfter(labelscoll->label_first_ptr, newlabel);
    return newlabel;
  }
  for(thislabel = labelscoll->label_first_ptr->next; thislabel!=NULL&&thislabel->next != NULL;
      thislabel = thislabel->next) {
    labeldata *nextlabel;

    nextlabel = thislabel->next;
    if(strcmp(thislabel->name, newlabel->name) < 0 &&
       strcmp(newlabel->name, nextlabel->name) < 0) {
      LabelInsertAfter(thislabel, newlabel);
      return newlabel;
    }
  }
  return NULL;
}

/* ------------------ LabelDelete ------------------------ */

void LabelDelete(labeldata *label) {
  labeldata *prev, *next;

  prev = label->prev;
  next = label->next;
  CheckMemory;
  FREEMEMORY(label);
  prev->next = next;
  next->prev = prev;
}

/* ------------------ LabelCopy ------------------------ */

void LabelCopy(labeldata *label_to, labeldata *label_from) {
  labeldata *prev, *next;

  prev = label_to->prev;
  next = label_to->next;
  memcpy(label_to, label_from, sizeof(labeldata));
  label_to->prev = prev;
  label_to->next = next;
}

/* ------------------ LabelResort ------------------------ */

void LabelResort(labels_collection *labelscoll, labeldata *label) {
  labeldata labelcopy;

  CheckMemory;
  memcpy(&labelcopy, label, sizeof(labeldata));
  CheckMemory;
  LabelDelete(label);
  LabelInsert(labelscoll, &labelcopy);
}

/* ------------------ LabelPrint ------------------------ */

void LabelPrint(labels_collection *labelscoll) {
  labeldata *thislabel;
  float *xyz;

  for(thislabel = labelscoll->label_first_ptr->next; thislabel->next != NULL;
      thislabel = thislabel->next) {
    xyz = thislabel->xyz;
    PRINTF("label: %s position: %f %f %f\n", thislabel->name, xyz[0], xyz[1],
           xyz[2]);
  }
}

/* ------------------ LabelNext ------------------------ */

labeldata *LabelNext(labels_collection *labelscoll, labeldata *label) {
  labeldata *thislabel;

  if(label == NULL) return NULL;
  if(labelscoll->label_first_ptr->next->next == NULL) return NULL;
  for(thislabel = label->next; thislabel != label;
      thislabel = thislabel->next) {
    if(thislabel->next == NULL) thislabel = labelscoll->label_first_ptr->next;
    if(thislabel->labeltype == TYPE_SMV) continue;
    return thislabel;
  }
  return NULL;
}

/* ------------------ LabelPrevious ------------------------ */

labeldata *LabelPrevious(labels_collection *labelscoll, labeldata *label) {
  labeldata *thislabel;

  if(label == NULL) return NULL;
  if(labelscoll->label_last_ptr->prev->prev == NULL) return NULL;
  for(thislabel = label->prev; thislabel != label;
      thislabel = thislabel->prev) {
    if(thislabel->prev == NULL) thislabel = labelscoll->label_last_ptr->prev;
    if(thislabel->labeltype == TYPE_SMV) continue;
    return thislabel;
  }
  return NULL;
}

/* ------------------ LabelInit ------------------------ */

int LabelInit(labels_collection *labelscoll, labeldata *gl) {
  labeldata *thislabel;

  for(thislabel = labelscoll->label_first_ptr->next; thislabel->next != NULL;
      thislabel = thislabel->next) {
    if(thislabel->labeltype == TYPE_SMV) continue;
    LabelCopy(gl, thislabel);
    return 1;
  }
  return 0;
}

/* ------------------ LabelGetNUserLabels ------------------------ */

int LabelGetNUserLabels(labels_collection *labelscoll) {
  int count = 0;
  labeldata *thislabel;

  for(thislabel = labelscoll->label_first_ptr->next; thislabel->next != NULL;
      thislabel = thislabel->next) {
    if(thislabel->labeltype == TYPE_INI) count++;
  }
  return count;
}

/* ------------------ InitLabelsCollection ------------------------ */

void InitLabelsCollection(labels_collection *labelscoll) {
  labelscoll->label_first_ptr = &labelscoll->label_first;
  labelscoll->label_last_ptr = &labelscoll->label_last;

  labelscoll->label_first_ptr->prev = NULL;
  labelscoll->label_first_ptr->next = labelscoll->label_last_ptr;
  strcpy(labelscoll->label_first_ptr->name, "first");

  labelscoll->label_last_ptr->prev = labelscoll->label_first_ptr;
  labelscoll->label_last_ptr->next = NULL;
  strcpy(labelscoll->label_last_ptr->name, "last");
}

/* ------------------ FreeLabelsCollection ------------------------ */

labels_collection *CreateLabelsCollection() {
  labels_collection *labelscoll;
  NEWMEMORY(labelscoll, sizeof(labels_collection));
  InitLabelsCollection(labelscoll);
  return labelscoll;
}

/* ------------------ FreeLabelsCollection ------------------------ */

void FreeLabelsCollection(labels_collection *labelscoll) {
  labeldata *thislabel = labelscoll->label_first_ptr->next;
  while(thislabel->next != NULL) {
    labeldata *nextlabel = thislabel->next;
    LabelDelete(thislabel);
    thislabel = nextlabel;
  }
}

/* ------------------ GetColorPtr ------------------------ */

float *GetColorPtr(smv_case *scase, float *color){
  colordata *colorptr,*oldlastcolor,*lastcolor;

  int i;

  if(scase->firstcolor==NULL){
    NewMemory((void *)&scase->firstcolor,sizeof(colordata));
    for(i=0;i<4;i++){
      scase->firstcolor->color[i]=color[i];
      scase->firstcolor->full_color[i]=color[i];
    }
    scase->firstcolor->bw_color[0] = TOBW(color);
    scase->firstcolor->bw_color[1] = scase->firstcolor->bw_color[0];
    scase->firstcolor->bw_color[2] = scase->firstcolor->bw_color[0];
    scase->firstcolor->bw_color[3] = color[3];
    scase->firstcolor->nextcolor=NULL;
    return scase->firstcolor->color;
  }
  oldlastcolor = scase->firstcolor;
  for(colorptr = scase->firstcolor; colorptr!=NULL; colorptr = colorptr->nextcolor){
    oldlastcolor=colorptr;
    if(ABS(colorptr->color[0]-color[0])>0.0001)continue;
    if(ABS(colorptr->color[1]-color[1])>0.0001)continue;
    if(ABS(colorptr->color[2]-color[2])>0.0001)continue;
    if(ABS(colorptr->color[3]-color[3])>0.0001)continue;
    return colorptr->color;
  }
  lastcolor=NULL;
  NewMemory((void *)&lastcolor,sizeof(colordata));
  oldlastcolor->nextcolor=lastcolor;
  for(i=0;i<4;i++){
    lastcolor->color[i]=color[i];
    lastcolor->full_color[i]=color[i];
  }
  lastcolor->bw_color[0] = TOBW(color);
  lastcolor->bw_color[1] = lastcolor->bw_color[0];
  lastcolor->bw_color[2] = lastcolor->bw_color[0];
  lastcolor->bw_color[3] = color[3];
  lastcolor->nextcolor=NULL;
  return lastcolor->color;
}

float foregroundcolor[4] = {1.0, 1.0, 1.0, 1.0};
float *block_ambient2 = NULL;
float *ventcolor = NULL;
GLfloat block_shininess = 100.0;
float *mat_ambient2 = NULL;
int setbw = 0;

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

/* ------------------ LabelGet ------------------------ */

labeldata *LabelGet(labeldata *label_first_ptr, char *name){
  labeldata *thislabel;

  if(name==NULL)return NULL;
  for(thislabel=label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
    if(strcmp(thislabel->name,name)==0)return thislabel;
  }
  return NULL;
}


void LabelInsertBefore(labeldata *listlabel, labeldata *label){
  labeldata *prev, *next;

  prev        = listlabel->prev;
  next        = listlabel;
  prev->next  = label;
  next->prev  = label;
  label->prev = prev;
  label->next = next;
}


void LabelInsertAfter(labeldata *listlabel, labeldata *label){
  labeldata *prev, *next;

  prev        = listlabel;
  next        = listlabel->next;
  prev->next  = label;
  next->prev  = label;
  label->prev = prev;
  label->next = next;
}


labeldata *LabelInsert(labeldata label_first, labeldata label_last,
                       labeldata *label_first_ptr, labeldata *label_last_ptr,
                       labeldata *labeltemp) {
  labeldata *newlabel, *thislabel;
  labeldata *firstuserptr, *lastuserptr;

  NewMemory((void **)&newlabel,sizeof(labeldata));
  memcpy(newlabel,labeltemp,sizeof(labeldata));

  thislabel = LabelGet(label_first_ptr,newlabel->name);
  if(thislabel!=NULL){
    LabelInsertAfter(thislabel->prev,newlabel);
    return newlabel;
  }

  firstuserptr=label_first_ptr->next;
  if(firstuserptr==label_last_ptr)firstuserptr=NULL;

  lastuserptr=label_last_ptr->prev;
  if(lastuserptr==label_first_ptr)lastuserptr=NULL;

  if(firstuserptr!=NULL&&strcmp(newlabel->name,firstuserptr->name)<0){
    LabelInsertBefore(firstuserptr,newlabel);
    return newlabel;
  }
  if(lastuserptr!=NULL&&strcmp(newlabel->name,lastuserptr->name)>0){
    LabelInsertAfter(lastuserptr,newlabel);
    return newlabel;
  }
  if(firstuserptr==NULL&&lastuserptr==NULL){
    LabelInsertAfter(label_first_ptr,newlabel);
    return newlabel;
  }
  for(thislabel=label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
    labeldata *nextlabel;

    nextlabel=thislabel->next;
    if(strcmp(thislabel->name,newlabel->name)<0&&strcmp(newlabel->name,nextlabel->name)<0){
      LabelInsertAfter(thislabel,newlabel);
      return newlabel;
    }
  }
  return NULL;
}

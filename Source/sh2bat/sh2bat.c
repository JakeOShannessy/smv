#define INMAIN
// convert the Linux/OSX script containing a list FDS cases
// to an equivalent Windows bat version
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_util.h"
#include "file_util.h"
#include "MALLOC.h"

/* ------------------ Usage ------------------------ */

void Usage(char *prog, int option){
 char githash[256];
 char gitdate[256];

  GetGitInfo(githash,gitdate);    // get githash

  fprintf(stderr, "\n%s (%s) %s\n", prog, githash, __DATE__);
  fprintf(stderr, "convert a bash script to a windows batch file\n\n");
  fprintf(stderr, "Usage: %s file_in file_out\n\n",prog);
  fprintf(stderr, " convert the Linux/OSX script file file_in to an equivalent windows batch\n");
  fprintf(stderr, " file file_out by ignoring lines beginning with # and converting variables\n");
  fprintf(stderr, " such as $var to %svar%s\n", "%", "%");
  UsageCommon(prog,HELP_SUMMARY);
  if(option == HELP_ALL)UsageCommon(HELP_ALL);
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  char buffer[1024];
  int i;
  char *filein=NULL,*fileout=NULL,*prog;
  FILE *streamin=NULL,*streamout=NULL;

  SetStdOut(stdout);
  initMALLOC();
  prog=argv[0];

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage("sh2bat",show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("sh2bat", argv[0]);
    return 1;
  }

  for(i=1;i<argc;i++){
    int lenarg;
    char *arg;

    arg=argv[i];
    lenarg=strlen(arg);
    if(arg[0]=='-'&&lenarg>1){
      switch(arg[1]){
      default:
        Usage(prog,HELP_ALL);
        exit(1);
        break;
      }
    }
    else{
      if(filein==NULL){
        filein=arg;
        continue;
      }
      if(fileout==NULL){
        fileout=arg;
        continue;
      }
    }
  }
  if(filein==NULL||fileout==NULL){
    Usage(prog,HELP_ALL);
    exit(1);
  }
  streamin=fopen(filein,"r");
  streamout=fopen(fileout,"w");

  if(streamin==NULL||streamout==NULL){
    if(streamin==NULL){
      fprintf(stderr,"unable to open %s for input\n",filein);
    }
    if(streamout==NULL){
      fprintf(stderr,"unable to open %s for output\n",fileout);
    }
    exit(1);
  }
  fprintf(streamout,"@echo off\n");
  for(;;){
    if(fgets(buffer,1024,streamin)==NULL)break;
    TrimBack(buffer);
    if(strlen(buffer)==0){
      fprintf(streamout,"\n");
      continue;
    }
    if(buffer[0]=='#'){
      if(strlen(buffer)>0){
        fprintf(streamout,":: %s\n",buffer+1);
      }
      else{
        fprintf(streamout,"::\n");
      }
      continue;
    }
    if(buffer[0]=='$'){
      char *comm_beg, *comm_end, *data;
      int j;

      comm_beg=buffer+1;
      comm_end=strchr(buffer,' ');
      if(comm_end == NULL)continue;
      data = comm_end+1;
      *comm_end=0;

      TrimBack(data);
      for(j = 0;j<strlen(data);j++){
        if(data[j]=='/')data[j] = '\\';
      }
      fprintf(streamout,"%s%s%s %s\n","%",comm_beg,"%",data);
      continue;
    }
    fprintf(streamout,"%s\n",buffer);
  }
}

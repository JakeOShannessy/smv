#ifndef WINPATH_H_DEFINED
#define WINPATH_H_DEFINED
#if defined(_WIN32) && defined(pp_UNICODE_PATHS)

#include "file_util.h"
#include <stdio.h>

char *WinGetHomeDir();
char *WinGetAppDir();
FILE *WinFOpen(const char *file, const char *mode);
int WinMkDir(const char *file);
int WinAccess(const char *file, int mode);
int WinStat(const char *file, STRUCTSTAT *buffer);
int WinChDir(const char *file);
int WinUnlink(const char *file);
int WinMakeFileList(const char *path, char *filter, int maxfiles,
                    int sort_files, filelistdata **filelist, int mode);
#endif
#endif

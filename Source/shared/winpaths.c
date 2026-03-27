/// @file winpaths.h
#if defined(_WIN32) && defined(pp_UNICODE_PATHS)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __MINGW32__
#undef S_IFBLK
#undef S_ISBLK
#undef S_ISFIFO
#undef S_ISDIR
#undef S_ISCHR
#undef S_ISREG
#endif
#include <direct.h>
#include <io.h>

#include <dirent_win.h>
#include <shlobj.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#include <pathcch.h>
#include <stdio.h>
#include <strsafe.h>
#include <tchar.h>
#include <windows.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "PathCch.lib")

#include "dmalloc.h"
#include "file_util.h"
#include "string_util.h"

char *WinGetHomeDir() {
  char *homedir = NULL;
  wchar_t *out;
  HRESULT res = SHGetKnownFolderPath(&FOLDERID_Profile, 0, NULL, &out);
  switch(res) {
  case S_OK:
    homedir = convert_utf16_to_utf8(out);
    CoTaskMemFree(out);
    break;
  default:
    CoTaskMemFree(out);
  }
  return homedir;
}

char *WinGetAppDir() {
  char *buffer;
  wchar_t *out;
  HRESULT res = SHGetKnownFolderPath(&FOLDERID_RoamingAppData, 0, NULL, &out);
  switch(res) {
  case S_OK:
    char *r = convert_utf16_to_utf8(out);
    CoTaskMemFree(out);
    buffer = CombinePaths(r, "Fireng\\Smokeview Lua\\");
    FREEMEMORY(r);
    return buffer;
  default:
    CoTaskMemFree(out);
    return NULL;
  }
}

FILE *WinFOpen(const char *file, const char *mode) {
  wchar_t *path = convert_utf8_to_utf16(file);
  wchar_t *wmode = convert_utf8_to_utf16(mode);
  FILE *stream = _wfsopen(path, wmode, _SH_DENYNO);
  FREEMEMORY(path);
  FREEMEMORY(wmode);
  return stream;
}

int WinMkDir(const char *file) {
  wchar_t *path = convert_utf8_to_utf16(file);
  int r = CreateDirectoryW(path, NULL);
  FREEMEMORY(path);
  return r;
}

int WinAccess(const char *file, int mode) {
  wchar_t *path = convert_utf8_to_utf16(file);
  int r = _waccess(path, mode);
  FREEMEMORY(path);
  return r;
}

int WinStat(const char *file, STRUCTSTAT *buffer) {
  wchar_t *path = convert_utf8_to_utf16(file);
  int r = _wstat64(path, buffer);
  FREEMEMORY(path);
  return r;
}

int WinChDir(const char *file) {
  wchar_t *path = convert_utf8_to_utf16(file);
  int r = SetCurrentDirectoryW(path);
  FREEMEMORY(path);
  return r;
}

int WinUnlink(const char *file) {
  wchar_t *path = convert_utf8_to_utf16(file);
  int r = _wunlink(path);
  FREEMEMORY(path);
  return r;
}

/// @brief Print an error from the windows API to stderr
/// @param lpszFunction
void DisplayErrorBox(LPTSTR lpszFunction) {
  WCHAR *lpMsgBuf = NULL;
  WCHAR *lpDisplayBuf = NULL;
  DWORD dw = GetLastError();
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf,
                 0, NULL);
  lpDisplayBuf =
      (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) +
                                         lstrlen((LPCTSTR)lpszFunction) + 40) *
                                            sizeof(WCHAR));
  StringCchPrintfW((LPWSTR)lpDisplayBuf,
                   LocalSize(lpDisplayBuf) / sizeof(WCHAR),
                   L"%s failed with error %d: %s", lpszFunction, dw, lpMsgBuf);
  fwprintf(stderr, L"%s", lpDisplayBuf);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
}

int WinCompareFileList(const void *arg1, const void *arg2) {
  filelistdata *x, *y;

  x = (filelistdata *)arg1;
  y = (filelistdata *)arg2;

  return strcmp(x->file, y->file);
}

int WinMakeFileList(const char *path, char *filter, int maxfiles,
                    int sort_files, filelistdata **filelist, int mode) {
  int nfiles = 0;
  filelistdata *flist;

  if(maxfiles == 0 || path == NULL || filter == NULL) {
    if(filelist != NULL) *filelist = NULL;
    return 0;
  }

  wchar_t *pathw = convert_utf8_to_utf16(path);

  WIN32_FIND_DATAW ffd;
  WCHAR szDir[MAX_PATH];
  size_t length_of_arg;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  StringCchLengthW(pathw, MAX_PATH, &length_of_arg);
  if(length_of_arg > (MAX_PATH - 3)) {
    fprintf(stderr, "Directory path is too long.\n");
    return (-1);
  }
  StringCchCopyW(szDir, MAX_PATH, pathw);
  StringCchCatW(szDir, MAX_PATH, L"\\*");
  FREEMEMORY(pathw);

  hFind = FindFirstFileW(szDir, &ffd);

  if(INVALID_HANDLE_VALUE == hFind) {
    fwprintf(stderr, L"Unable to open path %s\n", szDir);
    return (0);
  }
  if(maxfiles > 0) {
    *filelist = NULL;
    // If maxfiles is less than zero we're only in count mode and don't need to
    // allocate an array.
    NewMemory((void **)&flist, maxfiles * sizeof(filelistdata));
  }
  do {
    int is_dir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    int rel_type =
        (mode == DIR_MODE && is_dir) || (mode == FILE_MODE && !is_dir);
    if(wcsncmp(ffd.cFileName, L".", 4) == 0 ||
       wcsncmp(ffd.cFileName, L"..", 4) == 0)
      continue;
    char *fileNameA = convert_utf16_to_utf8(ffd.cFileName);
    int cRes = MatchWild(fileNameA, filter);
    if(rel_type && cRes == 1) {
      LPWSTR file;
      filelistdata *flisti;
      if(maxfiles > 0) {
        // If maxfiles is less than zero we're only in count mode and don't need
        // to record file names
        flisti = flist + nfiles;
        if(mode == DIR_MODE) {
          size_t l1 = wcslen(szDir);
          size_t l2 = wcslen(ffd.cFileName);
#ifdef pp_UNICODE_PATHS
          NEWMEMORY(file, l1 * sizeof(WCHAR) + l2 * sizeof(WCHAR) + 4);
#else
          NEWMEMORY(file, l1 + l2 + 2);
#endif
#pragma warning(suppress : 4995)
          PathCombineW(file, szDir, ffd.cFileName);
        }
        else {
          size_t l;
          StringCchLengthW(ffd.cFileName, MAX_PATH, &l);
          NEWMEMORY(file, l * sizeof(WCHAR) + 4);
#pragma warning(suppress : 4995)
          PathCombineW(file, NULL, ffd.cFileName);
        }
#if pp_UNICODE_PATHS
        flisti->file = convert_utf16_to_utf8(file);
#else
        flisti->file = file;
#endif
        flisti->type = 0;
        FREEMEMORY(file);
      }
      nfiles++;
    }
    FREEMEMORY(fileNameA);
  } while(FindNextFileW(hFind, &ffd) != 0);
  DWORD dwError = 0;
  dwError = GetLastError();
  if(dwError != ERROR_NO_MORE_FILES) {
    DisplayErrorBox(TEXT("FindFirstFile"));
  }
  FindClose(hFind);
  if(sort_files == YES && nfiles > 0) {
    qsort((filelistdata *)flist, (size_t)nfiles, sizeof(filelistdata),
          WinCompareFileList);
  }
  if(maxfiles > 0) {
    // If maxfiles is less than zero we're only in count mode and don't need
    // to record file names
    *filelist = flist;
  }
  return nfiles;
}

char *WinGetBinPath() {
  size_t max_buffer_size = MAX_PATH * 20;
  char *buffer;
  size_t buffer_size = MAX_PATH * sizeof(char);
  NEWMEMORY(buffer, buffer_size);
  for(;;) {
    GetModuleFileNameA(NULL, buffer, buffer_size);
    DWORD dw = GetLastError();
    if(dw == ERROR_SUCCESS) {
      return buffer;
    }
    else if(dw == ERROR_INSUFFICIENT_BUFFER && buffer_size < max_buffer_size) {
      // increase buffer size by a factor of 2
      buffer_size *= 2;
      RESIZEMEMORY(buffer, buffer_size);
    }
    else {
      FREEMEMORY(buffer);
      return NULL;
    }
  }
}
#endif

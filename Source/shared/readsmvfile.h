#ifndef READSMVFILE_H_DEFINED
#define READSMVFILE_H_DEFINED

#include "options_common.h"
#include "shared_structures.h"

/**
 * @brief A library-global variable that sets options for the parsing process.
 * This uses "extern CCC" as sometimes this file is included as part of a C++
 * compilation unit rather than compiled into a library.
 *
 */
extern CCC parse_options parse_opts;
EXTERNCPP int GetNDevices(char *file);
EXTERNCPP void AddCfastCsvf(smv_case *scase);
EXTERNCPP int ReadSMV_Init(smv_case *scase);
EXTERNCPP int ReadSMV_Parse(smv_case *scase, bufferstreamdata *stream);
EXTERNCPP int CompareSmoketypes(const void *arg1, const void *arg2);
EXTERNCPP int IsDupTexture(smv_case *scase, texturedata *texti);
EXTERNCPP int IsTerrainTexture(smv_case *scase, texturedata *texti);
EXTERNCPP surfdata *GetSurface(smv_case *scase, const char *label);
EXTERNCPP int ParseCHIDProcess(smv_case *scase, bufferstreamdata *stream,
                               int option);
EXTERNCPP float *GetColorPtr(smv_case *scase, float *color);
EXTERNCPP void GetElevAz(float *xyznorm, float *dtheta, float *rotate_axis,
                         float *dpsi);
EXTERNCPP void ReadSMVOrig(smv_case *scase);
EXTERNCPP void ReadSMVDynamic(smv_case *scase, char *file);
EXTERNCPP FILE_SIZE ReadCSVFile(smv_case *scase, csvfiledata *csvfi, int flag);
EXTERNCPP void ReadHRR(smv_case *scase, int flag);
EXTERNCPP int GetSmoke3DType(smv_case *scase, const char *label);
#ifdef pp_SMOKE3D_FORCE
EXTERNCPP int HaveSmoke3D(bufferstreamdata *stream);
#endif

/// @brief Create and initalize and a smokeview case (smv_case). It is necessary
/// to use this function rather than simply allocate the memory as certain
/// initilization steps need to be taken before parsing occurs.
/// @return An initialized smv_case.
EXTERNCPP smv_case *CreateScase();
/// @brief Read an SMV file and parse it into an object.
/// @param[in] input_file Path to the SMV file
/// @param[out] scase A struct to store the resulting parsed SMV file
/// @return zero on success, nonzero on failure.
EXTERNCPP int ScaseParseFromPath(const char *input_file, smv_case *scase);
/// @brief Destroy an smv_case and free the associated memory
/// @param[inout] scase The smv_case deconstruct and free.
EXTERNCPP void DestroyScase(smv_case *scase);
#endif

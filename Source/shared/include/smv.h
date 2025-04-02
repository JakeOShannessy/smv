#ifndef SMV_H_DEFINED
#define SMV_H_DEFINED

#include "shared_structures.h"

void initMALLOC(void);

/// @brief Create and initalize and a smokeview case (smv_case). It is necessary
/// to use this function rather than simply allocate the memory as certain
/// initilization steps need to be taken before parsing occurs.
/// @return An initialized smv_case.
EXTERNCPP smv_case *ScaseCreate();

/// @brief Read an SMV file and parse it into an object.
/// @param[in] input_file Path to the SMV file
/// @param[out] scase A struct to store the resulting parsed SMV file
/// @return zero on success, nonzero on failure.
EXTERNCPP int ScaseParseFromPath(const char *input_file, smv_case *scase);

/// @brief Clear the memory within an smv_case (i.e. unitialize) but don't free
/// the struct itself. This is useful when the struct is a global.
/// @param[inout] scase An smv_case.
EXTERNCPP void ScaseClear(smv_case *scase);

/// @brief Destroy an smv_case and free the associated memory
/// @param[inout] scase The smv_case deconstruct and free.
EXTERNCPP void ScaseDestroy(smv_case *scase);

#endif

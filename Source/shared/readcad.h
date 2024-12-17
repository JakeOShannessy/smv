#ifndef READCAD_H_DEFINED
#define READCAD_H_DEFINED

#include "shared_structures.h"

/**
 * @brief Initialise an @ref cadgeom_collection.
 *
 * @param[out] coll The memory to initialise the colleciton in.
 * @param capacity The maximum capacity of this collection.
 *
 * @returns zero on suceess, error code otherwise
 */
int CreateCADGeomCollection(cadgeom_collection *coll,
                                            int capacity);

int InitCADGeomCollection(cadgeom_collection *coll, int capacity);

/**
 * @brief Free an @ref cadgeom_collection previously created by @ref
 * CreateCADGeomCollection.
 *
 * @param[inout] coll The @ref cadgeom_collection to free.
 */
void FreeCADGeomCollection(cadgeom_collection *coll);

/**
 * @brief Read CAD geometry data and add it to a collection.
 *
 * @param coll A pointer to the cadgeom_collection, this must have been created
 * with CreateCADGeomCollection.
 * @param file A path to the file to read.
 * @param block_shininess The block shininess to apply.
 */
int ReadCADGeomToCollection(cadgeom_collection *coll, const char *file,
                            GLfloat block_shininess);

/**
 * @brief Return the number of CAD object definitions.
 *
 * @param coll The collection.
 * @return  Number of defined objects. If coll is NULL, return 0.
 */
int NCADGeom(cadgeom_collection *coll);

#endif

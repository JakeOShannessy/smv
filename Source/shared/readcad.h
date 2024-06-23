#ifndef READCAD_H_DEFINED
#define READCAD_H_DEFINED

#define BUFFER_LEN 255

// #include "options.h"
#include <GL/gl.h>

typedef struct _texturedata {
  char *file;
  int loaded, display, used, is_transparent;
  GLuint name;
} texturedata;

typedef struct _cadlookdata {
  int index;
  float texture_width, texture_height, texture_origin[3];
  float rgb[4], shininess;
  texturedata textureinfo;
  int onesided;
} cadlookdata;

typedef struct _cadquad {
  float xyzpoints[12];
  float txypoints[8];
  float normals[3];
  int colorindex;
  float colors[4];
  float time_show;
  cadlookdata *cadlookq;
} cadquad;

typedef struct _cadgeomdata {
  char *file;
  int *order;
  int version;
  int ncadlookinfo;
  cadlookdata *cadlookinfo;
  int nquads;
  cadquad *quad;
} cadgeomdata;

void CalcQuadNormal(float *xyz, float *out);
int CompareQuad(const void *arg1, const void *arg2);
void ReadCAD2Geom(cadgeomdata *cd, GLfloat block_shininess);
void ReadCADGeom(cadgeomdata *cd, GLfloat block_shininess);
void FreeCADInfo(cadgeomdata *cadgeominfo, int ncadgeom);
#endif

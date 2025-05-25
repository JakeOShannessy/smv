#ifndef C_API_H_DEFINED
#define C_API_H_DEFINED
#include "options_common.h"

#include "gd.h"

// Verified, the declarations below are part of the verified and test API.
#define ERROR_CODE int
#define ERR_OK 0
#define ERR_NOK 1

typedef struct _simple_bounds {
  float min, max;
} simple_bounds;

void SetColorbar(size_t value);
ERROR_CODE SetNamedColorbar(const char *name);
ERROR_CODE GetNamedColorbar(const char *name, size_t *index);
ERROR_CODE CApiSetSliceBounds(const char *slice_type, int set_min,
                              float value_min, int set_max, float value_max);
ERROR_CODE SetSliceBoundMin(const char *slice_type, int set, float value);
ERROR_CODE SetSliceBoundMax(const char *slice_type, int set, float value);
ERROR_CODE GetSliceBounds(const char *slice_type, simple_bounds *bounds);
ERROR_CODE CApiRender(const char *filename);
int Getframe();
void Setframe(int framenumber);
float Gettime();
int Settime(float timeval);

int SetRendertype(const char *type);
int GetRendertype(void);
void SetMovietype(const char *type);
int GetMovietype(void);
FILE_SIZE Loadsliceindex(size_t index, int frame, int *errorcode);
int Unloadall();
void ExitSmokeview();
void Setcolorbarflip(int flip);
int Getcolorbarflip();
int Setviewpoint(const char *viewpoint);
int SetOrthoPreset(const char *viewpoint);
int Setrenderdir(const char *dir);
void Setwindowsize(int width, int height);
void CameraSetAz(float az);
int CameraSetProjectionType(int projection_type);

int GetClippingMode();
void SetClippingMode(int mode);
void SetSceneclipXMin(int flag, float value);
void SetSceneclipXMax(int flag, float value);
void SetSceneclipYMin(int flag, float value);
void SetSceneclipYMax(int flag, float value);
void SetSceneclipZMin(int flag, float value);
void SetSceneclipZMax(int flag, float value);

int RenderFrameLua(int view_mode, const char *basename);

// title
void SetTitleVisibility(int setting);
int GetTitleVisibility();

// smv_version
void SetSmvVersionVisibility(int setting);
int GetSmvVersionVisibility();

// chid
void SetChidVisibility(int setting);
int GetChidVisibility();

// blockages
void BlockagesHideAll();

// outlines
void OutlinesHide();
// void OutlinesShow();

// surfaces
void SurfacesHideAll();

// devices
void DevicesHideAll();

int SetFontsize(int v);             // FONTSIZE
int SetScaledfontHeight2d(int height2d);

#define PROPINDEX_STRIDE 2
#endif

#ifndef SMVFILE_H_DEFINED
#define SMVFILE_H_DEFINED

typedef struct {
  char *fuel;
  float hoc;
} fueldata;

typedef struct _surfdata {
  char *surfacelabel;
  float color[4];
  char *texture;
  float emis;
  int geom_surf_color[3];
  float temp_ignition;
  /// @brief
  ///    0 - regular block non-textured
  ///    1 - regular block textured
  ///    2 - outline
  ///    3 - smoothed block
  ///    4 - invisible
  int type;
  float t_width, t_height;
} surfdata;

typedef struct {
  fueldata *fuelinfo;
  size_t nfuelinfo;
  char *fds_title;
  char *fds_version;
  surfdata *surfinfo;
  size_t nsurfinfo;
  size_t cap_surfinfo;
} SmvFile;

int ReadSMV(SmvFile *smvfile, FILE *stream);

#endif

#include "options.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include GLUT_H

#include "smokeviewvars.h"

#include "IOscript.h"
#include "IOvolsmoke.h"
#include "c_api.h"
#include "gd.h"
#include "glui_bounds.h"
#include "glui_motion.h"
#include "glui_smoke.h"
#include "infoheader.h"
#include "readsmvfile.h"
#include "smokeheaders.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include "readsmoke.h"

void UnLoadVolsmoke3DMenu(int value);
void OutputSliceData(void);
void LoadVolsmoke3DMenu(int value);

/// @brief Given a quantity type, return the appropriate index into the
/// slicebounds array.
/// @param slice_type A string describing the slice quantity.
/// @return If successful, the index > 0. If not found -1.
int GetSliceBoundIndex(const char *slice_type) {
  for(int i = 0; i < nslicebounds; i++) {
    if(strcmp(slicebounds[i].shortlabel, slice_type) == 0) {
      return i;
    }
  }
  return -1;
}

/// @brief Set the minimum bound of a given slice quantity.
/// @param slice_type The quantity string
/// @param set
/// @param value
/// @return Non-zero on error
int SetSliceBoundMin(const char *slice_type, int set, float value) {
  int slice_type_index = GetSliceBoundIndex(slice_type);
  if(slice_type_index < 0) {
    // Slice type index could not be found.
    return 1;
  }
  slicebounds[slice_type_index].dlg_setvalmin = set;
  slicebounds[slice_type_index].dlg_valmin = value;
  int error = 0;
  UpdateSliceBounds();
  // Update the colors given the bounds set above
  UpdateAllSliceColors(slice_type_index, &error);
  return error;
}

/// @brief Set the maximum bound of a given slice quantity.
/// @param[in] slice_type
/// @param[in] set
/// @param[in] value
/// @return
int SetSliceBoundMax(const char *slice_type, int set, float value) {
  int slice_type_index = GetSliceBoundIndex(slice_type);
  if(slice_type_index < 0) {
    // Slice type index could not be found.
    return 1;
  }
  slicebounds[slice_type_index].dlg_setvalmax = set;
  slicebounds[slice_type_index].dlg_valmax = value;
  int error = 0;
  UpdateSliceBounds();
  // Update the colors given the bounds set above
  UpdateAllSliceColors(slice_type_index, &error);
  return error;
}

/// @brief Set the bounds of a given slice quantity.
/// @param[in] slice_type
/// @param[in] set_valmin
/// @param[in] valmin
/// @param[in] set_valmax
/// @param[in] valmax
/// @return Non-zero on error
int CApiSetSliceBounds(const char *slice_type, int set_valmin, float valmin,
                       int set_valmax, float valmax) {
  int slice_type_index = GetSliceBoundIndex(slice_type);
  if(slice_type_index < 0) {
    // Slice type index could not be found.
    return 1;
  }
  // Set the requested bounds
  slicebounds[slice_type_index].dlg_setvalmin = set_valmin;
  slicebounds[slice_type_index].dlg_setvalmax = set_valmax;
  slicebounds[slice_type_index].dlg_valmin = valmin;
  slicebounds[slice_type_index].dlg_valmax = valmax;
  GLUISetMinMax(BOUND_SLICE, slicebounds[slice_type_index].shortlabel,
                set_valmin, valmin, set_valmax, valmax);
  UpdateSliceBounds2();
  return 0;
}

/// @brief Get the slice bounds of a given slice quantity.
/// @param[in] slice_type
/// @param[out] A simple_bounds struct containing the min and max bounds being
/// used.
/// @return Non-zero on error
ERROR_CODE GetSliceBounds(const char *slice_type, simple_bounds *bounds) {
  int slice_type_index = GetSliceBoundIndex(slice_type);
  if(slice_type_index < 0) {
    // Slice type index could not be found.
    return ERR_NOK;
  }
  (*bounds).min = slicebounds[slice_type_index].dlg_valmin;
  (*bounds).max = slicebounds[slice_type_index].dlg_valmax;
  return ERR_OK;
}

ERROR_CODE CApiRender(const char *filename) {
  DisplayCB();
  // strcpy(render_file_base,filename);
  return RenderFrameLua(VIEW_CENTER, filename);
}

/// @brief Construct filepath for image to be rendered.
/// @param view_mode
/// @param renderfile_name
/// @param renderfile_dir
/// @param renderfile_path
/// @param woffset
/// @param hoffset
/// @param screenH
/// @param basename
/// @return
char *FormFilename(int view_mode, char *renderfile_name, char *renderfile_dir,
                   char *renderfile_path, int woffset, int hoffset, int screenH,
                   const char *basename) {
  char *renderfile_ext;
  char *view_suffix;

  // determine the extension to be used, and set renderfile_ext to it
  switch(render_filetype) {
  case 0:
    renderfile_ext = ext_png;
    break;
  case 1:
    renderfile_ext = ext_jpg;
    break;
  default:
    render_filetype = 2;
    renderfile_ext = ext_png;
    break;
  }

  // if the basename has not been specified, use a predefined method to
  // determine the filename
  if(basename == NULL) {
    view_suffix = "";
    switch(view_mode) {
    case VIEW_LEFT:
      if(stereotype == STEREO_LR) {
        view_suffix = "_L";
      }
      break;
    case VIEW_RIGHT:
      if(stereotype == STEREO_LR) {
        view_suffix = "_R";
      }
      break;
    case VIEW_CENTER:
      break;
    default:
      assert(FFALSE);
      break;
    }

    if(Writable(renderfile_dir) == NO) {
      // TODO: ensure this can be made cross-platform
      if(strlen(renderfile_dir) > 0) {
#if defined(__MINGW32__)
        mkdir(renderfile_dir);
#elif defined(WIN32)
        CreateDirectory(renderfile_dir, NULL);
#else // linux or osx
        mkdir(renderfile_dir, 0755);
#endif
        // #ifdef __MINGW32__
        //                 fprintf(stderr, "%s\n", "making directory(mingw)\n");
        //                 mkdir(renderfile_dir);
        // #elif defined(pp_LINUX)
        //                 fprintf(stderr, "%s\n", "making directory(linux)\n");
        //                 mkdir(renderfile_dir, 0755);
        // #endif
        // #ifdef pp_OSX
        //                 mkdir(renderfile_dir, 0755);
        // #endif
      }
    }
    if(stereotype == STEREO_LR &&
       (view_mode == VIEW_LEFT || view_mode == VIEW_RIGHT)) {
    }

    snprintf(renderfile_name, 1024, "%s%s%s", global_scase.paths.chidfilebase,
             view_suffix, renderfile_ext);
  }
  else {
    snprintf(renderfile_name, 1024, "%s%s", basename, renderfile_ext);
  }
  return renderfile_name;
}

/// @brief Render a frame to file.
///
/// This is function fulfills the exact same purpose as the original
/// RenderFrame function, except that it takes a second argument, basename. This
/// could be be used as a drop in replacement as long as all existing calls are
/// modified to use basename = NULL.
/// @param view_mode
/// @param basename This is the name that should be given to the rendered file.
/// If NULL then a default filename is formed based on the chosen frame and
/// rendering options.
/// @return
int RenderFrameLua(int view_mode, const char *basename) {
  char renderfile_name[1024]; // the name the file (including extension)
  char renderfile_dir[1024];  // the directory into which the image will be
                              // rendered
  char renderfile_path[2048]; // the full path of the rendered image
  int woffset = 0, hoffset = 0;
  int screen_h;
  int return_code;

  if(script_dir_path != NULL) {
    strcpy(renderfile_dir, script_dir_path);
  }
  else {
    strcpy(renderfile_dir, ".");
  }

#ifdef WIN32
  // reset display idle timer to prevent screen saver from activating
  SetThreadExecutionState(ES_DISPLAY_REQUIRED);
#endif

  screen_h = screenHeight;
  // we should not be rendering under these conditions
  if(view_mode == VIEW_LEFT && stereotype == STEREO_RB) return 0;
  // construct filename for image to be rendered
  FormFilename(view_mode, renderfile_name, renderfile_dir, renderfile_path,
               woffset, hoffset, screen_h, basename);
  // render image
  return_code =
      SmokeviewImage2File(renderfile_dir, renderfile_name, render_filetype,
                          woffset, screenWidth, hoffset, screen_h);
  if(RenderTime == 1 && output_slicedata == 1) {
    OutputSliceData();
  }
  return return_code;
}
void LoadCsv(csvfiledata *csventry) {
  ReadCSVFile(&global_scase, csventry, LOAD);
  csventry->loaded = 1;
}


/// @brief Get the current frame number.
/// @return Time value in seconds.
int Getframe() {
  int framenumber = itimes;
  return framenumber;
}

/// @brief Get the time value of the current frame.
/// @return
float Gettime() { return global_times[itimes]; }

/// @brief Set the currrent time.
///
/// Switch to the frame with the closest time value to @p timeval.
/// @param timeval Time in seconds
/// @return Non-zero on error
int Settime(float timeval) {
  if(global_times != NULL && nglobal_times > 0) {
    if(timeval < global_times[0]) timeval = global_times[0];
    if(timeval > global_times[nglobal_times - 1] - 0.0001) {
#ifdef pp_SETTIME
      float dt;

      dt = timeval - global_times[nglobal_times - 1] - 0.0001;
      if(nglobal_times > 1 && dt > global_times[1] - global_times[0]) {
        fprintf(stderr, "*** Error: data not available at time requested\n");
        fprintf(stderr, "           time: %f s, min time: %f, max time: %f s\n",
                timeval, global_times[0], global_times[nglobal_times - 1]);
        if(script_labelstring != NULL)
          fprintf(stderr,
                  "                 "
                  "label: %s\n",
                  script_labelstring);
      }
#endif
      timeval = global_times[nglobal_times - 1] - 0.0001;
    }
    float valmin = ABS(global_times[0] - timeval);
    int imin = 0;
    for(int i = 1; i < nglobal_times; i++) {
      float val = ABS(global_times[i] - timeval);
      if(val < valmin) {
        valmin = val;
        imin = i;
      }
    }
    itimes = imin;
    script_itime = imin;
    stept = 0;
    force_redisplay = 1;
    UpdateFrameNumber(0);
    UpdateTimeLabels();
    return 0;
  }
  else {
    return 1;
  }
}

/// @brief Set the colorbar to one named @p name
/// @param name
/// @return
ERROR_CODE SetNamedColorbar(const char *name) {
  size_t index = 0;
  if(GetNamedColorbar(name, &index)) {
    return ERR_NOK;
  }
  SetColorbar(index);
  return ERR_OK;
}
ERROR_CODE GetNamedColorbar(const char *name, size_t *index) {
  for(size_t i = 0; i < colorbars.ncolorbars; i++) {
    if(strcmp(colorbars.colorbarinfo[i].menu_label, name) == 0) {
      *index = i;
      return 0;
    }
  }
  return 1;
}

/// @brief Set the colorbar to the given colorbar index.
/// @param value
void SetColorbar(size_t value) {
  colorbartype = value;
  colorbars.iso_colorbar_index = value;
  iso_colorbar = colorbars.colorbarinfo + colorbars.iso_colorbar_index;
  update_texturebar = 1;
  GLUIUpdateListIsoColorobar();
  UpdateCurrentColorbar(colorbars.colorbarinfo + colorbartype);
  GLUIUpdateColorbarType();
  if(colorbartype == colorbars.bw_colorbar_index && colorbars.bw_colorbar_index >= 0) {
    setbwdata = 1;
  }
  else {
    setbwdata = 0;
  }
  GLUIIsoBoundCB(ISO_COLORS);
  GLUISetLabelControls();
  if(value > -10) {
    UpdateRGBColors(COLORBAR_INDEX_NONE);
  }
}
/// @brief Set whether the title of the simulation is visible.
/// @param setting Boolean value.
void SetTitleVisibility(int setting) { vis_title_fds = setting; }

/// @brief Check whether the title of the simulation is visible.
/// @return
int GetTitleVisibility() { return vis_title_fds; }

void SetSmvVersionVisibility(int setting) { vis_title_smv_version = setting; }

int GetSmvVersionVisibility() { return vis_title_smv_version; }

void SetChidVisibility(int setting) { vis_title_CHID = setting; }

int GetChidVisibility() { return vis_title_CHID; }

void BlockagesHideAll() { BlockageMenu(visBLOCKHide); }
// TODO: clarify behaviour under isZoneFireModel
void OutlinesHide() {
  if(global_scase.isZoneFireModel == 0) global_scase.visFrame = 0;
}

void SurfacesHideAll() {
  visVents = 0;
  visOpenVents = 0;
  visDummyVents = 0;
  global_scase.visOtherVents = 0;
  visCircularVents = VENT_HIDE;
}

void DevicesHideAll() {
  for(size_t i = 0; i < global_scase.objectscoll.nobject_defs; i++) {
    sv_object *objecti = global_scase.objectscoll.object_defs[i];
    objecti->visible = 0;
  }
}

void Setframe(int framenumber) {
  itimes = framenumber;
  script_itime = itimes;
  stept = 0;
  force_redisplay = 1;
  UpdateFrameNumber(0);
  UpdateTimeLabels();
}

int SetRendertype(const char *type) {
  if(STRCMP(type, "JPG") == 0 || STRCMP(type, "JPEG") == 0) {
    render_filetype = JPEG;
    return 0;
  }
  else if(STRCMP(type, "PNG") == 0) {
    render_filetype = PNG;
    return 0;
  }
  else {
    return 1;
  }
  UpdateRenderType(render_filetype);
}

int GetRendertype() { return render_filetype; }

void SetMovietype(const char *type) {
  if(STRCMP(type, "WMV") == 0) {
    UpdateMovieType(WMV);
  }
  if(STRCMP(type, "MP4") == 0) {
    UpdateMovieType(MP4);
  }
  else {
    UpdateMovieType(AVI);
  }
}

int GetMovietype() { return movie_filetype; }

FILE_SIZE Loadsliceindex(size_t index, int *errorcode) {
  return ReadSlice(global_scase.slicecoll.sliceinfo[index].file, (int)index,
                   ALL_FRAMES, NULL, LOAD, SET_SLICECOLOR, errorcode);
}

/// @brief Unload all the currently loaded data.
int Unloadall() {
  int errorcode = 0;

  if(scriptoutstream != NULL) {
    fprintf(scriptoutstream, "UNLOADALL\n");
  }
  if(global_scase.paths.hrr_csv_filename != NULL) {
    ReadHRR(&global_scase, UNLOAD);
  }
  if(nvolrenderinfo > 0) {
    LoadVolsmoke3DMenu(UNLOAD_ALL);
  }
  for(size_t i = 0; i < global_scase.slicecoll.nsliceinfo; i++) {
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo + i;
    if(slicei->loaded == 1) {
      if(slicei->slice_filetype == SLICE_GEOM) {
        ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, 0,
                     &errorcode);
      }
      else {
        ReadSlice(slicei->file, i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR,
                  &errorcode);
      }
    }
  }
  for(size_t i = 0; i < global_scase.nplot3dinfo; i++) {
    ReadPlot3D("", i, UNLOAD, &errorcode);
  }
  for(size_t i = 0; i < global_scase.npatchinfo; i++) {
    ReadBoundary(i, UNLOAD, &errorcode);
  }
  for(size_t i = 0; i < global_scase.npartinfo; i++) {
    ReadPart("", i, UNLOAD, &errorcode);
  }
  for(size_t i = 0; i < global_scase.nisoinfo; i++) {
    ReadIso("", i, UNLOAD, NULL, &errorcode);
  }
  for(size_t i = 0; i < global_scase.nzoneinfo; i++) {
    ReadZone(i, UNLOAD, &errorcode);
  }
  for(size_t i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++) {
    ReadSmoke3D(ALL_SMOKE_FRAMES, i, UNLOAD, FIRST_TIME, &errorcode);
  }
  if(nvolrenderinfo > 0) {
    UnLoadVolsmoke3DMenu(UNLOAD_ALL);
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  return errorcode;
}

/// @brief Exit smokeview.
void ExitSmokeview() { exit(EXIT_SUCCESS); }

int Setviewpoint(const char *viewpoint) {
  int count = 0;
  int errorcode = 0;
  for(cameradata *ca = camera_list_first.next; ca->next != NULL;
      ca = ca->next) {
    if(strcmp(viewpoint, ca->name) == 0) {
      ResetMenu(ca->view_id);
      count++;
      break;
    }
  }
  if(count == 0) {
    errorcode = 1;
    fprintf(stderr, "*** Error: The viewpoint %s was not found\n", viewpoint);
  }
  fprintf(stderr, "Viewpoint set to %s\n", camera_current->name);
  return errorcode;
}

/// @brief Switch to a preset orthographic view.
/// @param viewpoint A string describe the view. Currently one of:
/// - "XMIN"
/// - "XMAX"
/// - "YMIN"
/// - "YMAX"
/// - "ZMIN"
/// - "ZMAX"
/// @return
int SetOrthoPreset(const char *viewpoint) {
  int command;
  fprintf(stderr, "setting ortho %s\n", viewpoint);
  if(STRCMP(viewpoint, "XMIN") == 0) {
    command = SCRIPT_VIEWXMIN;
  }
  else if(STRCMP(viewpoint, "XMAX") == 0) {
    command = SCRIPT_VIEWXMAX;
  }
  else if(STRCMP(viewpoint, "YMIN") == 0) {
    command = SCRIPT_VIEWYMIN;
  }
  else if(STRCMP(viewpoint, "YMAX") == 0) {
    command = SCRIPT_VIEWYMAX;
  }
  else if(STRCMP(viewpoint, "ZMIN") == 0) {
    command = SCRIPT_VIEWZMIN;
  }
  else if(STRCMP(viewpoint, "ZMAX") == 0) {
    command = SCRIPT_VIEWZMAX;
  }
  else {
    return 1;
  }
  ScriptViewXYZMINMAXOrtho(command);
  return 0;
}

int GetClippingMode() { return clip_mode; }

void SetClippingMode(int mode) {
  clip_mode = mode;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

int SetFontsize(int v) {
  FontMenu(v);
  return 0;
} // FONTSIZE

void SetSceneclipXMin(int flag, float value) {
  clipinfo.clip_xmin = flag;
  clipinfo.xmin = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipXMax(int flag, float value) {
  clipinfo.clip_xmax = flag;
  clipinfo.xmax = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipYMin(int flag, float value) {
  clipinfo.clip_ymin = flag;
  clipinfo.ymin = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipYMax(int flag, float value) {
  clipinfo.clip_ymax = flag;
  clipinfo.ymax = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipZMin(int flag, float value) {
  clipinfo.clip_zmin = flag;
  clipinfo.zmin = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipZMax(int flag, float value) {
  clipinfo.clip_zmax = flag;
  clipinfo.zmax = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

int Setrenderdir(const char *dir) {
  // TODO: as lua gives us consts, but most smv code uses non-const, we
  // must make a non-const copy
  int l = strlen(dir);
  char *dir_path_temp = malloc(l + 1);
  strncpy(dir_path_temp, dir, l + 1);
  // TODO: should we make the directory at this point?
  if(dir != NULL && strlen(dir_path_temp) > 0) {
#if defined(__MINGW32__)
    fprintf(stderr, "%s\n", "making directory(mingw)\n");
    mkdir(dir_path_temp);
#elif defined(WIN32)
    fprintf(stderr, "%s\n", "making directory(win32)\n");
    CreateDirectory(dir_path_temp, NULL);
#else // linux or osx
    fprintf(stderr, "%s\n", "making directory(linux/osx)\n");
    mkdir(dir_path_temp, 0755);
#endif
    if(Writable(dir_path_temp) == NO) {
      fprintf(stderr,
              "*** Error: Cannot write to the RENDERDIR "
              "directory: %s\n",
              dir_path_temp);
      return 1;
    }
    else {
      free(script_dir_path);
      script_dir_path = dir_path_temp;
      return 0;
    }
  }
  else {
    // TODO: why would we ever want to set the render directory to NULL
    script_dir_path = NULL;
    FREEMEMORY(dir_path_temp);
    return 1;
  }
}

void Setwindowsize(int width, int height) {
  glutReshapeWindow(width, height);
  ResizeWindow(width, height);
  ReshapeCB(width, height);
}


/// @brief Set the firection of the colorbar.
/// @param flip Boolean. If true, the colorbar runs in the opposite direction
/// than default.
void Setcolorbarflip(int flip) {
  colorbar_flip = flip;
  GLUIUpdateColorbarFlip();
  UpdateRGBColors(COLORBAR_INDEX_NONE);
}

/// @brief Get whether the direction of the colorbar is flipped.
/// @return
int Getcolorbarflip() { return colorbar_flip; }

int CameraSetProjectionType(int pt) {
  camera_current->projection_type = pt;
  projection_type = pt;
  ZoomMenu(UPDATE_PROJECTION);
  camera_current->projection_type = projection_type;
  // 1 is orthogonal
  // 0 is perspective
  return 0;
}

void CameraSetAz(float az) { camera_current->az_elev[0] = az; }

int SetScaledfont(int height2d, float height2dwidth, int thickness2d,
                  int height3d, float height3dwidth, int thickness3d) {
  scaled_font2d_height = height2d;
  scaled_font2d_height2width = height2dwidth;
  scaled_font3d_height = height3d;
  scaled_font3d_height2width = height3dwidth;
  scaled_font2d_thickness = thickness2d;
  scaled_font3d_thickness = thickness3d;
  return 0;
} // SCALEDFONT

int SetScaledfontHeight2d(int height2d) {
  scaled_font2d_height = height2d;
  return 0;
}

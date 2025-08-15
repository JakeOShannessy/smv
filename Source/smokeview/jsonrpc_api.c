
#include "jsonrpc.h"

#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"

#include "smokeviewvars.h"

#include "infoheader.h"

#include "c_api.h"

#include "glui_motion.h"

#include GLUT_H
#include "gd.h"

#if defined(_WIN32)
#include <direct.h>
#endif

#ifdef WIN32
#define snprintf _snprintf
#else
#include <unistd.h>
#endif

/// @brief Render the current frame to a file.
json_object *jsonrpc_Render(jrpc_context *context, json_object *params,
                            json_object *id) {
  DisplayCB();
  const char *basename =
      json_object_get_string(json_object_object_get(params, "basename"));
  int ret = CApiRender(basename);
  if(ret) {
    context->error_code = 111;
    context->error_message = strdup("render failure");
  }
  return NULL;
}

/// @brief Return the current frame number which Smokeivew has loaded.
json_object *jsonrpc_GetFrame(jrpc_context *context, json_object *params,
                              json_object *id) {
  int framenumber = Getframe();
  json_object *result_root = json_object_new_int(framenumber);
  return result_root;
}

/// @brief Shift to a specific frame number.
json_object *jsonrpc_SetFrame(jrpc_context *context, json_object *params,
                              json_object *id) {
  int f = json_object_get_int(json_object_array_get_idx(params, 0));
  fprintf(stderr, "setting frame to %d\n", f);
  Setframe(f);
  return NULL;
}

/// @brief Get the time value of the currently loaded frame.
json_object *jsonrpc_GetTime(jrpc_context *context, json_object *params,
                             json_object *id) {
  if(global_times != NULL && nglobal_times > 0) {
    float time = Gettime();
    json_object *result_root = json_object_new_double(time);
    return result_root;
  }
  else {
    return NULL;
  }
}

/// @brief Get the time value of the currently loaded frame.
json_object *jsonrpc_GetNGlobalTimes(jrpc_context *context, json_object *params,
                                     json_object *id) {
  if(global_times != NULL && nglobal_times > 0) {
    json_object *result_root = json_object_new_int(nglobal_times);
    return result_root;
  }
  else {
    return NULL;
  }
}

/// @brief Shift to the closest frame to given a time value.
json_object *jsonrpc_SetTime(jrpc_context *context, json_object *params,
                             json_object *id) {
  if(!json_object_is_type(params, json_type_array)) {
    context->error_code = JRPC_INVALID_PARAMS;
    context->error_message = strdup("expected an array");
    return NULL;
  }
  DisplayCB();
  float t = json_object_get_double(json_object_array_get_idx(params, 0));
  int return_code = Settime(t);
  if(return_code) {
    context->error_code = 112;
    context->error_message = strdup("set_time failure");
  }
  return NULL;
}

json_object *jsonrpc_SetCameraAz(jrpc_context *context, json_object *params,
                                 json_object *id) {
  if(!json_object_is_type(params, json_type_array)) {
    context->error_code = JRPC_INVALID_PARAMS;
    context->error_message = strdup("expected an array");
    return NULL;
  }
  float az = json_object_get_double(json_object_array_get_idx(params, 0));
  CameraSetAz(az);
  return NULL;
}

json_object *jsonrpc_SetCameraElev(jrpc_context *context, json_object *params,
                                   json_object *id) {
  if(!json_object_is_type(params, json_type_array)) {
    context->error_code = JRPC_INVALID_PARAMS;
    context->error_message = strdup("expected an array");
    return NULL;
  }
  float elev = json_object_get_double(json_object_array_get_idx(params, 0));
  CameraSetElev(elev);
  return NULL;
}

json_object *jsonrpc_SetCameraEye(jrpc_context *context, json_object *params,
                                  json_object *id) {
  if(!json_object_is_type(params, json_type_array)) {
    context->error_code = JRPC_INVALID_PARAMS;
    context->error_message = strdup("expected an array");
    return NULL;
  }
  float x = json_object_get_double(json_object_array_get_idx(params, 0));
  float y = json_object_get_double(json_object_array_get_idx(params, 1));
  float z = json_object_get_double(json_object_array_get_idx(params, 2));
  CameraSetEye(x, y, z);
  return NULL;
}
json_object *jsonrpc_SetCameraZoom(jrpc_context *context, json_object *params,
                                   json_object *id) {
  if(!json_object_is_type(params, json_type_array)) {
    context->error_code = JRPC_INVALID_PARAMS;
    context->error_message = strdup("expected an array");
    return NULL;
  }
  float x = json_object_get_double(json_object_array_get_idx(params, 0));
  CameraSetZoom(x);
  return NULL;
}
json_object *jsonrpc_SetCameraViewDir(jrpc_context *context,
                                      json_object *params, json_object *id) {
  if(!json_object_is_type(params, json_type_array)) {
    context->error_code = JRPC_INVALID_PARAMS;
    context->error_message = strdup("expected an array");
    return NULL;
  }
  float xcen = json_object_get_double(json_object_array_get_idx(params, 0));
  float ycen = json_object_get_double(json_object_array_get_idx(params, 1));
  float zcen = json_object_get_double(json_object_array_get_idx(params, 2));
  CameraSetViewdir(xcen, ycen, zcen);
  return NULL;
}

json_object *jsonrpc_GetClipping(jrpc_context *context, json_object *params,
                                 json_object *id) {
  json_object *result_root = json_object_new_object();
  json_object_object_add(result_root, "mode",
                         json_object_new_int(GetClippingMode()));
  json_object *x_object = json_object_new_object();
  if(clipinfo.clip_xmin) {
    json_object_object_add(x_object, "min",
                           json_object_new_double(clipinfo.xmin));
  }
  if(clipinfo.clip_xmax) {
    json_object_object_add(x_object, "max",
                           json_object_new_double(clipinfo.xmax));
  }
  json_object_object_add(result_root, "x", x_object);
  json_object *y_object = json_object_new_object();
  if(clipinfo.clip_ymin) {
    json_object_object_add(x_object, "min",
                           json_object_new_double(clipinfo.ymin));
  }
  if(clipinfo.clip_ymax) {
    json_object_object_add(x_object, "max",
                           json_object_new_double(clipinfo.ymax));
  }
  json_object_object_add(result_root, "y", y_object);
  json_object *z_object = json_object_new_object();
  if(clipinfo.clip_zmin) {
    json_object_object_add(x_object, "min",
                           json_object_new_double(clipinfo.zmin));
  }
  if(clipinfo.clip_zmax) {
    json_object_object_add(x_object, "max",
                           json_object_new_double(clipinfo.zmax));
  }
  json_object_object_add(result_root, "z", z_object);
  return result_root;
}

/// @brief Set the clipping mode, which determines which parts of the model are
/// clipped (based on the set clipping values). This function takes an int,
/// which is one
///  of:
///    0: No clipping.
///    1: Clip blockages and data.
///    2: Clip blockages.
///    3: Clip data.
json_object *jsonrpc_SetClipping(jrpc_context *context, json_object *params,
                                 json_object *id) {
  json_object *mode_object;
  if(json_object_object_get_ex(params, "mode", &mode_object)) {
    int mode = json_object_get_int(mode_object);
    SetClippingMode(mode);
  }

  json_object *x_object;
  if(json_object_object_get_ex(params, "x", &x_object)) {
    json_object *min_object;
    float min;
    int clip_min;
    if(json_object_object_get_ex(x_object, "min", &min_object)) {
      if(min_object == NULL) {
        // If min is set to null, it means we should disable min clipping. We
        // keep set the value the current value.
        min = clipinfo.xmin;
        clip_min = 0;
      }
      else {
        // Otherwise we turn on clipping and set the value.
        min = json_object_get_double(min_object);
        clip_min = 1;
      }
      SetSceneclipXMin(clip_min, min);
    }

    json_object *max_object;
    float max;
    int clip_max;
    if(json_object_object_get_ex(x_object, "max", &max_object)) {
      if(max_object == NULL) {
        // If min is set to null, it means we should disable min clipping. We
        // keep set the value the current value.
        max = clipinfo.xmax;
        clip_max = 0;
      }
      else {
        // Otherwise we turn on clipping and set the value.
        max = json_object_get_double(max_object);
        clip_max = 1;
      }
      SetSceneclipXMax(clip_max, max);
    }
  }

  json_object *y_object;
  if(json_object_object_get_ex(params, "y", &y_object)) {
    json_object *min_object;
    float min;
    int clip_min;
    if(json_object_object_get_ex(y_object, "min", &min_object)) {
      if(min_object == NULL) {
        // If min is set to null, it means we should disable min clipping. We
        // keep set the value the current value.
        min = clipinfo.ymin;
        clip_min = 0;
      }
      else {
        // Otherwise we turn on clipping and set the value.
        min = json_object_get_double(min_object);
        clip_min = 1;
      }
      SetSceneclipYMin(clip_min, min);
    }

    json_object *max_object;
    float max;
    int clip_max;
    if(json_object_object_get_ex(y_object, "max", &max_object)) {
      if(max_object == NULL) {
        // If min is set to null, it means we should disable min clipping. We
        // keep set the value the current value.
        max = clipinfo.ymax;
        clip_max = 0;
      }
      else {
        // Otherwise we turn on clipping and set the value.
        max = json_object_get_double(max_object);
        clip_max = 1;
      }
      SetSceneclipYMax(clip_max, max);
    }
  }

  json_object *z_object;
  if(json_object_object_get_ex(params, "z", &z_object)) {
    json_object *min_object;
    float min;
    int clip_min;
    if(json_object_object_get_ex(z_object, "min", &min_object)) {
      if(min_object == NULL) {
        // If min is set to null, it means we should disable min clipping. We
        // keep set the value the current value.
        min = clipinfo.zmin;
        clip_min = 0;
      }
      else {
        // Otherwise we turn on clipping and set the value.
        min = json_object_get_double(min_object);
        clip_min = 1;
      }
      SetSceneclipZMin(clip_min, min);
    }

    json_object *max_object;
    float max;
    int clip_max;
    if(json_object_object_get_ex(z_object, "max", &max_object)) {
      if(max_object == NULL) {
        // If min is set to null, it means we should disable min clipping. We
        // keep set the value the current value.
        max = clipinfo.zmax;
        clip_max = 0;
      }
      else {
        // Otherwise we turn on clipping and set the value.
        max = json_object_get_double(max_object);
        clip_max = 1;
      }
      SetSceneclipZMax(clip_max, max);
    }
  }

  return 0;
}

/// @brief Build a Lua table with information on the meshes of the model. The
/// key of the table is the mesh number.
json_object *jsonrpc_GetMeshes(jrpc_context *context, json_object *params,
                               json_object *id) {
  struct json_object *mesh_array = json_object_new_array();
  for(int i = 0; i < global_scase.meshescoll.nmeshes; i++) {
    meshdata *mesh = &global_scase.meshescoll.meshinfo[i];
    struct json_object *mesh_obj = json_object_new_object();
    json_object_object_add(mesh_obj, "index", json_object_new_int(i + 1));
    if(mesh->label != NULL) {
      json_object_object_add(mesh_obj, "id",
                             json_object_new_string(mesh->label));
    }
    struct json_object *mesh_coordinates = json_object_new_object();
    json_object_object_add(mesh_coordinates, "i",
                           json_object_new_int(mesh->ibar));
    json_object_object_add(mesh_coordinates, "j",
                           json_object_new_int(mesh->jbar));
    json_object_object_add(mesh_coordinates, "k",
                           json_object_new_int(mesh->kbar));
    json_object_object_add(mesh_obj, "coordinates", mesh_coordinates);
    struct json_object *mesh_dimensions = json_object_new_object();
    json_object_object_add(mesh_dimensions, "x_min",
                           json_object_new_double(mesh->boxmin_fds[0]));
    json_object_object_add(mesh_dimensions, "x_max",
                           json_object_new_double(mesh->boxmax_fds[0]));
    json_object_object_add(mesh_dimensions, "y_min",
                           json_object_new_double(mesh->boxmin_fds[1]));
    json_object_object_add(mesh_dimensions, "y_max",
                           json_object_new_double(mesh->boxmax_fds[1]));
    json_object_object_add(mesh_dimensions, "z_min",
                           json_object_new_double(mesh->boxmin_fds[2]));
    json_object_object_add(mesh_dimensions, "z_max",
                           json_object_new_double(mesh->boxmax_fds[2]));
    json_object_object_add(mesh_obj, "dimensions", mesh_dimensions);

    struct json_object *vents = json_object_new_array();
    for(int j = 0; j < mesh->nvents; j++) {
      ventdata *vent = &mesh->ventinfo[j];
      struct json_object *vent_obj = json_object_new_object();
      json_object_object_add(vent_obj, "index", json_object_new_int(j + 1));
      // json_object_object_add(vent_obj, "filename",
      //                        json_object_new_string(csv_file->file));
      // json_object_object_add(vent_obj, "type",
      //                        json_object_new_string(csv_file->c_type));
      json_object_object_add(
          vent_obj, "surface_id",
          json_object_new_string(vent->surf[0]->surfacelabel));
      struct json_object *dimensions = json_object_new_object();
      json_object_object_add(dimensions, "x_min",
                             json_object_new_double(vent->xmin));
      json_object_object_add(dimensions, "x_max",
                             json_object_new_double(vent->xmax));
      json_object_object_add(dimensions, "y_min",
                             json_object_new_double(vent->ymin));
      json_object_object_add(dimensions, "y_max",
                             json_object_new_double(vent->ymax));
      json_object_object_add(dimensions, "z_min",
                             json_object_new_double(vent->zmin));
      json_object_object_add(dimensions, "z_max",
                             json_object_new_double(vent->zmax));
      json_object_object_add(vent_obj, "dimensions", dimensions);

      struct json_object *coordinates = json_object_new_object();
      json_object_object_add(coordinates, "i_min",
                             json_object_new_int(vent->imin));
      json_object_object_add(coordinates, "i_max",
                             json_object_new_int(vent->imax));
      json_object_object_add(coordinates, "j_min",
                             json_object_new_int(vent->jmin));
      json_object_object_add(coordinates, "j_max",
                             json_object_new_int(vent->jmax));
      json_object_object_add(coordinates, "k_min",
                             json_object_new_int(vent->kmin));
      json_object_object_add(coordinates, "k_max",
                             json_object_new_int(vent->kmax));
      json_object_object_add(vent_obj, "coordinates", coordinates);
      json_object_array_add(vents, vent_obj);
    }
    json_object_object_add(mesh_obj, "vents", vents);

    struct json_object *xplt_orig = json_object_new_array();
    for(size_t j = 0; j < mesh->ibar; j++) {
      json_object_array_add(xplt_orig,
                            json_object_new_double(mesh->xplt_fds[j]));
    }
    json_object_object_add(mesh_obj, "xplt_orig", xplt_orig);

    struct json_object *yplt_orig = json_object_new_array();
    for(size_t j = 0; j < mesh->jbar; j++) {
      json_object_array_add(yplt_orig,
                            json_object_new_double(mesh->xplt_fds[j]));
    }
    json_object_object_add(mesh_obj, "yplt_orig", yplt_orig);

    struct json_object *zplt_orig = json_object_new_array();
    for(size_t j = 0; j < mesh->kbar; j++) {
      json_object_array_add(zplt_orig,
                            json_object_new_double(mesh->xplt_fds[j]));
    }
    json_object_object_add(mesh_obj, "zplt_orig", zplt_orig);

    json_object_array_add(mesh_array, mesh_obj);
  }
  return mesh_array;
}

json_object *json_GetSmoke3ds(jrpc_context *context, json_object *params,
                              json_object *id) {
  struct json_object *smoke3ds = json_object_new_array();
  for(int i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++) {
    smoke3ddata *val = &global_scase.smoke3dcoll.smoke3dinfo[i];
    struct json_object *slice_obj = json_object_new_object();
    json_object_object_add(slice_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(slice_obj, "mesh",
                           json_object_new_int(val->blocknumber));
    if(val->label.longlabel != NULL) {
      json_object_object_add(slice_obj, "longlabel",
                             json_object_new_string(val->label.longlabel));
    }
    if(val->label.shortlabel) {
      json_object_object_add(slice_obj, "shortlabel",
                             json_object_new_string(val->label.shortlabel));
    }
    json_object_object_add(slice_obj, "type", json_object_new_int(val->type));
    json_object_array_add(smoke3ds, slice_obj);
  }
  return smoke3ds;
}

json_object *json_GetSlices(void) {
  struct json_object *slices = json_object_new_array();
  for(int i = 0; i < global_scase.slicecoll.nsliceinfo; i++) {
    slicedata *slice = &global_scase.slicecoll.sliceinfo[i];
    struct json_object *slice_obj = json_object_new_object();
    json_object_object_add(slice_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(slice_obj, "mesh",
                           json_object_new_int(slice->blocknumber));
    if(slice->label.longlabel != NULL) {
      json_object_object_add(slice_obj, "longlabel",
                             json_object_new_string(slice->label.longlabel));
    }
    if(slice->label.shortlabel) {
      json_object_object_add(slice_obj, "shortlabel",
                             json_object_new_string(slice->label.shortlabel));
    }
    if(slice->slicelabel) {
      json_object_object_add(slice_obj, "id",
                             json_object_new_string(slice->slicelabel));
    }
    json_object_object_add(slice_obj, "idir", json_object_new_int(slice->idir));
    json_object_object_add(slice_obj, "position_orig",
                           json_object_new_double(slice->position_orig));
    if(slice->label.unit) {
      json_object_object_add(slice_obj, "unit",
                             json_object_new_string(slice->label.unit));
    }
    struct json_object *coordinates = json_object_new_object();
    json_object_object_add(coordinates, "i_min",
                           json_object_new_int(slice->ijk_min[0]));
    json_object_object_add(coordinates, "i_max",
                           json_object_new_int(slice->ijk_max[0]));
    json_object_object_add(coordinates, "j_min",
                           json_object_new_int(slice->ijk_min[1]));
    json_object_object_add(coordinates, "j_max",
                           json_object_new_int(slice->ijk_max[1]));
    json_object_object_add(coordinates, "k_min",
                           json_object_new_int(slice->ijk_min[2]));
    json_object_object_add(coordinates, "k_max",
                           json_object_new_int(slice->ijk_max[2]));
    json_object_object_add(slice_obj, "coordinates", coordinates);

    json_object_array_add(slices, slice_obj);
  }
  return slices;
}

json_object *jsonrpc_GetSlices(jrpc_context *context, json_object *params,
                               json_object *id) {
  return json_GetSlices();
}

json_object *jsonrpc_GetCsvs(jrpc_context *context, json_object *params,
                             json_object *id) {
  struct json_object *csv_files = json_object_new_array();
  for(int i = 0; i < global_scase.csvcoll.ncsvfileinfo; i++) {
    csvfiledata *csv_file = &global_scase.csvcoll.csvfileinfo[i];
    struct json_object *csv_obj = json_object_new_object();
    json_object_object_add(csv_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(csv_obj, "filename",
                           json_object_new_string(csv_file->file));
    json_object_object_add(csv_obj, "type",
                           json_object_new_string(csv_file->c_type));
    json_object_array_add(csv_files, csv_obj);
  }
  return csv_files;
}

json_object *CreateVectorJson(csvdata *csv_x, csvdata *csv_y) {
  struct json_object *csv_obj = json_object_new_object();
  json_object_object_add(csv_obj, "name",
                         json_object_new_string(csv_y->label.longlabel));
  struct json_object *x_obj = json_object_new_object();
  json_object_object_add(x_obj, "name",
                         json_object_new_string(csv_x->label.longlabel));
  json_object_object_add(x_obj, "units",
                         json_object_new_string(csv_x->label.unit));
  json_object_object_add(csv_obj, "x", x_obj);
  struct json_object *y_obj = json_object_new_object();
  json_object_object_add(y_obj, "name",
                         json_object_new_string(csv_y->label.longlabel));
  json_object_object_add(y_obj, "units",
                         json_object_new_string(csv_y->label.unit));
  json_object_object_add(csv_obj, "y", y_obj);
  struct json_object *values = json_object_new_array();
  for(int i = 0; i < csv_x->nvals; ++i) {
    struct json_object *p_obj = json_object_new_object();
    json_object_object_add(p_obj, "x", json_object_new_double(csv_x->vals[i]));
    json_object_object_add(p_obj, "y", json_object_new_double(csv_y->vals[i]));
    json_object_array_add(values, p_obj);
  }
  json_object_object_add(csv_obj, "values", values);
  return csv_obj;
}

void LoadCsv(csvfiledata *csventry);

json_object *jsonrpc_GetCsvVectors(jrpc_context *context, json_object *params,
                                   json_object *id) {
  struct json_object *csv_files = json_object_new_object();

  for(int i = 0; i < global_scase.csvcoll.ncsvfileinfo; ++i) {
    struct json_object *csvs_obj = json_object_new_object();
    json_object_object_add(
        csv_files, global_scase.csvcoll.csvfileinfo[i].c_type, csvs_obj);
    csvfiledata *csventry = &global_scase.csvcoll.csvfileinfo[i];
    if(!csventry->loaded) {
      LoadCsv(csventry);
    }
    for(size_t j = 0; j < csventry->ncsvinfo; j++) {
      json_object *vector =
          CreateVectorJson(csventry->time, &(csventry->csvinfo[j]));
      json_object_object_add(csvs_obj, csventry->csvinfo[j].label.longlabel,
                             vector);
    }
  }
  return csv_files;
}

/// @brief Load a slice based on its index in sliceinfo.
json_object *jsonrpc_LoadSliceIndices(jrpc_context *context,
                                      json_object *params, json_object *id) {
  int errorcode = 0;
  size_t n_files = json_object_array_length(params);
  for(size_t n = 0; n < n_files; n++) {
    int i = json_object_get_int(json_object_array_get_idx(params, n));
    Loadsliceindex(i, ALL_FRAMES, &errorcode);
    if(errorcode) {
      context->error_code = 115;
      context->error_message = strdup("could not load slice");
      return NULL;
    }
  }
  return NULL;
}

json_object *jsonrpc_LoadSlices(jrpc_context *context, json_object *params,
                                json_object *id) {
  int errorcode = 0;
  json_object *specs;
  // specs: { index: number; frame?: number }[]
  int res = json_object_object_get_ex(params, "specs", &specs);
  size_t n_files = json_object_array_length(specs);
  const char *json_output =
      json_object_to_json_string_ext(params, JSON_C_TO_STRING_PRETTY);
  fprintf(stderr, "%s\n", json_output);
  for(size_t n = 0; n < n_files; n++) {
    json_object *spec = json_object_array_get_idx(specs, n);
    json_object *index_obj;
    int r2 = json_object_object_get_ex(spec, "index", &index_obj);
    int index = json_object_get_int(index_obj);
    json_object_put(index_obj);
    int frame = ALL_FRAMES;
    json_object *frame_obj;
    if(json_object_object_get_ex(spec, "frame", &frame_obj)) {
      frame = json_object_get_int(frame_obj);
      json_object_put(frame_obj);
    }
    json_object_put(spec);
    Loadsliceindex(index, frame, &errorcode);
    if(errorcode) {
      context->error_code = 115;
      context->error_message = strdup("could not load slice");
      return NULL;
    }
  }
  return NULL;
}

json_object *jsonrpc_Load3dSmokeIndices(jrpc_context *context,
                                        json_object *params, json_object *id) {
  int errorcode = 0;
  size_t n_files = json_object_array_length(params);
  for(size_t n = 0; n < n_files; n++) {
    int i = json_object_get_int(json_object_array_get_idx(params, n));
    fprintf(stderr, "loading smoke3d index %d\n", i);
    smoke3ddata *smoke3di;
    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    smoke3di->finalize = 0;
    if(n == (n_files - 1)) smoke3di->finalize = 1;
    ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
    if(errorcode) {
      context->error_code = 117;
      context->error_message = strdup("failed to load smoke3d");
    }
  }
  force_redisplay = 1;
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  return NULL;
}

/// @brief Set the format of images which will be exported. The value should be
/// a string. The acceptable values are:
///   "JPG"
///   "PNG"
json_object *jsonrpc_SetRendertype(jrpc_context *context, json_object *params,
                                   json_object *id) {
  const char *type =
      json_object_get_string(json_object_array_get_idx(params, 0));
  SetRendertype(type);
  return NULL;
}

json_object *jsonrpc_GetRendertype(jrpc_context *context, json_object *params,
                                   json_object *id) {
  json_object *result_root = NULL;
  int render_type = GetRendertype();
  switch(render_type) {
  case JPEG:
    result_root = json_object_new_string("JPG");
    break;
  case PNG:
    result_root = json_object_new_string("PNG");
    break;
  }
  return result_root;
}

/// @brief Set the format of movies which will be exported. The value should be
/// a string. The acceptable values are:
///    - "WMV"
///    - "MP4"
///    - "AVI"
json_object *jsonrpc_SetMovietype(jrpc_context *context, json_object *params,
                                  json_object *id) {
  const char *type =
      json_object_get_string(json_object_array_get_idx(params, 0));
  SetMovietype(type);
  return 0;
}

json_object *jsonrpc_GetMovietype(jrpc_context *context, json_object *params,
                                  json_object *id) {
  json_object *result_root = NULL;
  int movie_type = GetMovietype();
  switch(movie_type) {
  case WMV:
    result_root = json_object_new_string("WMV");
    break;
  case MP4:
    result_root = json_object_new_string("MP4");
    break;
  case AVI:
    result_root = json_object_new_string("AVI");
    break;
  }
  return result_root;
}

json_object *jsonrpc_Unloadall(jrpc_context *context, json_object *params,
                               json_object *id) {
  Unloadall();
  return NULL;
}

json_object *jsonrpc_Setrenderdir(jrpc_context *context, json_object *params,
                                  json_object *id) {
  const char *dir =
      json_object_get_string(json_object_array_get_idx(params, 0));
  if(Setrenderdir(dir)) {
    context->error_code = 112;
    context->error_message = strdup("set render dir failure");
  }
  return NULL;
}

json_object *jsonrpc_Getrenderdir(jrpc_context *context, json_object *params,
                                  json_object *id) {
  return json_object_new_string(script_dir_path);
}

json_object *jsonrpc_SetOrthoPreset(jrpc_context *context, json_object *params,
                                    json_object *id) {
  GLUIUpdateTranslate();
  DisplayCB();
  const char *viewpoint =
      json_object_get_string(json_object_array_get_idx(params, 0));
  if(SetOrthoPreset(viewpoint)) {
    context->error_code = 119;
    context->error_message = strdup("SetOrthoPreset failure");
  }
  DisplayCB();
  return NULL;
}

json_object *jsonrpc_Setviewpoint(jrpc_context *context, json_object *params,
                                  json_object *id) {
  const char *viewpoint =
      json_object_get_string(json_object_array_get_idx(params, 0));
  if(Setviewpoint(viewpoint)) {
    context->error_code = 113;
    context->error_message = strdup("set viewpoint failure");
  }
  return NULL;
}

json_object *jsonrpc_Getviewpoint(jrpc_context *context, json_object *params,
                                  json_object *id) {
  return json_object_new_string(camera_current->name);
}

json_object *jsonrpc_ExitSmokeview(jrpc_context *context, json_object *params,
                                   json_object *id) {
  // TODO: we need to return response before exiting
  ExitSmokeview();
  return NULL;
}

json_object *jsonrpc_SetWindowSize(jrpc_context *context, json_object *params,
                                   json_object *id) {
  // TODO: use the _ex API
  int width = json_object_get_int(json_object_object_get(params, "width"));
  int height = json_object_get_int(json_object_object_get(params, "height"));
  Setwindowsize(width, height);
  return NULL;
}

json_object *jsonrpc_SetColorbarFlip(jrpc_context *context, json_object *params,
                                     json_object *id) {
  json_bool flip =
      json_object_get_boolean(json_object_array_get_idx(params, 0));
  Setcolorbarflip(flip ? 1 : 0);
  return NULL;
}

json_object *jsonrpc_Getcolorbarflip(jrpc_context *context, json_object *params,
                                     json_object *id) {
  int flip = Getcolorbarflip();
  if(flip) {
    return json_object_new_boolean(1);
  }
  else {
    return json_object_new_boolean(0);
  }
}

json_object *jsonrpc_SetNamedColorbar(jrpc_context *context,
                                      json_object *params, json_object *id) {
  const char *name =
      json_object_get_string(json_object_array_get_idx(params, 0));
  int err = SetNamedColorbar(name);
  if(err == 1) {
    context->error_code = 125;
    context->error_message = strdup("not a valid colorbar name");
  }
  return 0;
}

// title
json_object *jsonrpc_SetTitleVisibility(jrpc_context *context,
                                        json_object *params, json_object *id) {
  json_bool v = json_object_get_boolean(json_object_array_get_idx(params, 0));
  SetTitleVisibility(v ? 1 : 0);
  return NULL;
}

json_object *jsonrpc_GetTitleVisibility(jrpc_context *context,
                                        json_object *params, json_object *id) {
  int v = GetTitleVisibility();
  if(v) {
    return json_object_new_boolean(1);
  }
  else {
    return json_object_new_boolean(0);
  }
  return NULL;
}

// smv_version
json_object *jsonrpc_SetSmvVersionVisibility(jrpc_context *context,
                                             json_object *params,
                                             json_object *id) {
  json_bool v = json_object_get_boolean(json_object_array_get_idx(params, 0));
  SetSmvVersionVisibility(v ? 1 : 0);
  return NULL;
}

json_object *jsonrpc_GetSmvVersionVisibility(jrpc_context *context,
                                             json_object *params,
                                             json_object *id) {
  int v = GetSmvVersionVisibility();
  if(v) {
    return json_object_new_boolean(1);
  }
  else {
    return json_object_new_boolean(0);
  }
}

// #define JsonRpcBoolSetting(name,func)

json_object *jsonrpc_SetChidVisibility(jrpc_context *context,
                                       json_object *params, json_object *id) {
  json_bool v = json_object_get_boolean(json_object_array_get_idx(params, 0));
  SetChidVisibility(v ? 1 : 0);
  return NULL;
}

json_object *jsonrpc_GetChidVisibility(jrpc_context *context,
                                       json_object *params, json_object *id) {
  int v = GetChidVisibility();
  if(v) {
    return json_object_new_boolean(1);
  }
  else {
    return json_object_new_boolean(0);
  }
}

json_object *jsonrpc_CameraSetProjectionType(jrpc_context *context,
                                             json_object *params,
                                             json_object *id) {
  if(!json_object_is_type(params, json_type_array)) {
    context->error_code = JRPC_INVALID_PARAMS;
    context->error_message = strdup("expected an array");
    return NULL;
  }
  int projection_type_local =
      json_object_get_int(json_object_array_get_idx(params, 0));
  if(CameraSetProjectionType(projection_type_local)) {
    context->error_code = 121;
    context->error_message = strdup("set proejction type failure");
  }
  return NULL;
}

json_object *jsonrpc_SetSliceBounds(jrpc_context *context, json_object *params,
                                    json_object *id) {
  const char *slice_type =
      json_object_get_string(json_object_object_get(params, "type"));
  int set_min = json_object_get_int(json_object_object_get(params, "set_min"));
  float value_min =
      json_object_get_double(json_object_object_get(params, "value_min"));
  int set_max = json_object_get_int(json_object_object_get(params, "set_max"));
  float value_max =
      json_object_get_double(json_object_object_get(params, "value_max"));
  fprintf(stderr, "setting slice_bounds: %s %d, %g, %d, %g\n", slice_type,
          set_min, value_min, set_max, value_max);
  CApiSetSliceBounds(slice_type, set_min, value_min, set_max, value_max);
  return NULL;
}

json_object *jsonrpc_SetFontSize(jrpc_context *context, json_object *params,
                                 json_object *id) {
  json_object *arg0 = json_object_array_get_idx(params, 0);
  if(json_object_is_type(arg0, json_type_string)) {
    const char *size_name =
        json_object_get_string(json_object_array_get_idx(params, 0));
    if(strcmp(size_name, "small") == 0) {
      SetFontsize(0);
    }
    else if(strcmp(size_name, "large") == 0) {
      SetFontsize(1);
    }
    else {
      context->error_code = 134;
      context->error_message = strdup("invalid font size");
    }
  }
  else if(json_object_is_type(arg0, json_type_int)) {
    int height2d = json_object_get_int(arg0);
    if(SetScaledfontHeight2d(height2d)) {
      context->error_code = 133;
      context->error_message = strdup("could not set font size");
    }
  }
  return NULL;
}

json_object *jsonrpc_BlockagesHideAll(jrpc_context *context,
                                      json_object *params, json_object *id) {
  BlockagesHideAll();
  return NULL;
}

json_object *jsonrpc_SurfacesHideAll(jrpc_context *context, json_object *params,
                                     json_object *id) {
  SurfacesHideAll();
  return NULL;
}

json_object *jsonrpc_OutlinesHide(jrpc_context *context, json_object *params,
                                  json_object *id) {
  OutlinesHide();
  return NULL;
}

json_object *jsonrpc_DevicesHideAll(jrpc_context *context, json_object *params,
                                    json_object *id) {
  DevicesHideAll();
  return NULL;
}

int register_procedures(struct jrpc_server *server_arg) {
  jrpc_register_procedure(server_arg, &jsonrpc_SetFrame, "set_frame", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetTime, "set_time", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetCameraAz, "set_camera_az",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetCameraElev, "set_camera_elev",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetCameraEye, "set_camera_eye",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetCameraZoom, "set_camera_zoom",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetCameraViewDir,
                          "set_camera_view_dir", NULL);

  jrpc_register_procedure(server_arg, &jsonrpc_GetNGlobalTimes,
                          "get_n_global_times", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_GetTime, "get_time", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetClipping, "set_clipping",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_Render, "render", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_Unloadall, "unload_all", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetChidVisibility,
                          "set_chid_visibility", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_ExitSmokeview, "exit", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetSliceBounds,
                          "set_slice_bounds", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetWindowSize, "set_window_size",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_LoadSliceIndices,
                          "load_slice_indices", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_LoadSlices, "load_slices", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_GetSlices, "get_slices", NULL);
  jrpc_register_procedure(server_arg, &json_GetSmoke3ds, "get_smoke3ds", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_Load3dSmokeIndices,
                          "load_smoke3d_indices", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetOrthoPreset,
                          "set_ortho_preset", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_GetMeshes, "get_meshes", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_CameraSetProjectionType,
                          "set_projection_type", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetNamedColorbar, "set_colorbar",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetRendertype, "set_render_type",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_Setrenderdir, "set_render_dir",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetFontSize, "set_font_size",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetTitleVisibility,
                          "set_title_visibility", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_GetTitleVisibility,
                          "get_title_visibility", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetSmvVersionVisibility,
                          "set_smv_version_visibility", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_GetSmvVersionVisibility,
                          "get_smv_version_visibility", NULL);

  jrpc_register_procedure(server_arg, &jsonrpc_BlockagesHideAll,
                          "blockages_hide_all", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SurfacesHideAll,
                          "surfaces_hide_all", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_OutlinesHide,
                          "outlines_hide_all", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_DevicesHideAll,
                          "devices_hide_all", NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_GetCsvVectors, "get_csv_vectors",
                          NULL);
  jrpc_register_procedure(server_arg, &jsonrpc_SetColorbarFlip,
                          "set_colorbar_flip", NULL);

  return 0;
}

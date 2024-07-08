
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "options.h"

#include "smokeviewvars.h"

#include "infoheader.h"

#include "c_api.h"

#include "jsonrpc.h"

#include GLUT_H
#include "gd.h"

#if defined(_WIN32)
#include <direct.h>
#endif

// // NOLINTNEXTLINE
// jrpc_context *context, json_object *params,  json_object *id;
// json_object *jsonrpc_DisplayCb(jrpc_context *context, json_object *params,
// json_object *id);

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
  if (ret) {
    context->error_code = 111;
    context->error_message = strdup("render failure");
  }
  return NULL;
}

// /// @brief Returns an error code then the image data.
// json_object *jsonrpc_RenderVar(jrpc_context *context, json_object *params,
// json_object *id) {
//   gdImagePtr rende_rimage;
//   int return_code;
//   char *image_data;
//   int image_size;

//   // render image to RENDERimage gd buffer
//   return_code = RenderFrameLuaVar(VIEW_CENTER, &rende_rimage);
//   lua_pushnumber(L, return_code);
//   // convert to a simpler byte-buffer
//   image_data = gdImagePngPtr(rende_rimage, &image_size);
//   // push to stack
//   lua_pushlstring(L, image_data, image_size);
//   // destroy C copy
//   gdImageDestroy(rende_rimage);

//   return 2;
// }

// /// @brief Trigger the display callback.
// json_object *jsonrpc_DisplayCb(jrpc_context *context, json_object *params,
// json_object *id) {
//   // runluascript=0;
//   DisplayCB();
//   // runluascript=1;
//   return 0;
// }

// /// @brief Hide the smokeview window. This should not currently be used as it
// /// prevents the display callback being called, and therefore the script will
// /// not continue (the script is called as part of the display callback).
// json_object *jsonrpc_Hidewindow(jrpc_context *context, json_object *params,
// json_object *id) {
//   glutHideWindow();
//   // once we hide the window the display callback is never called
//   return 0;
// }

/// @brief Return the current frame number which Smokeivew has loaded.
json_object *jsonrpc_Getframe(jrpc_context *context, json_object *params,
                              json_object *id) {
  int framenumber = Getframe();
  json_object *result_root = json_object_new_int(framenumber);
  return result_root;
}

/// @brief Shift to a specific frame number.
json_object *jsonrpc_Setframe(jrpc_context *context, json_object *params,
                              json_object *id) {
  int f = json_object_get_int(json_object_array_get_idx(params, 0));
  fprintf(stderr, "setting frame to %d\n", f);
  Setframe(f);
  return NULL;
}

/// @brief Get the time value of the currently loaded frame.
json_object *jsonrpc_Gettime(jrpc_context *context, json_object *params,
                             json_object *id) {
  if (global_times != NULL && nglobal_times > 0) {
    float time = Gettime();
    json_object *result_root = json_object_new_double(time);
    return result_root;
  }
  else {
    return NULL;
  }
}

/// @brief Shift to the closest frame to given a time value.
json_object *jsonrpc_Settime(jrpc_context *context, json_object *params,
                             json_object *id) {
  DisplayCB();
  float t = json_object_get_double(json_object_array_get_idx(params, 0));
  int return_code = Settime(t);
  if (return_code) {
    context->error_code = 112;
    context->error_message = strdup("render failure");
  }
  return NULL;
}

// /// @brief Load an FDS vector data file directly (i.e. as a filepath). This
// /// function handles the loading of any additional data files necessary to
// /// display vectors.
// json_object *jsonrpc_Loadvdatafile(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *filename = lua_tostring(L, 1);
//   int return_value = Loadvfile(filename);
//   lua_pushnumber(L, return_value);
//   return 1;
// }

// /// @brief Load an FDS boundary file directly (i.e. as a filepath). This is
// /// equivalent to lua_loadfile, but specialised for boundary files. This is
// /// included to reflect the underlying code.
// json_object *jsonrpc_Loadboundaryfile(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *filename = lua_tostring(L, 1);
//   Loadboundaryfile(filename);
//   return 0;
// }

json_object *jsonrpc_GetClipping(jrpc_context *context, json_object *params,
                                 json_object *id) {
  json_object *result_root = json_object_new_object();
  json_object_object_add(result_root, "mode",
                         json_object_new_int(GetClippingMode()));
  json_object *x_object = json_object_new_object();
  if (clipinfo.clip_xmin) {
    json_object_object_add(x_object, "min",
                           json_object_new_double(clipinfo.xmin));
  }
  if (clipinfo.clip_xmax) {
    json_object_object_add(x_object, "max",
                           json_object_new_double(clipinfo.xmax));
  }
  json_object_object_add(result_root, "x", x_object);
  json_object *y_object = json_object_new_object();
  if (clipinfo.clip_ymin) {
    json_object_object_add(x_object, "min",
                           json_object_new_double(clipinfo.ymin));
  }
  if (clipinfo.clip_ymax) {
    json_object_object_add(x_object, "max",
                           json_object_new_double(clipinfo.ymax));
  }
  json_object_object_add(result_root, "y", y_object);
  json_object *z_object = json_object_new_object();
  if (clipinfo.clip_zmin) {
    json_object_object_add(x_object, "min",
                           json_object_new_double(clipinfo.zmin));
  }
  if (clipinfo.clip_zmax) {
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
  if (json_object_object_get_ex(params, "mode", &mode_object)) {
    int mode = json_object_get_int(mode_object);
    SetClippingMode(mode);
  }

  json_object *x_object;
  if (json_object_object_get_ex(params, "x", &x_object)) {
    json_object *min_object;
    float min;
    int clip_min;
    if (json_object_object_get_ex(x_object, "min", &min_object)) {
      if (min_object == NULL) {
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
    if (json_object_object_get_ex(x_object, "max", &max_object)) {
      if (max_object == NULL) {
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
  if (json_object_object_get_ex(params, "y", &y_object)) {
    json_object *min_object;
    float min;
    int clip_min;
    if (json_object_object_get_ex(y_object, "min", &min_object)) {
      if (min_object == NULL) {
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
    if (json_object_object_get_ex(y_object, "max", &max_object)) {
      if (max_object == NULL) {
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
  if (json_object_object_get_ex(params, "z", &z_object)) {
    json_object *min_object;
    float min;
    int clip_min;
    if (json_object_object_get_ex(z_object, "min", &min_object)) {
      if (min_object == NULL) {
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
    if (json_object_object_get_ex(z_object, "max", &max_object)) {
      if (max_object == NULL) {
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

// /// @brief Return a table (an array) of the times available in Smokeview.
// They
// /// key of the table is an int representing the frame number, and the value
// of
// /// the table is a float representing the time.
// /// @param L The lua interpreter
// /// @return Number of stack items left on stack.
// json_object *jsonrpc_GetGlobalTimes(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_createtable(L, 0, nglobal_times);
//   int i;
//   for (i = 0; i < nglobal_times; i++) {
//     lua_pushnumber(L, i);
//     lua_pushnumber(L, global_times[i]);
//     lua_settable(L, -3);
//   }
//   return 1;
// }

// /// @brief Given a frame number return the time.
// /// @param L The lua interpreter
// /// @return Number of stack items left on stack.
// json_object *jsonrpc_GetGlobalTime(jrpc_context *context, json_object
// *params, json_object *id) {
//   int frame_number = lua_tonumber(L, 1);
//   if (frame_number >= 0 && frame_number < nglobal_times) {
//     lua_pushnumber(L, global_times[frame_number]);
//   }
//   else {
//     lua_pushnil(L);
//   }
//   return 1;
// }

// /// @brief Get the number of (global) frames available to smokeview.
// /// @param L
// /// @return
// json_object *jsonrpc_GetNglobalTimes(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_pushnumber(L, nglobal_times);
//   return 1;
// }

// /// @brief Get the number of meshes in the loaded model.
// json_object *jsonrpc_GetNmeshes(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushnumber(L, nmeshes);
//   return 1;
// }

// /// @brief Get the number of particle files in the loaded model.
// json_object *jsonrpc_GetNpartinfo(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushnumber(L, npartinfo);
//   return 1;
// }

// /// @brief Build a Lua table with information on the meshes of the model. The
// /// key of the table is the mesh number.
// // TODO: provide more information via this interface.
// json_object *jsonrpc_GetMeshes(jrpc_context *context, json_object *params,
// json_object *id) {
//   int entries = nmeshes;
//   meshdata *infotable = meshinfo;
//   lua_createtable(L, 0, entries);
//   int i;
//   for (i = 0; i < entries; i++) {
//     lua_pushnumber(L, i);
//     lua_createtable(L, 0, 5);

//     lua_pushnumber(L, infotable[i].ibar);
//     lua_setfield(L, -2, "ibar");

//     lua_pushnumber(L, infotable[i].jbar);
//     lua_setfield(L, -2, "jbar");

//     lua_pushnumber(L, infotable[i].kbar);
//     lua_setfield(L, -2, "kbar");

//     lua_pushstring(L, infotable[i].label);
//     lua_setfield(L, -2, "label");

//     lua_pushnumber(L, infotable[i].kbar);
//     lua_setfield(L, -2, "cellsize");

//     lua_pushnumber(L, xbar0);
//     lua_setfield(L, -2, "xbar0");

//     lua_pushnumber(L, ybar0);
//     lua_setfield(L, -2, "ybar0");

//     lua_pushnumber(L, zbar0);
//     lua_setfield(L, -2, "zbar0");

//     lua_pushnumber(L, xyzmaxdiff);
//     lua_setfield(L, -2, "xyzmaxdiff");

//     lua_pushnumber(L, i);
//     lua_pushcclosure(L, LuaGetiblankcell, 1);
//     lua_setfield(L, -2, "iblank_cell");

//     lua_pushnumber(L, i);
//     lua_pushcclosure(L, LuaGetiblanknode, 1);
//     lua_setfield(L, -2, "iblank_node");

//     // loop for less than ibar
//     int j;
//     lua_createtable(L, 0, infotable[i].ibar);
//     for (j = 0; j < infotable[i].ibar; j++) {
//       lua_pushnumber(L, j);
//       lua_pushnumber(L, infotable[i].xplt[j]);
//       lua_settable(L, -3);
//     }
//     lua_setfield(L, -2, "xplt");

//     lua_createtable(L, 0, infotable[i].jbar);
//     for (j = 0; j < infotable[i].jbar; j++) {
//       lua_pushnumber(L, j);
//       lua_pushnumber(L, infotable[i].yplt[j]);
//       lua_settable(L, -3);
//     }
//     lua_setfield(L, -2, "yplt");

//     lua_createtable(L, 0, infotable[i].kbar);
//     for (j = 0; j < infotable[i].kbar; j++) {
//       lua_pushnumber(L, j);
//       lua_pushnumber(L, infotable[i].zplt[j]);
//       lua_settable(L, -3);
//     }
//     lua_setfield(L, -2, "zplt");

//     lua_createtable(L, 0, infotable[i].ibar);
//     for (j = 0; j < infotable[i].ibar; j++) {
//       lua_pushnumber(L, j);
//       lua_pushnumber(L, infotable[i].xplt_orig[j]);
//       lua_settable(L, -3);
//     }
//     lua_setfield(L, -2, "xplt_orig");

//     lua_createtable(L, 0, infotable[i].jbar);
//     for (j = 0; j < infotable[i].jbar; j++) {
//       lua_pushnumber(L, j);
//       lua_pushnumber(L, infotable[i].yplt_orig[j]);
//       lua_settable(L, -3);
//     }
//     lua_setfield(L, -2, "yplt_orig");

//     lua_createtable(L, 0, infotable[i].kbar);
//     for (j = 0; j < infotable[i].kbar; j++) {
//       lua_pushnumber(L, j);
//       lua_pushnumber(L, infotable[i].zplt_orig[j]);
//       lua_settable(L, -3);
//     }
//     lua_setfield(L, -2, "zplt_orig");

//     lua_settable(L, -3);
//   }
//   // Leaves one returned value on the stack, the mesh table.
//   return 1;
// }

// /// @brief Get the number of meshes in the loaded model.
// json_object *jsonrpc_GetNdevices(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushnumber(L, ndeviceinfo);
//   return 1;
// }

// /// @brief Build a Lua table with information on the devices of the model.
// json_object *jsonrpc_GetDevices(jrpc_context *context, json_object *params,
// json_object *id) {
//   int entries = ndeviceinfo;
//   devicedata *infotable = deviceinfo;
//   lua_createtable(L, 0, entries);
//   int i;
//   for (i = 0; i < entries; i++) {
//     lua_pushstring(L, infotable[i].deviceID);
//     lua_createtable(L, 0, 2);

//     lua_pushstring(L, infotable[i].deviceID);
//     lua_setfield(L, -2, "label");

//     lua_settable(L, -3);
//   }
//   return 1;
// }

// json_object *jsonrpc_CreateVector(jrpc_context *context, json_object *params,
// json_object *id, csvdata *csv_x, csvdata *csv_y) {
//   size_t i;
//   lua_createtable(L, 0, 3);

//   lua_pushstring(L, csv_y->label.longlabel);
//   lua_setfield(L, -2, "name");

//   // x-vector
//   lua_createtable(L, 0, 3);
//   lua_pushstring(L, csv_x->label.longlabel);
//   lua_setfield(L, -2, "name");
//   lua_pushstring(L, csv_x->label.unit);
//   lua_setfield(L, -2, "units");
//   lua_createtable(L, 0, csv_x->nvals);
//   for (i = 0; i < csv_x->nvals; ++i) {
//     lua_pushnumber(L, i + 1);
//     lua_pushnumber(L, csv_x->vals[i]);
//     lua_settable(L, -3);
//   }
//   lua_setfield(L, -2, "values");
//   lua_setfield(L, -2, "x");
//   // y-vector
//   lua_createtable(L, 0, 3);
//   lua_pushstring(L, csv_y->label.longlabel);
//   lua_setfield(L, -2, "name");
//   lua_pushstring(L, csv_y->label.unit);
//   lua_setfield(L, -2, "units");
//   lua_createtable(L, 0, csv_y->nvals);
//   for (i = 0; i < csv_y->nvals; ++i) {
//     lua_pushnumber(L, i + 1);
//     lua_pushnumber(L, csv_y->vals[i]);
//     lua_settable(L, -3);
//   }
//   lua_setfield(L, -2, "values");
//   lua_setfield(L, -2, "y");
//   return 1;
// }

// /// @brief Get the number of CSV files available to the model.
// json_object *jsonrpc_GetNcsvinfo(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushnumber(L, ncsvfileinfo);
//   return 1;
// }

// csvfiledata *GetCsvinfo(const char *key) {
//   // Loop through csvinfo until we find the right entry
//   size_t i;
//   for (i = 0; i < ncsvfileinfo; ++i) {
//     if (strcmp(csvfileinfo[i].c_type, key) == 0) {
//       return &csvfileinfo[i];
//     }
//   }
//   return NULL;
// }

// int GetCsvindex(const char *key) {
//   // Loop through csvinfo until we find the right entry
//   size_t i;
//   for (i = 0; i < ncsvfileinfo; ++i) {
//     if (strcmp(csvfileinfo[i].c_type, key) == 0) {
//       return i;
//     }
//   }
//   return -1;
// }

// void LoadCsv(csvfiledata *csventry) {
//   ReadCSVFile(csventry, LOAD);
//   csventry->loaded = 1;
// }

// json_object *jsonrpc_LoadCsv(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushstring(L, "c_type");
//   lua_gettable(L, 1);
//   const char *key = lua_tostring(L, -1);
//   csvfiledata *csventry = GetCsvinfo(key);
//   if (csventry == NULL) return 0;
//   LoadCsv(csventry);
//   return 0;
// }

// int AccessCsventryProp(jrpc_context *context, json_object *params,
// json_object *id) {
//   // Take the index from the table.
//   lua_pushstring(L, "index");
//   lua_gettable(L, 1);
//   int index = lua_tonumber(L, -1);
//   const char *field = lua_tostring(L, 2);
//   if (strcmp(field, "loaded") == 0) {
//     lua_pushboolean(L, csvfileinfo[index].loaded);
//     return 1;
//   }
//   else if (strcmp(field, "display") == 0) {
//     lua_pushboolean(L, csvfileinfo[index].display);
//     return 1;
//   }
//   else if (strcmp(field, "vectors") == 0) {
//     csvfiledata *csventry = &csvfileinfo[index];
//     if (!csventry->loaded) {
//       LoadCsv(csventry);
//     }
//     // TODO: don't create every time
//     lua_createtable(L, 0, csventry->ncsvinfo);
//     size_t j;
//     for (j = 0; j < csventry->ncsvinfo; j++) {
//       // Load vector data into lua.
//       // TODO: change to access indirectly rater than copying via stack
//       // printf("adding: %s\n", csventry->vectors[j].y->name);

//       LuaCreateVector(L, csventry->time, &(csventry->csvinfo[j]));
//       lua_setfield(L, -2, csventry->csvinfo[j].label.longlabel);
//     }
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_CsvIsLoaded(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *key = lua_tostring(L, lua_upvalueindex(1));
//   csvfiledata *csventry = GetCsvinfo(key);
//   lua_pushboolean(L, csventry->loaded);
//   return 1;
// }

// /// @brief Create a table so that a metatable can be used.
// json_object *jsonrpc_GetCsvdata(jrpc_context *context, json_object *params,
// json_object *id) {
//   // L1 is the table
//   // L2 is the string key
//   const char *key = lua_tostring(L, 2);
//   // char *file = lua_tostring(L, 1);
//   csvfiledata *csventry = GetCsvinfo(key);
//   // Check if the chosen csv data is loaded
//   if (!csventry->loaded) {
//     // Load the data.
//     LoadCsv(csventry);
//   }
//   // TODO: put userdata on stack
//   lua_pushlightuserdata(L, csventry->csvinfo);
//   return 1;
// }

// int AccessPl3dentryProp(jrpc_context *context, json_object *params,
// json_object *id) {
//   // Take the index from the table.
//   lua_pushstring(L, "index");
//   lua_gettable(L, 1);
//   int index = lua_tonumber(L, -1);
//   const char *field = lua_tostring(L, 2);
//   if (strcmp(field, "loaded") == 0) {
//     lua_pushboolean(L, plot3dinfo[index].loaded);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_SetPl3dBoundMin(jrpc_context *context, json_object
// *params, json_object *id) {
//   int pl3d_value_index = lua_tonumber(L, 1);
//   int set = lua_toboolean(L, 2);
//   float value = lua_tonumber(L, 3);
//   SetPl3dBoundMin(pl3d_value_index, set, value);
//   return 0;
// }

// json_object *jsonrpc_SetPl3dBoundMax(jrpc_context *context, json_object
// *params, json_object *id) {
//   int pl3d_value_index = lua_tonumber(L, 1);
//   int set = lua_toboolean(L, 2);
//   float value = lua_tonumber(L, 3);
//   SetPl3dBoundMax(pl3d_value_index, set, value);
//   return 0;
// }

// /// @brief Get the number of PL3D files available to the model.
// json_object *jsonrpc_GetNplot3dinfo(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_pushnumber(L, nplot3dinfo);
//   return 1;
// }

// json_object *jsonrpc_GetPlot3dentry(jrpc_context *context, json_object
// *params, json_object *id) {
//   int lua_index = lua_tonumber(L, -1);
//   int index = lua_index - 1;
//   int i;

//   // csvdata *csventry = get_csvinfo(key);
//   // fprintf(stderr, "csventry->file: %s\n", csventry->file);
//   lua_createtable(L, 0, 4);

//   lua_pushnumber(L, index);
//   lua_setfield(L, -2, "index");

//   lua_pushstring(L, plot3dinfo[index].file);
//   lua_setfield(L, -2, "file");

//   lua_pushstring(L, plot3dinfo[index].reg_file);
//   lua_setfield(L, -2, "reg_file");

//   lua_pushstring(L, plot3dinfo[index].longlabel);
//   lua_setfield(L, -2, "longlabel");

//   lua_pushnumber(L, plot3dinfo[index].time);
//   lua_setfield(L, -2, "time");

//   lua_pushnumber(L, plot3dinfo[index].u);
//   lua_setfield(L, -2, "u");
//   lua_pushnumber(L, plot3dinfo[index].v);
//   lua_setfield(L, -2, "v");
//   lua_pushnumber(L, plot3dinfo[index].w);
//   lua_setfield(L, -2, "w");

//   lua_pushnumber(L, plot3dinfo[index].nplot3dvars);
//   lua_setfield(L, -2, "nplot3dvars");

//   lua_pushnumber(L, plot3dinfo[index].blocknumber);
//   lua_setfield(L, -2, "blocknumber");

//   lua_pushnumber(L, plot3dinfo[index].display);
//   lua_setfield(L, -2, "display");

//   lua_createtable(L, 0, 6);
//   for (i = 0; i < 6; ++i) {
//     lua_pushnumber(L, i + 1);

//     lua_createtable(L, 0, 3);
//     lua_pushstring(L, plot3dinfo[index].label[i].longlabel);
//     lua_setfield(L, -2, "longlabel");
//     lua_pushstring(L, plot3dinfo[index].label[i].shortlabel);
//     lua_setfield(L, -2, "shortlabel");
//     lua_pushstring(L, plot3dinfo[index].label[i].unit);
//     lua_setfield(L, -2, "unit");

//     lua_settable(L, -3);
//   }
//   lua_setfield(L, -2, "label");

//   // Create a metatable.
//   // TODO: this metatable might be more easily implemented directly in Lua
//   // so that we don't need to reimplement table access.
//   lua_createtable(L, 0, 1);
//   lua_pushcfunction(L, &AccessPl3dentryProp);
//   lua_setfield(L, -2, "__index");
//   // then set the metatable
//   lua_setmetatable(L, -2);

//   return 1;
// }

// json_object *jsonrpc_GetPlot3dinfo(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_createtable(L, 0, nplot3dinfo);
//   int i;
//   for (i = 0; i < nplot3dinfo; i++) {
//     lua_pushnumber(L, i + 1);
//     LuaGetPlot3dentry(L);

//     lua_settable(L, -3);
//   }
//   return 1;
// }

// json_object *jsonrpc_GetQdataSum(jrpc_context *context, json_object *params,
// json_object *id) {
//   int meshnumber = lua_tonumber(L, 1);
//   int vari, i, j, k;
//   meshdata mesh = meshinfo[meshnumber];
//   int ntotal = (mesh.ibar + 1) * (mesh.jbar + 1) * (mesh.kbar + 1);
//   int vars = 5;
//   float totals[5];
//   totals[0] = 0.0;
//   totals[1] = 0.0;
//   totals[2] = 0.0;
//   totals[3] = 0.0;
//   totals[4] = 0.0;
//   for (vari = 0; vari < 5; ++vari) {
//     int offset = vari * ntotal;
//     for (k = 0; k <= mesh.kbar; ++k) {
//       for (j = 0; j <= mesh.jbar; ++j) {
//         for (i = 0; i <= mesh.ibar; ++i) {
//           int n = offset + k * (mesh.jbar + 1) * (mesh.ibar + 1) +
//                   j * (mesh.ibar + 1) + i;
//           totals[vari] += mesh.qdata[n];
//         }
//       }
//     }
//   }
//   for (vari = 0; vari < vars; ++vari) {
//     lua_pushnumber(L, totals[vari]);
//   }
//   lua_pushnumber(L, ntotal);
//   return vars + 1;
// }

// json_object *jsonrpc_GetGlobalTimeN(jrpc_context *context, json_object
// *params, json_object *id) {
//   // argument 1 is the table, argument 2 is the index
//   int index = lua_tonumber(L, 2);
//   if (index < 0 || index >= nglobal_times) {
//     return luaL_error(L, "%d is not a valid global time index\n", index);
//   }
//   lua_pushnumber(L, global_times[index]);
//   return 1;
// }
json_object *json_GetSmoke3ds() {
  struct json_object *smoke3ds = json_object_new_array();
  for (int i = 0; i < nsmoke3dinfo; i++) {
    smoke3ddata *val = &smoke3dinfo[i];
    struct json_object *slice_obj = json_object_new_object();
    json_object_object_add(slice_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(slice_obj, "mesh",
                           json_object_new_int(val->blocknumber));
    if (val->label.longlabel != NULL) {
      json_object_object_add(slice_obj, "longlabel",
                             json_object_new_string(val->label.longlabel));
    }
    if (val->label.shortlabel) {
      json_object_object_add(slice_obj, "shortlabel",
                             json_object_new_string(val->label.shortlabel));
    }
    json_object_object_add(slice_obj, "type", json_object_new_int(val->type));
    // if (val->label.unit) {
    //   json_object_object_add(slice_obj, "unit",
    //                          json_object_new_string(val->smoke_type));
    // }
    // struct json_object *coordinates = json_object_new_object();
    // json_object_object_add(coordinates, "i_min",
    //                        json_object_new_int(smoke3ddata->ijk_min[0]));
    // json_object_object_add(coordinates, "i_max",
    //                        json_object_new_int(smoke3ddata->ijk_max[0]));
    // json_object_object_add(coordinates, "j_min",
    //                        json_object_new_int(smoke3ddata->ijk_min[1]));
    // json_object_object_add(coordinates, "j_max",
    //                        json_object_new_int(smoke3ddata->ijk_max[1]));
    // json_object_object_add(coordinates, "k_min",
    //                        json_object_new_int(smoke3ddata->ijk_min[2]));
    // json_object_object_add(coordinates, "k_max",
    //                        json_object_new_int(smoke3ddata->ijk_max[2]));
    // json_object_object_add(slice_obj, "coordinates", coordinates);
    json_object_array_add(smoke3ds, slice_obj);
  }
  return smoke3ds;
}

json_object *json_GetSlices() {
  struct json_object *slices = json_object_new_array();
  for (int i = 0; i < nsliceinfo; i++) {
    slicedata *slice = &sliceinfo[i];
    struct json_object *slice_obj = json_object_new_object();
    json_object_object_add(slice_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(slice_obj, "mesh",
                           json_object_new_int(slice->blocknumber));
    if (slice->label.longlabel != NULL) {
      json_object_object_add(slice_obj, "longlabel",
                             json_object_new_string(slice->label.longlabel));
    }
    if (slice->label.shortlabel) {
      json_object_object_add(slice_obj, "shortlabel",
                             json_object_new_string(slice->label.shortlabel));
    }
    if (slice->slicelabel) {
      json_object_object_add(slice_obj, "id",
                             json_object_new_string(slice->slicelabel));
    }
    if (slice->label.unit) {
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

// /// @brief This takes a lightuserdata pointer as an argument, and returns the
// /// slice label as a string.
// json_object *jsonrpc_SliceGetLabel(jrpc_context *context, json_object
// *params, json_object *id) {
//   // get the lightuserdata from the stack, which is a pointer to the
//   'slicedata' slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   // Push the string onto the stack
//   lua_pushstring(L, slice->slicelabel);
//   return 1;
// }

// /// @brief This takes a lightuserdata pointer as an argument, and returns the
// /// slice filename as a string.
// json_object *jsonrpc_SliceGetFilename(jrpc_context *context, json_object
// *params, json_object *id) {
//   // Get the lightuserdata from the stack, which is a pointer to the
//   // 'slicedata'.
//   slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   // Push the string onto the stack
//   lua_pushstring(L, slice->file);
//   return 1;
// }

// json_object *jsonrpc_SliceGetData(jrpc_context *context, json_object *params,
// json_object *id) {
//   // get the lightuserdata from the stack, which is a pointer to the
//   'slicedata' slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   // Push a lightuserdata (a pointer) onto the lua stack that points to the
//   // qslicedata.
//   lua_pushlightuserdata(L, slice->qslicedata);
//   return 1;
// }

// json_object *jsonrpc_SliceGetTimes(jrpc_context *context, json_object
// *params, json_object *id) {
//   int i;
//   // get the lightuserdata from the stack, which is a pointer to the
//   'slicedata' slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   // Push a lightuserdata (a pointer) onto the lua stack that points to the
//   // qslicedata.
//   lua_createtable(L, slice->ntimes, 0);
//   for (i = 0; i < slice->ntimes; i++) {
//     lua_pushnumber(L, i + 1);
//     lua_pushnumber(L, slice->times[i]);
//     lua_settable(L, -3);
//   }
//   return 1;
// }

// json_object *jsonrpc_GetPart(jrpc_context *context, json_object *params,
// json_object *id) {
//   // This should push a lightuserdata onto the stack which is a pointer to
//   the
//   // partdata. This takes the index of the part (in the partinfo array) as an
//   // argument.
//   // Get the index of the slice as an argument to the lua function.
//   int part_index = lua_tonumber(L, 1);
//   // Get the pointer to the slicedata struct.
//   partdata *part = &partinfo[part_index];
//   // Push the pointer onto the lua stack as lightuserdata.
//   lua_pushlightuserdata(L, part);
//   // lua_newuserdata places the data on the stack, so return a single stack
//   // item.
//   return 1;
// }

// // pass in the part data
// json_object *jsonrpc_GetPartNpoints(jrpc_context *context, json_object
// *params, json_object *id) {
//   int index;
//   partdata *parti = (partdata *)lua_touserdata(L, 1);
//   if (!parti->loaded) {
//     return luaL_error(L, "particle file %s not loaded", parti->file);
//   }

//   // Create a table with an entry for x, y and name
//   lua_createtable(L, 3, 0);

//   lua_pushstring(L, "name");
//   lua_pushstring(L, "(unknown)");
//   lua_settable(L, -3);

//   // x entries
//   lua_pushstring(L, "x");
//   lua_createtable(L, 3, 0);

//   lua_pushstring(L, "units");
//   lua_pushstring(L, "s");
//   lua_settable(L, -3);

//   lua_pushstring(L, "name");
//   lua_pushstring(L, "Time");
//   lua_settable(L, -3);

//   lua_pushstring(L, "values");
//   lua_createtable(L, parti->ntimes, 0);

//   // Create a table with an entry for each time
//   for (index = 0; index < parti->ntimes; index++) {
//     part5data *part5 = parti->data5 + index * parti->nclasses;
//     // sum += part5->npoints_file;

//     // use a 1-indexed array to match lua
//     lua_pushnumber(L, index + 1);
//     lua_pushnumber(L, part5->time);
//     lua_settable(L, -3);
//   }
//   lua_settable(L, -3);
//   lua_settable(L, -3);

//   // y entries
//   lua_pushstring(L, "y");
//   lua_createtable(L, 3, 0);

//   lua_pushstring(L, "units");
//   lua_pushstring(L, "#");
//   lua_settable(L, -3);

//   lua_pushstring(L, "name");
//   lua_pushstring(L, "# Particles");
//   lua_settable(L, -3);

//   lua_pushstring(L, "values");
//   lua_createtable(L, parti->ntimes, 0);

//   // Create a table with an entry for each time
//   for (index = 0; index < parti->ntimes; index++) {
//     part5data *part5 = parti->data5 + index * parti->nclasses;
//     // sum += part5->npoints_file;

//     // use a 1-indexed array to match lua
//     lua_pushnumber(L, index + 1);
//     lua_pushnumber(L, part5->npoints_file);
//     lua_settable(L, -3);
//   }
//   lua_settable(L, -3);
//   lua_settable(L, -3);

//   // Return a table of values.
//   return 1;
// }

// json_object *jsonrpc_GetCsventry(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *key = lua_tostring(L, -1);
//   csvfiledata *csventry = GetCsvinfo(key);
//   int index = GetCsvindex(key);
//   lua_createtable(L, 0, 4);
//   lua_pushstring(L, csventry->file);
//   lua_setfield(L, -2, "file");

//   lua_pushstring(L, csventry->c_type);
//   lua_setfield(L, -2, "c_type");

//   lua_pushnumber(L, index);
//   lua_setfield(L, -2, "index");

//   lua_pushstring(L, key);
//   lua_pushcclosure(L, &jsonrpc_LoadCsv, 1);
//   lua_setfield(L, -2, "load");

//   // Create a metatable.
//   // TODO: this metatable might be more easily implemented directly in Lua.
//   lua_createtable(L, 0, 1);
//   lua_pushcfunction(L, &AccessCsventryProp);
//   lua_setfield(L, -2, "__index");
//   // then set the metatable
//   lua_setmetatable(L, -2);
//   return 1;
// }

// /// @brief Build a Lua table with information on the CSV files available to
// the
// /// model.
// // TODO: provide more information via this interface.
// // TODO: use metatables so that the most up-to-date information is retrieved.
// json_object *jsonrpc_GetCsvinfo(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_createtable(L, 0, ncsvfileinfo);
//   int i;
//   for (i = 0; i < ncsvfileinfo; i++) {
//     lua_pushstring(L, csvfileinfo[i].c_type);
//     LuaGetCsventry(L);
//     lua_settable(L, -3);
//   }
//   return 1;
// }

// json_object *jsonrpc_Loadvslice(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *type = lua_tostring(L, 1);
//   int axis = lua_tonumber(L, 2);
//   float distance = lua_tonumber(L, 3);
//   Loadvslice(type, axis, distance);
//   return 0;
// }

// json_object *jsonrpc_Loadiso(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *type = lua_tostring(L, 1);
//   Loadiso(type);
//   return 0;
// }

/// @brief Load a slice based on its index in sliceinfo.
json_object *jsonrpc_LoadSliceIndices(jrpc_context *context,
                                      json_object *params, json_object *id) {
  int errorcode = 0;
  size_t n_files = json_object_array_length(params);
  for (size_t n = 0; n < n_files; n++) {
    int i = json_object_get_int(json_object_array_get_idx(params, n));
    Loadsliceindex(i, &errorcode);
    if (errorcode) {
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
  for (size_t n = 0; n < n_files; n++) {
    int i = json_object_get_int(json_object_array_get_idx(params, n));
    fprintf(stderr, "loading smoke3d index %d\n", i);
    smoke3ddata *smoke3di;
    smoke3di = smoke3dinfo + i;
    smoke3di->finalize = 0;
    if (n == (n_files - 1)) smoke3di->finalize = 1;
    ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
    if (errorcode) {
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
  switch (render_type) {
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
  switch (movie_type) {
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

// json_object *jsonrpc_Makemovie(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *name = lua_tostring(L, 1);
//   const char *base = lua_tostring(L, 2);
//   float framerate = lua_tonumber(L, 3);
//   Makemovie(name, base, framerate);
//   return 0;
// }

// json_object *jsonrpc_Loadparticles(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *name = lua_tostring(L, 1);
//   Loadparticles(name);
//   return 0;
// }

json_object *jsonrpc_Unloadall(jrpc_context *context, json_object *params,
                               json_object *id) {
  Unloadall();
  return NULL;
}

json_object *jsonrpc_Setrenderdir(jrpc_context *context, json_object *params,
                                  json_object *id) {
  const char *dir =
      json_object_get_string(json_object_array_get_idx(params, 0));
  if (Setrenderdir(dir)) {
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

  const char *viewpoint =
      json_object_get_string(json_object_array_get_idx(params, 0));
  if (SetOrthoPreset(viewpoint)) {
    context->error_code = 119;
    context->error_message = strdup("SetOrthoPreset failure");
  }
  return NULL;
}

json_object *jsonrpc_Setviewpoint(jrpc_context *context, json_object *params,
                                  json_object *id) {
  const char *viewpoint =
      json_object_get_string(json_object_array_get_idx(params, 0));
  if (Setviewpoint(viewpoint)) {
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

json_object *jsonrpc_Setcolorbarflip(jrpc_context *context, json_object *params,
                                     json_object *id) {
  json_bool flip =
      json_object_get_boolean(json_object_array_get_idx(params, 0));
  Setcolorbarflip(flip ? 1 : 0);
  return NULL;
}

json_object *jsonrpc_Getcolorbarflip(jrpc_context *context, json_object *params,
                                     json_object *id) {
  int flip = Getcolorbarflip();
  if (flip) {
    return json_object_new_boolean(1);
  }
  else {
    return json_object_new_boolean(0);
  }
}

// json_object *jsonrpc_Setcolorbarindex(jrpc_context *context,
//                                       json_object *params, json_object *id) {
//   int chosen_index = lua_tonumber(L, 1);
//   Setcolorbarindex(chosen_index);
//   return 0;
// }

// json_object *jsonrpc_Getcolorbarindex(jrpc_context *context,
//                                       json_object *params, json_object *id) {
//   int index = Getcolorbarindex();
//   lua_pushnumber(L, index);
//   return 1;
// }

// json_object *jsonrpc_SetColorbar(jrpc_context *context, json_object *params,
// json_object *id) {
//   int index = lua_tonumber(L, 1);
//   SetColorbar(index);
//   return 0;
// }

// json_object *jsonrpc_SetNamedColorbar(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *name = lua_tostring(L, 1);
//   int err = SetNamedColorbar(name);
//   if (err == 1) {
//     luaL_error(L, "%s is not a valid colorbar name", name);
//   }
//   return 0;
// }

// // int lua_get_named_colorbar(jrpc_context *context, json_object *params,
// json_object *id) {
// //   int err = GetNamedColorbar();
// //   if (err == 1) {
// //     luaL_error(L, "%s is not a valid colorbar name", name);
// //   }
// //   return 0;
// // }

// //////////////////////

// json_object *jsonrpc_SetColorbarVisibility(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetColorbarVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetColorbarVisibility(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int setting = GetColorbarVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_SetColorbarVisibilityHorizontal(jrpc_context *context,
// json_object *jsonrpc_params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetColorbarVisibilityHorizontal(setting);
//   return 0;
// }

// json_object *jsonrpc_GetColorbarVisibilityHorizontal(jrpc_context *context,
// json_object *jsonrpc_params,  json_object *id) {
//   int setting = GetColorbarVisibilityHorizontal();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_SetColorbarVisibilityVertical(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetColorbarVisibilityVertical(setting);
//   return 0;
// }

// json_object *jsonrpc_GetColorbarVisibilityVertical(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetColorbarVisibilityVertical();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_SetTimebarVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetTimebarVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetTimebarVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = GetTimebarVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// // title
// json_object *jsonrpc_SetTitleVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetTitleVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetTitleVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = GetTitleVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// // smv_version
// json_object *jsonrpc_SetSmvVersionVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetSmvVersionVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetSmvVersionVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetSmvVersionVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

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
  if (v) {
    return json_object_new_boolean(1);
  }
  else {
    return json_object_new_boolean(0);
  }
}

// // blockages
// json_object *jsonrpc_BlockagesHideAll(jrpc_context *context, json_object
// *params, json_object *id) {
//   BlockagesHideAll();
//   return 0;
// }

// // outlines
// json_object *jsonrpc_OutlinesHide(jrpc_context *context, json_object *params,
// json_object *id) {
//   OutlinesHide();
//   return 0;
// }
// json_object *jsonrpc_OutlinesShow(jrpc_context *context, json_object *params,
// json_object *id) {
//   OutlinesShow();
//   return 0;
// }

// // surfaces
// json_object *jsonrpc_SurfacesHideAll(jrpc_context *context, json_object
// *params, json_object *id) {
//   SurfacesHideAll();
//   return 0;
// }

// // devices
// json_object *jsonrpc_DevicesHideAll(jrpc_context *context, json_object
// *params, json_object *id) {
//   DevicesHideAll();
//   return 0;
// }

// // axis
// json_object *jsonrpc_SetAxisVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetAxisVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetAxisVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = GetAxisVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// // frame
// json_object *jsonrpc_SetFramelabelVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetFramelabelVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetFramelabelVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetFramelabelVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// // framerate
// json_object *jsonrpc_SetFramerateVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetFramerateVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetFramerateVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetFramerateVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// // grid locations
// json_object *jsonrpc_SetGridlocVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetGridlocVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetGridlocVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = GetGridlocVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// // time
// json_object *jsonrpc_SetTimeVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetTimeVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetTimeVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = GetTimeVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// // version info
// json_object *jsonrpc_SetVersionInfoVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetVersionInfoVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetVersionInfoVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetVersionInfoVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// //////////////////////////////////////

// json_object *jsonrpc_BlockageViewMethod(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_tonumber(L, 1);
//   int return_code = BlockageViewMethod(setting);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_BlockageOutlineColor(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_tonumber(L, 1);
//   int return_code = BlockageOutlineColor(setting);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_BlockageLocations(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_tonumber(L, 1);
//   int return_code = BlockageLocations(setting);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_CameraModEyex(jrpc_context *context, json_object
// *params, json_object *id) {
//   float delta = lua_tonumber(L, 1);
//   CameraModEyex(delta);
//   return 0;
// }

// json_object *jsonrpc_CameraSetEyex(jrpc_context *context, json_object
// *params, json_object *id) {
//   float eyex = lua_tonumber(L, 1);
//   CameraSetEyex(eyex);
//   return 0;
// }

// json_object *jsonrpc_CameraModEyey(jrpc_context *context, json_object
// *params, json_object *id) {
//   float delta = lua_tonumber(L, 1);
//   CameraModEyey(delta);
//   return 0;
// }

// json_object *jsonrpc_CameraSetEyey(jrpc_context *context, json_object
// *params, json_object *id) {
//   float eyey = lua_tonumber(L, 1);
//   CameraSetEyey(eyey);
//   return 0;
// }

// json_object *jsonrpc_CameraModEyez(jrpc_context *context, json_object
// *params, json_object *id) {
//   float delta = lua_tonumber(L, 1);
//   CameraModEyez(delta);
//   return 0;
// }

// json_object *jsonrpc_CameraSetEyez(jrpc_context *context, json_object
// *params, json_object *id) {
//   float eyez = lua_tonumber(L, 1);
//   CameraSetEyez(eyez);
//   return 0;
// }

// json_object *jsonrpc_CameraModAz(jrpc_context *context, json_object *params,
// json_object *id) {
//   float delta = lua_tonumber(L, 1);
//   CameraModAz(delta);
//   return 0;
// }

// json_object *jsonrpc_CameraSetAz(jrpc_context *context, json_object *params,
// json_object *id) {
//   float az = lua_tonumber(L, 1);
//   CameraSetAz(az);
//   return 0;
// }

// json_object *jsonrpc_CameraGetAz(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushnumber(L, CameraGetAz());
//   return 1;
// }

// json_object *jsonrpc_CameraModElev(jrpc_context *context, json_object
// *params, json_object *id) {
//   float delta = lua_tonumber(L, 1);
//   CameraModElev(delta);
//   return 0;
// }
// json_object *jsonrpc_CameraZoomToFit(jrpc_context *context, json_object
// *params, json_object *id) {
//   CameraZoomToFit();
//   return 0;
// }

// json_object *jsonrpc_CameraSetElev(jrpc_context *context, json_object
// *params, json_object *id) {
//   float elev = lua_tonumber(L, 1);
//   CameraSetElev(elev);
//   return 0;
// }

// json_object *jsonrpc_CameraGetElev(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_pushnumber(L, CameraGetElev());
//   return 1;
// }
// json_object *jsonrpc_CameraGetProjectionType(jrpc_context *context,
// json_object *params,  json_object *id) {
//   float projection_type = CameraGetProjectionType();
//   lua_pushnumber(L, projection_type);
//   return 1;
// }
// json_object *jsonrpc_CameraSetProjectionType(jrpc_context *context,
// json_object *params,  json_object *id) {
//   float projection_type = lua_tonumber(L, 1);
//   int return_value = CameraSetProjectionType(projection_type);
//   lua_pushnumber(L, return_value);
//   return 1;
// }

// json_object *jsonrpc_CameraGetRotationType(jrpc_context *context, json_object
// *params,  json_object *id) {
//   float rotation_type = CameraGetRotationType();
//   lua_pushnumber(L, rotation_type);
//   return 1;
// }

// json_object *jsonrpc_CameraGetRotationIndex(jrpc_context *context,
// json_object *params,  json_object *id) {
//   float rotation_index = CameraGetRotationIndex();
//   lua_pushnumber(L, rotation_index);
//   return 1;
// }

// json_object *jsonrpc_CameraSetRotationType(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int rotation_type = lua_tonumber(L, 1);
//   CameraSetRotationType(rotation_type);
//   return 0;
// }

// json_object *jsonrpc_CameraGetZoom(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_pushnumber(L, zoom);
//   return 1;
// }

// json_object *jsonrpc_CameraSetZoom(jrpc_context *context, json_object
// *params, json_object *id) {
//   float x = lua_tonumber(L, 1);
//   zoom = x;
//   return 0;
// }

// json_object *jsonrpc_CameraGetEyex(jrpc_context *context, json_object
// *params, json_object *id) {
//   float eyex = CameraGetEyex();
//   lua_pushnumber(L, eyex);
//   return 1;
// }

// json_object *jsonrpc_CameraGetEyey(jrpc_context *context, json_object
// *params, json_object *id) {
//   float eyey = CameraGetEyex();
//   lua_pushnumber(L, eyey);
//   return 1;
// }

// json_object *jsonrpc_CameraGetEyez(jrpc_context *context, json_object
// *params, json_object *id) {
//   float eyez = CameraGetEyez();
//   lua_pushnumber(L, eyez);
//   return 1;
// }
// json_object *jsonrpc_CameraSetViewdir(jrpc_context *context, json_object
// *params, json_object *id) {
//   float xcen = lua_tonumber(L, 1);
//   float ycen = lua_tonumber(L, 2);
//   float zcen = lua_tonumber(L, 3);
//   CameraSetViewdir(xcen, ycen, zcen);
//   return 0;
// }

// json_object *jsonrpc_CameraGetViewdir(jrpc_context *context, json_object
// *params, json_object *id) {
//   float xcen = CameraGetXcen();
//   float ycen = CameraGetYcen();
//   float zcen = CameraGetZcen();

//   lua_createtable(L, 0, 3);

//   lua_pushstring(L, "x");
//   lua_pushnumber(L, xcen);
//   lua_settable(L, -3);

//   lua_pushstring(L, "y");
//   lua_pushnumber(L, ycen);
//   lua_settable(L, -3);

//   lua_pushstring(L, "z");
//   lua_pushnumber(L, zcen);
//   lua_settable(L, -3);

//   return 1;
// }

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
  CApiSetSliceBounds(slice_type, set_min, value_min, set_max, value_max);
  return NULL;
}

// json_object *jsonrpc_SetFlip(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_toboolean(L, 1);
//   int return_code = SetFlip(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_GetFlip(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushboolean(L, GetFlip());
//   return 1;
// }

// json_object *jsonrpc_SetTimebarcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetTimebarcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetScaledfont(jrpc_context *context, json_object
// *params, json_object *id) {
//   int height2d = lua_tonumber(L, 1);
//   int height2dwidth = lua_tonumber(L, 2);
//   int thickness2d = lua_tonumber(L, 3);
//   int height3d = lua_tonumber(L, 3);
//   int height3dwidth = lua_tonumber(L, 5);
//   int thickness3d = lua_tonumber(L, 6);
//   int return_code = SetScaledfont(height2d, height2dwidth, thickness2d,
//                                   height3d, height3dwidth, thickness3d);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_GetFontsize(jrpc_context *context, json_object *params,
// json_object *id) {
//   switch (fontindex) {
//   case SMALL_FONT:
//     lua_pushstring(L, "small");
//     return 1;
//     break;
//   case LARGE_FONT:
//     lua_pushstring(L, "large");
//     return 1;
//     break;
//   case SCALED_FONT:
//     lua_pushnumber(L, scaled_font2d_height);
//     return 1;
//     break;
//   default:
//     return luaL_error(L, "font size is invalid");
//     break;
//   }
// }

// json_object *jsonrpc_SetScaledfontHeight2d(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int height2d = lua_tonumber(L, 1);
//   int return_code = SetScaledfontHeight2d(height2d);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowhmstimelabel(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowhmstimelabel(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowlabels(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowlabels(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// #ifdef pp_memstatus
// json_object *jsonrpc_SetShowmemload(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowmemload(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// #endif

// json_object *jsonrpc_ShowSmoke3dShowall(jrpc_context *context, json_object
// *params, json_object *id) {
//   int return_code = ShowSmoke3dShowall();
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_ShowSmoke3dHideall(jrpc_context *context, json_object
// *params, json_object *id) {
//   int return_code = ShowSmoke3dHideall();
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_ShowSlicesShowall(jrpc_context *context, json_object
// *params, json_object *id) {
//   int return_code = ShowSlicesShowall();
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_ShowSlicesHideall(jrpc_context *context, json_object
// *params, json_object *id) {
//   int return_code = ShowSlicesHideall();
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_AddTitleLine(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *string = lua_tostring(L, 1);
//   int return_code = addTitleLine(&titleinfo, string);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_ClearTitleLines(jrpc_context *context, json_object
// *params, json_object *id) {
//   int return_code = clearTitleLines(&titleinfo);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

int register_procedures(struct jrpc_server *server) {
  jrpc_register_procedure(server, &jsonrpc_Setframe, "set_frame", NULL);
  jrpc_register_procedure(server, &jsonrpc_SetClipping, "set_clipping", NULL);
  jrpc_register_procedure(server, &jsonrpc_Render, "render", NULL);
  jrpc_register_procedure(server, &jsonrpc_Unloadall, "unload_all", NULL);
  jrpc_register_procedure(server, &jsonrpc_SetChidVisibility,
                          "set_chid_visibility", NULL);
  jrpc_register_procedure(server, &jsonrpc_ExitSmokeview, "exit", NULL);
  jrpc_register_procedure(server, &jsonrpc_SetSliceBounds, "set_slice_bounds",
                          NULL);
  jrpc_register_procedure(server, &jsonrpc_SetWindowSize, "set_window_size",
                          NULL);
  jrpc_register_procedure(server, &jsonrpc_LoadSliceIndices,
                          "load_slice_indices", NULL);
  jrpc_register_procedure(server, &jsonrpc_GetSlices, "get_slices", NULL);
  jrpc_register_procedure(server, &json_GetSmoke3ds, "get_smoke3ds", NULL);
  jrpc_register_procedure(server, &jsonrpc_Load3dSmokeIndices,
                          "load_smoke3d_indices", NULL);
  jrpc_register_procedure(server, &jsonrpc_SetOrthoPreset, "set_ortho_preset", NULL);

  return 0;
}

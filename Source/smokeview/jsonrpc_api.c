
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

// char *ParseCommandline(int argc, char **argv);

// int CheckSMVFileLua(char *file, char *subdir) {
//   char casedir[256], *casedirptr, casename[256];
//   FILE *stream;

//   if (file == NULL) return 1;

//   strcpy(casename, file);
//   if (subdir == NULL) {
//     casedirptr = casedir;
//     strcpy(casedir, casename);
//   }
//   else {
//     casedirptr = subdir;
//   }
//   stream = fopen(casename, "r");
//   if (stream == NULL) {
//     stream = fopen_indir(casedirptr, casename, "r");
//     if (stream == NULL) {
//       printf("***error: unable to open %s\n", casename);
//       return 0;
//     }
//     CHDIR(casedirptr);
//   }
//   fclose(stream);
//   return 1;
// }

// int ProgramSetupLua(jrpc_context *context, json_object *params,  json_object
// *id, int argc, char **argv) {
//   char *progname;
//   InitVars();
//   ParseCommonOptions(argc, argv);
//   smv_filename = ParseCommandline(argc, argv);
//   printf("smv_filename: %s\n", smv_filename);

//   progname = argv[0];
//   prog_fullpath = progname;
// #ifdef pp_LUA
//   smokeview_bindir_abs = getprogdirabs(progname, &smokeviewpath);
// #endif
//   if (smokeview_bindir == NULL) {
//     smokeview_bindir = GetProgDir(progname, &smokeviewpath);
//   }
//   if (show_version == 1 || smv_filename == NULL) {
//     DisplayVersionInfo("Smokeview ");
//     SMV_EXIT(0);
//   }
//   if (CheckSMVFileLua(smv_filename, smokeview_casedir) == 0) {
//     SMV_EXIT(1);
//   }
//   InitTextureDir();
//   InitScriptErrorFiles();
//   smokezippath = GetSmokeZipPath(smokeview_bindir);
// #ifdef WIN32
//   have_ffmpeg = HaveProg("ffmpeg -version> Nul 2>Nul");
//   have_ffplay = HaveProg("ffplay -version> Nul 2>Nul");
// #else
//   have_ffmpeg = HaveProg("ffmpeg -version >/dev/null 2>/dev/null");
//   have_ffplay = HaveProg("ffplay -version >/dev/null 2>/dev/null");
// #endif
//   DisplayVersionInfo("Smokeview ");

//   return 0;
// }

// /// @brief We can only take strings from the Lua interpreter as consts, as
// they
// /// are 'owned' by the Lua code and we should not change them in C. This
// /// function creates a copy that we can change in C.
// char **CopyArgv(const int argc, const char *const *argv_sv) {
//   char **argv_sv_non_const;
//   // Allocate pointers for list of smokeview arguments
//   NewMemory((void **)&argv_sv_non_const, argc * sizeof(char *));
//   // Allocate space for each smokeview argument
//   int i;
//   for (i = 0; i < argc; i++) {
//     int length = strlen(argv_sv[i]);
//     NewMemory((void **)&argv_sv_non_const[i], (length + 1) * sizeof(char));
//     strcpy(argv_sv_non_const[i], argv_sv[i]);
//   }
//   return argv_sv_non_const;
// }

// /// @brief The corresponding function to free the memory allocated by
// copy_argv. void FreeArgv(const int argc, char **argv_sv_non_const) {
//   // Free the memory allocated for each argument
//   int i;
//   for (i = 0; i < argc; i++) {
//     FREEMEMORY(argv_sv_non_const[i]);
//   }
//   // Free the memory allocated for the array
//   FREEMEMORY(argv_sv_non_const);
// }

// json_object *jsonrpc_SetupGlut(jrpc_context *context, json_object *params,
// json_object *id) {
//   int argc = lua_tonumber(L, 1);
//   const char *const *argv_sv = lua_topointer(L, 2);
//   // Here we must copy the arguments received from the Lua interperter to
//   // allow them to be non-const (i.e. let the C code modify them).
//   char **argv_sv_non_const = CopyArgv(argc, argv_sv);
//   InitStartupDirs();
//   SetupGlut(argc, argv_sv_non_const);
//   FreeArgv(argc, argv_sv_non_const);
//   return 0;
// }

// json_object *jsonrpc_SetupCase(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *filename = lua_tostring(L, -1);
//   char *filename_mut;
//   // Allocate some new memory in case smv tries to modify it.
//   int f_len = strlen(filename);
//   if (NewMemory((void **)&filename_mut, sizeof(char) * f_len + 1) == 0)
//     return 2;
//   strncpy(filename_mut, filename, f_len);
//   filename_mut[f_len] = '\0';
//   int return_code = SetupCase(filename_mut);
//   lua_pushnumber(L, return_code);
//   FREEMEMORY(filename_mut);
//   return 1;
// }

// int RunLuaBranch(jrpc_context *context, json_object *params,  json_object
// *id, int argc, char **argv) {
//   int return_code;
//   SetStdOut(stdout);
//   initMALLOC();
//   InitRandAB(1000000);
//   // Setup the program, including parsing commandline arguments. Does not
//   // initialise any graphical components.
//   ProgramSetupLua(L, argc, argv);
//   // From here on out, control is passed to the lua interpreter. All further
//   // setup, including graphical display setup, is handled (or at least
//   // triggered) by the interpreter.
//   // TODO: currently the commands are issued here via C, but they are
//   designed
//   // such that they can be issued from lua.
//   // Setup glut. TODO: this is currently done via to C because the
//   commandline
//   // arguments are required for glutInit.

//   lua_pushnumber(L, argc);
//   lua_pushlightuserdata(L, argv);
//   LuaSetupGlut(L);
//   START_TIMER(startup_time);
//   // Load information about smokeview into the lua interpreter.
//   LuaInitsmvproginfo(L);

//   if (smv_filename == NULL) {
//     return 0;
//   }
//   lua_pushstring(L, smv_filename);
//   // TODO: only set up a case if one is specified, otherwise leave it to the
//   // interpreter to call this.
//   LuaSetupCase(L);
//   return_code = lua_tonumber(L, -1);
// #ifdef pp_HIST
//   if (return_code == 0 && update_bounds == 1) {
//     INIT_PRINT_TIMER(timer_update_bounds);
//     return_code = Update_Bounds();
//     PRINT_TIMER(timer_update_bounds, "Update_Bounds");
//   }
// #endif
//   if (return_code != 0) return 1;
//   if (convert_ini == 1) {
//     INIT_PRINT_TIMER(timer_read_ini);
//     ReadIni(ini_from);
//     PRINT_TIMER(timer_read_ini, "ReadIni");
//   }
//   if (runhtmlscript == 1) {
//     DoScriptHtml();
//   }
//   STOP_TIMER(startup_time);
//   if (runhtmlscript == 1) {
//     return 0;
//   }

//   glutMainLoop();
//   return 0;
// }

// // /// @brief Run a script.
// // /// @details There are two options for scripting, Lua and SSF. Which is
// run
// // is
// // /// set here based on the commandline arguments. If either (exclusive) of
// // these
// // /// values are set to true, then that script will run from within the
// display
// // /// callback (DisplayCB, in callbacks.c). These two loading routines are
// // /// included to load the scripts early in the piece, before the display
// // /// callback. Both runluascript and runscript are global.
// // int LoadScript(jrpc_context *context, json_object *params,  json_object
// *id, const char *filename) {
// //   if (runluascript == 1 && runscript == 1) {
// //     fprintf(stderr, "Both a Lua script and an SSF script cannot be run "
// //                     "simultaneously\n");
// //     exit(1);
// //   }
// //   if (runluascript == 1) {
// //     // Load the Lua script in order for it to be run later.
// //     if (LoadLuaScript(L, filename) != LUA_OK) {
// //       fprintf(stderr, "There was an error loading the script, and so it "
// //                       "will not run.\n");
// //       if (exit_on_script_crash) {
// //         exit(1); // exit with an error code
// //       }
// //       runluascript = 0; // set this to false so that the smokeview no
// longer
// //                         // tries to run the script as it failed to load
// //       fprintf(stderr, "Running smokeview normally.\n");
// //     }
// //     else {
// //       fprintf(stderr, "%s successfully loaded\n", filename);
// //     }
// //   }
// // #ifdef pp_LUA_SSF
// //   if (runscript == 1) {
// //     // Load the ssf script in order for it to be run later
// //     // This still uses the Lua interpreter
// //     if (loadSSFScript(filename) != LUA_OK) {
// //       fprintf(stderr, "There was an error loading the script, and so it "
// //                       "will not run.\n");
// //       if (exit_on_script_crash) {
// //         exit(1); // exit with an error code
// //       }
// //       runluascript = 0; // set this to false so that the smokeview no
// longer
// //                         // tries to run the script as it failed to load
// //       fprintf(stderr, "Running smokeview normally.\n");
// //     }
// //   }
// // #endif
// //   return 1;
// // }

// // /// @brief Load a .smv file. This is currently not used as it is dependent
// on
// // /// Smokeview being able to run without a .smv file loaded.
// // json_object *jsonrpc_Loadsmvall(jrpc_context *context, json_object
// *params, json_object *id) {
// //   // The first argument is taken from the stack as a string.
// //   const char *filepath = lua_tostring(L, 1);
// //   // The function from the C api is called using this string.
// //   Loadsmvall(filepath);
// //   // 0 arguments are returned.
// //   return 0;
// // }

// /// @brief Set render clipping.
// json_object *jsonrpc_Renderclip(jrpc_context *context, json_object *params,
// json_object *id) {
//   int flag = lua_toboolean(L, 1);
//   int left = lua_tonumber(L, 2);
//   int right = lua_tonumber(L, 3);
//   int bottom = lua_tonumber(L, 4);
//   int top = lua_tonumber(L, 5);
//   Renderclip(flag, left, right, bottom, top);
//   return 0;
// }

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

// json_object *jsonrpc_Gsliceview(jrpc_context *context, json_object *params,
// json_object *id) {
//   int data = lua_tonumber(L, 1);
//   int show_triangles = lua_toboolean(L, 2);
//   int show_triangulation = lua_toboolean(L, 3);
//   int show_normal = lua_toboolean(L, 4);
//   Gsliceview(data, show_triangles, show_triangulation, show_normal);
//   return 0;
// }

// json_object *jsonrpc_Showplot3ddata(jrpc_context *context, json_object
// *params, json_object *id) {
//   int meshnumber = lua_tonumber(L, 1);
//   int plane_orientation = lua_tonumber(L, 2);
//   int display = lua_tonumber(L, 3);
//   int showhide = lua_toboolean(L, 4);
//   float position = lua_tonumber(L, 5);
//   ShowPlot3dData(meshnumber, plane_orientation, display, showhide, position,
//   0); return 0;
// }

// json_object *jsonrpc_Gslicepos(jrpc_context *context, json_object *params,
// json_object *id) {
//   float x = lua_tonumber(L, 1);
//   float y = lua_tonumber(L, 2);
//   float z = lua_tonumber(L, 3);
//   Gslicepos(x, y, z);
//   return 0;
// }
// json_object *jsonrpc_Gsliceorien(jrpc_context *context, json_object *params,
// json_object *id) {
//   float az = lua_tonumber(L, 1);
//   float elev = lua_tonumber(L, 2);
//   Gsliceorien(az, elev);
//   return 0;
// }

// json_object *jsonrpc_Settourview(jrpc_context *context, json_object *params,
// json_object *id) {
//   int edittour_arg = lua_tonumber(L, 1);
//   int mode = lua_tonumber(L, 2);
//   int show_tourlocus_arg = lua_toboolean(L, 3);
//   float tour_global_tension_arg = lua_tonumber(L, 4);
//   Settourview(edittour_arg, mode, show_tourlocus_arg,
//   tour_global_tension_arg); return 0;
// }

// json_object *jsonrpc_Settourkeyframe(jrpc_context *context, json_object
// *params, json_object *id) {
//   float keyframe_time = lua_tonumber(L, 1);
//   Settourkeyframe(keyframe_time);
//   return 0;
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

// /// @brief By calling yieldscript, the script is suspended and the smokeview
// /// display is updated. It is necessary to call this before producing any
// /// outputs (such as renderings).
// json_object *jsonrpc_Yieldscript(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_yield(L, 0 /*zero results*/);
//   return 0;
// }

// /// @brief As with lua_yieldscript, but immediately resumes the script after
// /// letting the display callback run.
// json_object *jsonrpc_Tempyieldscript(jrpc_context *context, json_object
// *params, json_object *id) {
//   runluascript = 1;
//   lua_yield(L, 0 /*zero results*/);
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
  int f = json_object_get_int(params);
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
  float t = json_object_get_double(params);
  int return_code = Settime(t);
  if (return_code) {
    context->error_code = 112;
    context->error_message = strdup("render failure");
  }
  return NULL;
}

// /// @brief Load an FDS data file directly (i.e. as a filepath).
// json_object *jsonrpc_Loaddatafile(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *filename = lua_tostring(L, 1);
//   int return_value = Loadfile(filename);
//   lua_pushnumber(L, return_value);
//   return 1;
// }

// /// @brief Load a Smokeview config (.ini) file.
// json_object *jsonrpc_Loadinifile(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *filename = lua_tostring(L, 1);
//   Loadinifile(filename);
//   return 0;
// }

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

// /// @brief Load a slice file given the type of slice, the axis along which it
// /// exists and its position along this axis.
// json_object *jsonrpc_Loadslice(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *type = lua_tostring(L, 1);
//   int axis = lua_tonumber(L, 2);
//   float distance = lua_tonumber(L, 3);
//   Loadslice(type, axis, distance);
//   return 0;
// }

// /// @brief Load a slice based on its index in sliceinfo.
// json_object *jsonrpc_Loadsliceindex(jrpc_context *context, json_object
// *params, json_object *id) {
//   size_t index = lua_tonumber(L, 1);
//   int error = 0;
//   Loadsliceindex(index, &error);
//   if (error) {
//     return luaL_error(L, "Could not load slice at index %zu", index);
//   }
//   return 0;
// }

// json_object *jsonrpc_GetClippingMode(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_pushnumber(L, GetClippingMode());
//   return 1;
// }

// /// @brief Set the clipping mode, which determines which parts of the model
// are
// /// clipped (based on the set clipping values). This function takes an int,
// /// which is one
// ///  of:
// ///    0: No clipping.
// ///    1: Clip blockages and data.
// ///    2: Clip blockages.
// ///    3: Clip data.
// json_object *jsonrpc_SetClippingMode(jrpc_context *context, json_object
// *params, json_object *id) {
//   int mode = lua_tonumber(L, 1);
//   SetClippingMode(mode);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipX(jrpc_context *context, json_object
// *params, json_object *id) {
//   int clip_min = lua_toboolean(L, 1);
//   float min = lua_tonumber(L, 2);
//   int clip_max = lua_toboolean(L, 3);
//   float max = lua_tonumber(L, 4);
//   SetSceneclipX(clip_min, min, clip_max, max);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipXMin(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flag = lua_toboolean(L, 1);
//   float value = lua_tonumber(L, 2);
//   SetSceneclipXMin(flag, value);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipXMax(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flag = lua_toboolean(L, 1);
//   float value = lua_tonumber(L, 2);
//   SetSceneclipXMax(flag, value);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipY(jrpc_context *context, json_object
// *params, json_object *id) {
//   int clip_min = lua_toboolean(L, 1);
//   float min = lua_tonumber(L, 2);
//   int clip_max = lua_toboolean(L, 3);
//   float max = lua_tonumber(L, 4);
//   SetSceneclipY(clip_min, min, clip_max, max);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipYMin(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flag = lua_toboolean(L, 1);
//   float value = lua_tonumber(L, 2);
//   SetSceneclipYMin(flag, value);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipYMax(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flag = lua_toboolean(L, 1);
//   float value = lua_tonumber(L, 2);
//   SetSceneclipYMax(flag, value);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipZ(jrpc_context *context, json_object
// *params, json_object *id) {
//   int clip_min = lua_toboolean(L, 1);
//   float min = lua_tonumber(L, 2);
//   int clip_max = lua_toboolean(L, 3);
//   float max = lua_tonumber(L, 4);
//   SetSceneclipZ(clip_min, min, clip_max, max);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipZMin(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flag = lua_toboolean(L, 1);
//   float value = lua_tonumber(L, 2);
//   SetSceneclipZMin(flag, value);
//   return 0;
// }

// json_object *jsonrpc_SetSceneclipZMax(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flag = lua_toboolean(L, 1);
//   float value = lua_tonumber(L, 2);
//   SetSceneclipZMax(flag, value);
//   return 0;
// }

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

// json_object *jsonrpc_Getiblankcell(jrpc_context *context, json_object
// *params, json_object *id) {
//   // The offset in the global meshinfo table.
//   int mesh_index = lua_tonumber(L, lua_upvalueindex(1));
//   // The offsets into the mesh requested
//   int i = lua_tonumber(L, 1);
//   int j = lua_tonumber(L, 2);
//   int k = lua_tonumber(L, 3);

//   meshdata *mesh = &meshinfo[mesh_index];
//   char iblank =
//       mesh->c_iblank_cell[(i) + (j)*mesh->ibar + (k)*mesh->ibar *
//       mesh->jbar];
//   if (iblank == GAS) {
//     lua_pushboolean(L, 1);
//   }
//   else {
//     lua_pushboolean(L, 0);
//   }
//   return 1;
// }

// json_object *jsonrpc_Getiblanknode(jrpc_context *context, json_object
// *params, json_object *id) {
//   // The offset in the global meshinfo table.
//   int mesh_index = lua_tonumber(L, lua_upvalueindex(1));
//   // The offsets into the mesh requested.
//   int i = lua_tonumber(L, 1);
//   int j = lua_tonumber(L, 2);
//   int k = lua_tonumber(L, 3);

//   meshdata *mesh = &meshinfo[mesh_index];
//   char iblank = mesh->c_iblank_node[(i) + (j) * (mesh->ibar + 1) +
//                                     (k) * (mesh->ibar + 1) * (mesh->jbar +
//                                     1)];
//   if (iblank == GAS) {
//     lua_pushboolean(L, 1);
//   }
//   else {
//     lua_pushboolean(L, 0);
//   }
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

// /// @brief Sum bounded data in a given mesh
// json_object *jsonrpc_GetQdataSumBounded(jrpc_context *context, json_object
// *params, json_object *id) {
//   int meshnumber = lua_tonumber(L, 1);
//   int vari, i, j, k;
//   int i1, i2, j1, j2, k1, k2;
//   i1 = lua_tonumber(L, 2);
//   i2 = lua_tonumber(L, 3);
//   j1 = lua_tonumber(L, 4);
//   j2 = lua_tonumber(L, 5);
//   k1 = lua_tonumber(L, 6);
//   k2 = lua_tonumber(L, 7);
//   meshdata mesh = meshinfo[meshnumber];
//   int ntotal = (mesh.ibar + 1) * (mesh.jbar + 1) * (mesh.kbar + 1);
//   int bounded_total = (i2 - i1 + 1) * (j2 - j1 + 1) * (k2 - k1 + 1);
//   int vars = 5;
//   float totals[5];
//   totals[0] = 0.0;
//   totals[1] = 0.0;
//   totals[2] = 0.0;
//   totals[3] = 0.0;
//   totals[4] = 0.0;
//   for (vari = 0; vari < 5; ++vari) {
//     int offset = vari * ntotal;
//     for (k = k1; k <= k2; ++k) {
//       for (j = j1; j <= j2; ++j) {
//         for (i = i1; i <= i2; ++i) {
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
//   lua_pushnumber(L, bounded_total);
//   return vars + 1;
// }

// /// @brief Sum bounded data in a given mesh
// json_object *jsonrpc_GetQdataMaxBounded(jrpc_context *context, json_object
// *params, json_object *id) {
//   int meshnumber = lua_tonumber(L, 1);
//   int vari, i, j, k;
//   int i1, i2, j1, j2, k1, k2;
//   i1 = lua_tonumber(L, 2);
//   i2 = lua_tonumber(L, 3);
//   j1 = lua_tonumber(L, 4);
//   j2 = lua_tonumber(L, 5);
//   k1 = lua_tonumber(L, 6);
//   k2 = lua_tonumber(L, 7);
//   meshdata mesh = meshinfo[meshnumber];
//   int ntotal = (mesh.ibar + 1) * (mesh.jbar + 1) * (mesh.kbar + 1);
//   int bounded_total = (i2 - i1 + 1) * (j2 - j1 + 1) * (k2 - k1 + 1);
//   int vars = 5;
//   float maxs[5];
//   maxs[0] = -1 * FLT_MAX;
//   maxs[1] = -1 * FLT_MAX;
//   maxs[2] = -1 * FLT_MAX;
//   maxs[3] = -1 * FLT_MAX;
//   maxs[4] = -1 * FLT_MAX;
//   for (vari = 0; vari < 5; ++vari) {
//     int offset = vari * ntotal;
//     for (k = k1; k <= k2; ++k) {
//       for (j = j1; j <= j2; ++j) {
//         for (i = i1; i <= i2; ++i) {
//           int n = offset + k * (mesh.jbar + 1) * (mesh.ibar + 1) +
//                   j * (mesh.ibar + 1) + i;
//           if (maxs[vari] < mesh.qdata[n]) {
//             maxs[vari] = mesh.qdata[n];
//           }
//         }
//       }
//     }
//   }

//   for (vari = 0; vari < vars; ++vari) {
//     lua_pushnumber(L, maxs[vari]);
//   }
//   lua_pushnumber(L, bounded_total);
//   return vars + 1;
// }

// json_object *jsonrpc_GetQdataMean(jrpc_context *context, json_object *params,
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
//     lua_pushnumber(L, totals[vari] / ntotal);
//   }
//   return vars;
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

// // TODO: remove this from a hardcoded string.
// int SetupPl3dtables(jrpc_context *context, json_object *params,  json_object
// *id) {
//   luaL_dostring(L, "\
//     pl3d = {}\
//     local allpl3dtimes = {}\
//     for i,v in ipairs(plot3dinfo) do\
//         if not allpl3dtimes[v.time] then allpl3dtimes[v.time] = {} end\
//         assert(not allpl3dtimes[v.time][v.blocknumber+1])\
//         allpl3dtimes[v.time][v.blocknumber+1] = v\
//     end\
//     local pl3dtimes = {}\
//     for k,v in pairs(allpl3dtimes) do\
//         pl3dtimes[#pl3dtimes+1] = {time = k, entries = v}\
//     end\
//     table.sort( pl3dtimes, function(a,b) return a.time < b.time end)\
//     pl3d.entries = plot3dinfo\
//     pl3d.frames = pl3dtimes");
//   return 0;
// }

// json_object *jsonrpc_CaseTitle(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushstring(L, "chid");
//   lua_gettable(L, 1);
//   const char *chid = lua_tostring(L, -1);
//   const char *name = lua_tostring(L, 2);
//   lua_pushfstring(L, "%s for %s", name, chid);
//   return 1;
// }

// json_object *jsonrpc_CaseIndex(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *field = lua_tostring(L, 2);
//   if (strcmp(field, "chid") == 0) {
//     lua_pushstring(L, chidfilebase);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_CaseNewindex(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *field = lua_tostring(L, 2);
//   if (strcmp(field, "chid") == 0) {
//     luaL_error(L, "case.chid is read-only");
//     // lua_pushstring(L, value);
//     // lua_setrenderdir(L);
//     return 0;
//   }
//   else {
//     return 0;
//   }
// }

// /// @brief Load data about the loaded module into the lua interpreter. This
// /// initsmvdata is necessary to bring some data into the Lua interpreter from
// /// the model. This is included here rather than doing in the Smokeview code
// to
// /// increase separation. This will likely be removed in future versions.
// // TODO: Consider converting most of these to userdata, rather than copying
// them
// // into the lua interpreter.
// json_object *jsonrpc_CreateCase(jrpc_context *context, json_object *params,
// json_object *id) {
//   // Create case table
//   lua_newtable(L);
//   // lua_pushstring(L, chidfilebase);
//   // lua_setfield(L, -2, "chid");
//   lua_pushstring(L, fdsprefix);
//   lua_setfield(L, -2, "fdsprefix");

//   lua_pushcfunction(L, &jsonrpc_CaseTitle);
//   lua_setfield(L, -2, "plot_title");

//   // TODO: copying the array into lua allows for slightly faster access,
//   // but is less ergonomic, leave direct access as the default, with copying
//   // in cases where it is shown to be a useful speedup
//   // lua_get_global_times(L);
//   // global_times is currently on the stack
//   // add a metatable to it.
//   // first create the table

//   // Create "global_times" table
//   lua_newtable(L);
//   // Create "global_times" metatable
//   lua_newtable(L);
//   lua_pushcfunction(L, &jsonrpc_GetNglobalTimes);
//   lua_setfield(L, -2, "__len");
//   lua_pushcfunction(L, &jsonrpc_GetGlobalTimeN);
//   lua_setfield(L, -2, "__index");
//   // then set the metatable
//   lua_setmetatable(L, -2);
//   lua_setfield(L, -2, "global_times");

//   // while the meshes themselve will rarely change, the information about
//   them
//   // will change regularly. This is handled by the mesh table.
//   LuaGetMeshes(L);
//   // meshes is currently on the stack
//   // add a metatable to it.
//   // first create the table
//   lua_createtable(L, 0, 1);
//   lua_pushcfunction(L, &jsonrpc_GetNmeshes);
//   lua_setfield(L, -2, "__len");
//   // then set the metatable
//   lua_setmetatable(L, -2);
//   lua_setfield(L, -2, "meshes");

//   // As with meshes the number and names of devices is unlikely to change
//   LuaGetDevices(L);
//   // devices is currently on the stack
//   // add a metatable to it.
//   // first create the table
//   lua_createtable(L, 0, 1);
//   lua_pushcfunction(L, &jsonrpc_GetNdevices);
//   lua_setfield(L, -2, "__len");
//   // then set the metatable
//   lua_setmetatable(L, -2);
//   lua_setfield(L, -2, "devices");

//   // sliceinfo is a 1-indexed array so the lua length operator
//   // works without the need for a metatable
//   LuaGetSliceinfo(L);
//   lua_setfield(L, -2, "slices");

//   // lua_get_rampinfo(L);
//   // lua_setglobal(L, "rampinfo");

//   LuaGetCsvinfo(L);
//   // csvinfo is currently on the stack
//   // add a metatable to it.
//   // first create the table
//   lua_createtable(L, 0, 1);
//   lua_pushcfunction(L, &jsonrpc_GetNcsvinfo);
//   lua_setfield(L, -2, "__len");
//   // then set the metatable
//   lua_setmetatable(L, -2);
//   lua_setfield(L, -2, "csvs");

//   LuaGetPlot3dinfo(L);
//   // plot3dinfo is currently on the stack
//   // add a metatable to it.
//   // first create the table
//   lua_createtable(L, 0, 1);
//   lua_pushcfunction(L, &jsonrpc_GetNplot3dinfo);
//   lua_setfield(L, -2, "__len");
//   // then set the metatable
//   lua_setmetatable(L, -2);
//   lua_setfield(L, -2, "pl3ds");

//   // set up tables to access pl3dinfo better
//   // setup_pl3dtables(L);

//   // lua_get_geomdata(L);
//   // lua_setglobal(L, "geomdata");

//   // Case metatable
//   lua_createtable(L, 0, 1);
//   lua_pushcfunction(L, &jsonrpc_CaseIndex);
//   lua_setfield(L, -2, "__index");
//   lua_pushcfunction(L, &jsonrpc_CaseNewindex);
//   lua_setfield(L, -2, "__newindex");
//   lua_setmetatable(L, -2);

//   return 1;
// }

// /// @brief As with lua_initsmvdata(), but for information relating to
// Smokeview
// /// itself.
// json_object *jsonrpc_Initsmvproginfo(jrpc_context *context, json_object
// *params, json_object *id) {
//   char version[256];
//   // char githash[256];

//   GetProgVersion(version);
//   AddLuaPaths(L);
//   // getGitHash(githash);

//   lua_createtable(L, 0, 6);

//   lua_pushstring(L, version);
//   lua_setfield(L, -2, "version");

//   // lua_pushstring(L, githash);
//   // lua_setfield(L, -2, "githash");

//   lua_pushstring(L, __DATE__);
//   lua_setfield(L, -2, "builddate");

//   lua_pushstring(L, fds_githash);
//   lua_setfield(L, -2, "fdsgithash");

//   lua_pushstring(L, smokeviewpath);
//   lua_setfield(L, -2, "smokeviewpath");

//   lua_pushstring(L, smokezippath);
//   lua_setfield(L, -2, "smokezippath");

//   lua_pushstring(L, texturedir);
//   lua_setfield(L, -2, "texturedir");

//   lua_setglobal(L, "smokeviewProgram");
//   return 0;
// }

// json_object *jsonrpc_GetSlice(jrpc_context *context, json_object *params,
// json_object *id) {
//   // This should push a lightuserdata onto the stack which is a pointer to
//   the
//   // slicedata. This takes the index of the slice (in the sliceinfo array) as
//   an
//   // argument.
//   // Get the index of the slice as an argument to the lua function.
//   int slice_index = lua_tonumber(L, 1);
//   // Get the pointer to the slicedata struct.
//   slicedata *slice = &sliceinfo[slice_index];
//   // Push the pointer onto the lua stack as lightuserdata.
//   lua_pushlightuserdata(L, slice);
//   // lua_newuserdata places the data on the stack, so return a single stack
//   // item.
//   return 1;
// }

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

// // int lua_get_all_part_

// json_object *jsonrpc_SliceDataMapFrames(jrpc_context *context, json_object
// *params, json_object *id) {
//   // The first argument to this function is the slice pointer. This function
//   // receives the values of the slice at a particular frame as an array.
//   slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   if (!slice->loaded) {
//     return luaL_error(L, "slice %s not loaded", slice->file);
//   }
//   int framepoints = slice->nslicex * slice->nslicey;
//   // Pointer to the first frame.
//   float *qslicedata = slice->qslicedata;
//   // The second argument is the function to be called on each frame.
//   lua_createtable(L, slice->ntimes, 0);
//   // framenumber is the index of the frame (0-based).
//   int framenumber;
//   for (framenumber = 0; framenumber < slice->ntimes; framenumber++) {
//     // duplicate the function so that we can use it and keep it
//     lua_pushvalue(L, 2);
//     // Push the first frame onto the stack by first putting them into a lua
//     // table. Values are indexed from 1.
//     // Feed the lua function a lightuserdata (pointer) that is can use
//     // with a special function to index the array.
//     lua_pushnumber(L, framepoints);
//     // lua_pushlightuserdata(L, &qslicedata[framenumber*framepoints]);

//     // this table method is more flexible but slower
//     lua_createtable(L, framepoints, 0);
//     // pointnumber is the index of the data point in the frame (0-based).
//     int pointnumber;
//     for (pointnumber = 0; pointnumber < framepoints; pointnumber++) {
//       // adjust the index to start from 1
//       lua_pushnumber(L, pointnumber + 1);
//       lua_pushnumber(L, qslicedata[framenumber * framepoints + pointnumber]);
//       lua_settable(L, -3);
//     }

//     // The function takes 2 arguments and returns 1 result.
//     lua_call(L, 2, 1);
//     // Add the value to the results table.
//     lua_pushnumber(L, framenumber + 1);
//     lua_pushvalue(L, -2);
//     lua_settable(L, -4);
//     lua_pop(L, 1);
//   }
//   // Return a table of values.
//   return 1;
// }

// json_object *jsonrpc_SliceDataMapFramesCountLess(jrpc_context *context,
// json_object *params,  json_object *id) {
//   slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   if (!slice->loaded) {
//     return luaL_error(L, "slice %s not loaded", slice->file);
//   }
//   float threshold = lua_tonumber(L, 2);
//   int framepoints = slice->nslicex * slice->nslicey;
//   // Pointer to the first frame.
//   float *qslicedata = slice->qslicedata;
//   lua_createtable(L, slice->ntimes, 0);
//   int framenumber;
//   for (framenumber = 0; framenumber < slice->ntimes; framenumber++) {
//     int count = 0;
//     int pointnumber;
//     for (pointnumber = 0; pointnumber < framepoints; pointnumber++) {
//       if (*qslicedata < threshold) {
//         count++;
//       }
//       qslicedata++;
//     }
//     lua_pushnumber(L, framenumber + 1);
//     lua_pushnumber(L, count);
//     lua_settable(L, -3);
//   }
//   // Return a table of values.
//   return 1;
// }

// json_object *jsonrpc_SliceDataMapFramesCountLessEq(jrpc_context *context,
// json_object *params,  json_object *id) {
//   slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   if (!slice->loaded) {
//     return luaL_error(L, "slice %s not loaded", slice->file);
//   }
//   float threshold = lua_tonumber(L, 2);
//   int framepoints = slice->nslicex * slice->nslicey;
//   // Pointer to the first frame.
//   float *qslicedata = slice->qslicedata;
//   lua_createtable(L, slice->ntimes, 0);
//   int framenumber;
//   for (framenumber = 0; framenumber < slice->ntimes; framenumber++) {
//     int count = 0;
//     int pointnumber;
//     for (pointnumber = 0; pointnumber < framepoints; pointnumber++) {
//       if (*qslicedata <= threshold) {
//         count++;
//       }
//       qslicedata++;
//     }
//     lua_pushnumber(L, framenumber + 1);
//     lua_pushnumber(L, count);
//     lua_settable(L, -3);
//   }
//   // Return a table of values.
//   return 1;
// }

// json_object *jsonrpc_SliceDataMapFramesCountGreater(jrpc_context *context,
// json_object *jsonrpc_params,  json_object *id) {
//   slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   if (!slice->loaded) {
//     return luaL_error(L, "slice %s not loaded", slice->file);
//   }
//   float threshold = lua_tonumber(L, 2);
//   int framepoints = slice->nslicex * slice->nslicey;
//   // Pointer to the first frame.
//   float *qslicedata = slice->qslicedata;
//   lua_createtable(L, slice->ntimes, 0);
//   int framenumber;
//   for (framenumber = 0; framenumber < slice->ntimes; framenumber++) {
//     int count = 0;
//     int pointnumber;
//     for (pointnumber = 0; pointnumber < framepoints; pointnumber++) {
//       if (*qslicedata > threshold) {
//         count++;
//       }
//       qslicedata++;
//     }
//     lua_pushnumber(L, framenumber + 1);
//     lua_pushnumber(L, count);
//     lua_settable(L, -3);
//   }
//   // Return a table of values.
//   return 1;
// }

// json_object *jsonrpc_SliceDataMapFramesCountGreaterEq(jrpc_context *context,
// json_object *jsonrpc_params,  json_object *id) {
//   slicedata *slice = (slicedata *)lua_touserdata(L, 1);
//   if (!slice->loaded) {
//     return luaL_error(L, "slice %s not loaded", slice->file);
//   }
//   float threshold = lua_tonumber(L, 2);
//   int framepoints = slice->nslicex * slice->nslicey;
//   // Pointer to the first frame.
//   float *qslicedata = slice->qslicedata;
//   lua_createtable(L, slice->ntimes, 0);
//   int framenumber;
//   for (framenumber = 0; framenumber < slice->ntimes; framenumber++) {
//     int count = 0;
//     int pointnumber;
//     for (pointnumber = 0; pointnumber < framepoints; pointnumber++) {
//       if (*qslicedata >= threshold) {
//         count++;
//       }
//       qslicedata++;
//     }
//     lua_pushnumber(L, framenumber + 1);
//     lua_pushnumber(L, count);
//     lua_settable(L, -3);
//   }
//   // Return a table of values.
//   return 1;
// }

// /// @brief Pushes a value from a slice onto the stack (a single slice, not
// /// multi). The arguments are
// /// 1. int framenumber
// /// 2. int i
// /// 3. int j
// /// 4. ink k
// /// The slice index is stored as part of a closure.
// json_object *jsonrpc_Getslicedata(jrpc_context *context, json_object *params,
// json_object *id) {
//   // The offset in the global sliceinfo table of the slice.
//   int slice_index = lua_tonumber(L, lua_upvalueindex(1));
//   // The time frame to use
//   int f = lua_tonumber(L, 1);
//   // The offsets into the mesh requested (NOT the data array)
//   int i = lua_tonumber(L, 2);
//   int j = lua_tonumber(L, 3);
//   int k = lua_tonumber(L, 4);
//   // printf("getting slice data: %d, %d, %d-%d-%d\n", slice_index, f, i, j,
//   k);
//   // print all the times
//   // printf("times: %d\n", sliceinfo[slice_index].ntimes);
//   // int n = 0;
//   // for (n; n < sliceinfo[slice_index].ntimes; n++) {
//   //   fprintf(stderr, "t:%.2f s\n", sliceinfo[slice_index].times[n]);
//   // }
//   // fprintf(stderr, "f:%d i:%d j:%d  k:%d\n", f, i,j,k);

//   int imax = sliceinfo[slice_index].ijk_max[0];
//   int jmax = sliceinfo[slice_index].ijk_max[1];
//   int kmax = sliceinfo[slice_index].ijk_max[2];

//   int di = sliceinfo[slice_index].nslicei;
//   int dj = sliceinfo[slice_index].nslicej;
//   int dk = sliceinfo[slice_index].nslicek;
//   // Check that the offsets do not exceed the bounds of a single data frame
//   if (i > imax || j > jmax || k > kmax) {
//     fprintf(stderr, "ERROR: offsets exceed bounds");
//     exit(1);
//   }
//   // Convert the offsets into the mesh into offsets into the data array
//   int i_offset = i - sliceinfo[slice_index].ijk_min[0];
//   int j_offset = j - sliceinfo[slice_index].ijk_min[1];
//   int k_offset = k - sliceinfo[slice_index].ijk_min[2];

//   // Offset into a single frame
//   int offset = (dk * dj) * i_offset + dk * j_offset + k_offset;
//   int framesize = di * dj * dk;
//   float val = sliceinfo[slice_index].qslicedata[offset + f * framesize];
//   // lua_pushstring(L,sliceinfo[slice_index].file);
//   lua_pushnumber(L, val);
//   return 1;
// }

// /// @brief Build a Lua table with information on the slices of the model.
// // TODO: change this to use userdata instead
// json_object *jsonrpc_GetSliceinfo(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_createtable(L, 0, nsliceinfo);
//   int i;
//   for (i = 0; i < nsliceinfo; i++) {
//     lua_pushnumber(L, i + 1);
//     lua_createtable(L, 0, 21);

//     if (sliceinfo[i].slicelabel != NULL) {
//       lua_pushstring(L, sliceinfo[i].slicelabel);
//       lua_setfield(L, -2, "label");
//     }

//     lua_pushnumber(L, i);
//     lua_setfield(L, -2, "n");

//     if (sliceinfo[i].label.longlabel != NULL) {
//       lua_pushstring(L, sliceinfo[i].label.longlabel);
//       lua_setfield(L, -2, "longlabel");
//     }

//     if (sliceinfo[i].label.shortlabel != NULL) {
//       lua_pushstring(L, sliceinfo[i].label.shortlabel);
//       lua_setfield(L, -2, "shortlabel");
//     }

//     lua_pushstring(L, sliceinfo[i].file);
//     lua_setfield(L, -2, "file");

//     lua_pushnumber(L, sliceinfo[i].slice_filetype);
//     lua_setfield(L, -2, "slicefile_type");

//     lua_pushnumber(L, sliceinfo[i].idir);
//     lua_setfield(L, -2, "idir");

//     lua_pushnumber(L, sliceinfo[i].sliceoffset);
//     lua_setfield(L, -2, "sliceoffset");

//     lua_pushnumber(L, sliceinfo[i].ijk_min[0]);
//     lua_setfield(L, -2, "imin");

//     lua_pushnumber(L, sliceinfo[i].ijk_max[0]);
//     lua_setfield(L, -2, "imax");

//     lua_pushnumber(L, sliceinfo[i].ijk_min[1]);
//     lua_setfield(L, -2, "jmin");

//     lua_pushnumber(L, sliceinfo[i].ijk_max[1]);
//     lua_setfield(L, -2, "jmax");

//     lua_pushnumber(L, sliceinfo[i].ijk_min[2]);
//     lua_setfield(L, -2, "kmin");

//     lua_pushnumber(L, sliceinfo[i].ijk_max[2]);
//     lua_setfield(L, -2, "kmax");

//     lua_pushnumber(L, sliceinfo[i].blocknumber);
//     lua_setfield(L, -2, "blocknumber");

//     lua_pushnumber(L, sliceinfo[i].position_orig);
//     lua_setfield(L, -2, "position_orig");

//     lua_pushnumber(L, sliceinfo[i].nslicex);
//     lua_setfield(L, -2, "nslicex");

//     lua_pushnumber(L, sliceinfo[i].nslicey);
//     lua_setfield(L, -2, "nslicey");

//     lua_pushstring(L, sliceinfo[i].cdir);
//     lua_setfield(L, -2, "slicedir");

//     // can't be done until loaded
//     // lua_pushnumber(L, sliceinfo[i].ntimes);
//     // lua_setfield(L, -2, "ntimes");

//     // Push the slice index so that getslicedata knows which slice to operate
//     // on.
//     lua_pushnumber(L, i);
//     // Push a closure which has been provided with the first argument (the
//     slice
//     // index)
//     lua_pushcclosure(L, LuaGetslicedata, 1);
//     lua_setfield(L, -2, "getdata");

//     lua_settable(L, -3);
//   }
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

// json_object *jsonrpc_Load3dsmoke(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *smoke_type = lua_tostring(L, 1);
//   Load3dsmoke(smoke_type);
//   return 0;
// }

// json_object *jsonrpc_Loadvolsmoke(jrpc_context *context, json_object *params,
// json_object *id) {
//   int meshnumber = lua_tonumber(L, 1);
//   Loadvolsmoke(meshnumber);
//   return 0;
// }

// json_object *jsonrpc_Loadvolsmokeframe(jrpc_context *context, json_object
// *params, json_object *id) {
//   int meshnumber = lua_tonumber(L, 1);
//   int framenumber = lua_tonumber(L, 1);
//   Loadvolsmokeframe(meshnumber, framenumber, 1);
//   // returnval = 1; // TODO: determine if this is the correct behaviour.
//   // this is what is done in the SSF code.
//   return 0;
// }

// /// @brief Set the format of images which will be exported. The value should
// be
// /// a string. The acceptable values are:
// ///   "JPG"
// ///   "PNG"
// json_object *jsonrpc_SetRendertype(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *type = lua_tostring(L, 1);
//   if (SetRendertype(type)) {
//     return luaL_error(L, "%s is not a valid render type", type);
//   }
//   return 0;
// }

// json_object *jsonrpc_GetRendertype(jrpc_context *context, json_object
// *params, json_object *id) {
//   int render_type = GetRendertype();
//   switch (render_type) {
//   case JPEG:
//     lua_pushstring(L, "JPG");
//     break;
//   case PNG:
//     lua_pushstring(L, "PNG");
//     break;
//   default:
//     lua_pushstring(L, NULL);
//     break;
//   }
//   return 1;
// }

// /// @brief Set the format of movies which will be exported. The value should
// be
// /// a string. The acceptable values are:
// ///    - "WMV"
// ///    - "MP4"
// ///    - "AVI"
// json_object *jsonrpc_SetMovietype(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *type = lua_tostring(L, 1);
//   SetMovietype(type);
//   return 0;
// }

// json_object *jsonrpc_GetMovietype(jrpc_context *context, json_object *params,
// json_object *id) {
//   int movie_type = GetMovietype();
//   switch (movie_type) {
//   case WMV:
//     lua_pushstring(L, "WMV");
//     break;
//   case MP4:
//     lua_pushstring(L, "MP4");
//     break;
//   case AVI:
//     lua_pushstring(L, "AVI");
//     break;
//   default:
//     lua_pushstring(L, NULL);
//     break;
//   }
//   return 1;
// }

// json_object *jsonrpc_Makemovie(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *name = lua_tostring(L, 1);
//   const char *base = lua_tostring(L, 2);
//   float framerate = lua_tonumber(L, 3);
//   Makemovie(name, base, framerate);
//   return 0;
// }

// json_object *jsonrpc_Loadtour(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *name = lua_tostring(L, 1);
//   int error_code = Loadtour(name);
//   lua_pushnumber(L, error_code);
//   return 1;
// }

// json_object *jsonrpc_Loadparticles(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *name = lua_tostring(L, 1);
//   Loadparticles(name);
//   return 0;
// }

// json_object *jsonrpc_Partclasscolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *color = lua_tostring(L, 1);
//   Partclasscolor(color);
//   return 0;
// }

// json_object *jsonrpc_Partclasstype(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *type = lua_tostring(L, 1);
//   Partclasstype(type);
//   return 0;
// }

// json_object *jsonrpc_Plot3dprops(jrpc_context *context, json_object *params,
// json_object *id) {
//   int variable_index = lua_tonumber(L, 1);
//   int showvector = lua_toboolean(L, 2);
//   int vector_length_index = lua_tonumber(L, 3);
//   int display_type = lua_tonumber(L, 4);
//   float vector_length = lua_tonumber(L, 5);
//   Plot3dprops(variable_index, showvector, vector_length_index, display_type,
//               vector_length);
//   return 0;
// }

// json_object *jsonrpc_Loadplot3d(jrpc_context *context, json_object *params,
// json_object *id) {
//   int meshnumber = lua_tonumber(L, 1);
//   float time_local = lua_tonumber(L, 2);
//   Loadplot3d(meshnumber, time_local);
//   return 0;
// }

// json_object *jsonrpc_Unloadall(jrpc_context *context, json_object *params,
// json_object *id) {
//   Unloadall();
//   return 0;
// }

// json_object *jsonrpc_Unloadtour(jrpc_context *context, json_object *params,
// json_object *id) {
//   Unloadtour();
//   return 0;
// }

// json_object *jsonrpc_Setrenderdir(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *dir = lua_tostring(L, -1);
//   int return_code = Setrenderdir(dir);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_Getrenderdir(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushstring(L, script_dir_path);
//   return 1;
// }

// json_object *jsonrpc_SetOrthoPreset(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *viewpoint = lua_tostring(L, 1);
//   int errorcode = SetOrthoPreset(viewpoint);
//   lua_pushnumber(L, errorcode);
//   return 1;
// }

// json_object *jsonrpc_Setviewpoint(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *viewpoint = lua_tostring(L, 1);
//   int errorcode = Setviewpoint(viewpoint);
//   lua_pushnumber(L, errorcode);
//   return 1;
// }

// json_object *jsonrpc_Getviewpoint(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushstring(L, camera_current->name);
//   return 1;
// }

// json_object *jsonrpc_ExitSmokeview(jrpc_context *context, json_object
// *params, json_object *id) {
//   ExitSmokeview();
//   return 0;
// }

// json_object *jsonrpc_Setwindowsize(jrpc_context *context, json_object
// *params, json_object *id) {
//   int width = lua_tonumber(L, 1);
//   int height = lua_tonumber(L, 2);
//   Setwindowsize(width, height);
//   // Using the DisplayCB is not sufficient in this case,
//   // control must be temporarily returned to the main glut loop.
//   LuaTempyieldscript(L);
//   return 0;
// }

// json_object *jsonrpc_Setgridvisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int selection = lua_tonumber(L, 1);
//   Setgridvisibility(selection);
//   return 0;
// }

// json_object *jsonrpc_Setgridparms(jrpc_context *context, json_object *params,
// json_object *id) {
//   int x_vis = lua_tonumber(L, 1);
//   int y_vis = lua_tonumber(L, 2);
//   int z_vis = lua_tonumber(L, 3);

//   int x_plot = lua_tonumber(L, 4);
//   int y_plot = lua_tonumber(L, 5);
//   int z_plot = lua_tonumber(L, 6);

//   Setgridparms(x_vis, y_vis, z_vis, x_plot, y_plot, z_plot);

//   return 0;
// }

// json_object *jsonrpc_Setcolorbarflip(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flip = lua_toboolean(L, 1);
//   Setcolorbarflip(flip);
//   LuaTempyieldscript(L);
//   return 0;
// }

// json_object *jsonrpc_Getcolorbarflip(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flip = Getcolorbarflip();
//   lua_pushboolean(L, flip);
//   return 1;
// }

// json_object *jsonrpc_Setcolorbarindex(jrpc_context *context, json_object
// *params, json_object *id) {
//   int chosen_index = lua_tonumber(L, 1);
//   Setcolorbarindex(chosen_index);
//   return 0;
// }

// json_object *jsonrpc_Getcolorbarindex(jrpc_context *context, json_object
// *params, json_object *id) {
//   int index = Getcolorbarindex();
//   lua_pushnumber(L, index);
//   return 1;
// }

// json_object *jsonrpc_SetSliceInObst(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetSliceInObst(setting);
//   return 0;
// }

// json_object *jsonrpc_GetSliceInObst(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = GetSliceInObst();
//   lua_pushboolean(L, setting);
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

// json_object *jsonrpc_ToggleColorbarVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleColorbarVisibility();
//   return 0;
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

// json_object *jsonrpc_ToggleColorbarVisibilityHorizontal(jrpc_context
// *context, json_object *jsonrpc_params,  json_object *id) {
//   ToggleColorbarVisibilityHorizontal();
//   return 0;
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

// json_object *jsonrpc_ToggleColorbarVisibilityVertical(jrpc_context *context,
// json_object *jsonrpc_params,  json_object *id) {
//   ToggleColorbarVisibilityVertical();
//   return 0;
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

// json_object *jsonrpc_ToggleTimebarVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleTimebarVisibility();
//   return 0;
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

// json_object *jsonrpc_ToggleTitleVisibility(jrpc_context *context, json_object
// *params,  json_object *id) {
//   ToggleTitleVisibility();
//   return 0;
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

// json_object *jsonrpc_ToggleSmvVersionVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleSmvVersionVisibility();
//   return 0;
// }

// // chid
// json_object *jsonrpc_SetChidVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetChidVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetChidVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = GetChidVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_ToggleChidVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   ToggleChidVisibility();
//   return 0;
// }

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

// json_object *jsonrpc_ToggleAxisVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   ToggleAxisVisibility();
//   return 0;
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

// json_object *jsonrpc_ToggleFramelabelVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleFramelabelVisibility();
//   return 0;
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

// json_object *jsonrpc_ToggleFramerateVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleFramerateVisibility();
//   return 0;
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

// json_object *jsonrpc_ToggleGridlocVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleGridlocVisibility();
//   return 0;
// }

// // hrrpuv cutoff
// json_object *jsonrpc_SetHrrcutoffVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetHrrcutoffVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetHrrcutoffVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetHrrcutoffVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_ToggleHrrcutoffVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleHrrcutoffVisibility();
//   return 0;
// }

// // HRR Label Visbility
// json_object *jsonrpc_SetHrrlabelVisibility(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetHrrlabelVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetHrrlabelVisibility(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int setting = GetHrrlabelVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_ToggleHrrlabelVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleHrrlabelVisibility();
//   return 0;
// }
// // memory load
// #ifdef pp_memstatus
// json_object *jsonrpc_SetMemloadVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   set_memload_visibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetMemloadVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   int setting = get_memload_visibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_ToggleMemloadVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   toggle_memload_visibility();
//   return 0;
// }
// #endif

// // mesh label
// json_object *jsonrpc_SetMeshlabelVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetMeshlabelVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetMeshlabelVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetMeshlabelVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_ToggleMeshlabelVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleMeshlabelVisibility();
//   return 0;
// }

// // slice average
// json_object *jsonrpc_SetSliceAverageVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetSliceAverageVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetSliceAverageVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetSliceAverageVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_ToggleSliceAverageVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleSliceAverageVisibility();
//   return 0;
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

// json_object *jsonrpc_ToggleTimeVisibility(jrpc_context *context, json_object
// *params, json_object *id) {
//   ToggleTimeVisibility();
//   return 0;
// }

// // user settable ticks
// json_object *jsonrpc_SetUserTicksVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetUserTicksVisibility(setting);
//   return 0;
// }

// json_object *jsonrpc_GetUserTicksVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int setting = GetUserTicksVisibility();
//   lua_pushboolean(L, setting);
//   return 1;
// }

// json_object *jsonrpc_ToggleUserTicksVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleUserTicksVisibility();
//   return 0;
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

// json_object *jsonrpc_ToggleVersionInfoVisibility(jrpc_context *context,
// json_object *params,  json_object *id) {
//   ToggleVersionInfoVisibility();
//   return 0;
// }

// // set all
// json_object *jsonrpc_SetAllLabelVisibility(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int setting = lua_toboolean(L, 1);
//   SetAllLabelVisibility(setting);
//   return 0;
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

// json_object *jsonrpc_SetAmbientlight(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetAmbientlight(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_GetBackgroundcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_createtable(L, 0, 3);

//   lua_pushnumber(L, backgroundbasecolor[0]);
//   lua_setfield(L, -2, "r");

//   lua_pushnumber(L, backgroundbasecolor[1]);
//   lua_setfield(L, -2, "g");

//   lua_pushnumber(L, backgroundbasecolor[2]);
//   lua_setfield(L, -2, "b");

//   return 1;
// }

// json_object *jsonrpc_SetBackgroundcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetBackgroundcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetBlockcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetBlockcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetBlockshininess(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetBlockshininess(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetBlockspecular(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetBlockspecular(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetBoundcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetBoundcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // int lua_set_colorbar_textureflag(jrpc_context *context, json_object
// *params,  json_object *id) {
// //   int setting = lua_tonumber(L, 1);
// //   int return_code = set_colorbar_textureflag(setting);
// //   lua_pushnumber(L, 1);
// //   return 1;
// // }

// float Getcolorfield(jrpc_context *context, json_object *params,  json_object
// *id, int stack_index, const char *key) {
//   if (!lua_istable(L, stack_index)) {
//     fprintf(stderr,
//             "stack is not a table at index, cannot use getcolorfield\n");
//     exit(1);
//   }
//   // if stack index is relative (negative) convert to absolute (positive)
//   if (stack_index < 0) {
//     stack_index = lua_gettop(L) + stack_index + 1;
//   }
//   lua_pushstring(L, key);
//   lua_gettable(L, stack_index);
//   float result = lua_tonumber(L, -1);
//   lua_pop(L, 1);
//   return result;
// }

// int GetColor(jrpc_context *context, json_object *params,  json_object *id,
// int stack_index, float *color) {
//   if (!lua_istable(L, stack_index)) {
//     fprintf(stderr, "color table is not present\n");
//     return 1;
//   }
//   float r = Getcolorfield(L, stack_index, "r");
//   float g = Getcolorfield(L, stack_index, "g");
//   float b = Getcolorfield(L, stack_index, "b");
//   color[0] = r;
//   color[1] = g;
//   color[2] = b;
//   return 0;
// }
// json_object *jsonrpc_SetColorbarColors(jrpc_context *context, json_object
// *params, json_object *id) {
//   printf("running: lua_set_colorbar_colors\n");
//   if (!lua_istable(L, 1)) {
//     fprintf(stderr, "colorbar table is not present\n");
//     return 1;
//   }
//   int ncolors = 0;
//   lua_pushnil(L);
//   while (lua_next(L, 1) != 0) {
//     ncolors++;
//     lua_pop(L, 1);
//   }
//   if (ncolors > 0) {
//     float *colors = malloc(sizeof(float) * ncolors * 3);
//     for (int i = 1; i <= ncolors; i++) {
//       lua_pushnumber(L, i);
//       lua_gettable(L, 1);
//       GetColor(L, -1, &colors[i - 1]);
//     }

//     int return_code = SetColorbarColors(ncolors, colors);
//     lua_pushnumber(L, return_code);
//     free(colors);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_GetColorbarColors(jrpc_context *context, json_object
// *params, json_object *id) {
//   int i;
//   float *rgb_ini_copy_p = rgb_ini;
//   lua_createtable(L, 0, nrgb_ini);
//   for (i = 0; i < nrgb_ini; i++) {
//     lua_pushnumber(L, i + 1);
//     lua_createtable(L, 0, 2);

//     lua_pushnumber(L, *rgb_ini_copy_p);
//     lua_setfield(L, -2, "r");

//     lua_pushnumber(L, *(rgb_ini_copy_p + 1));
//     lua_setfield(L, -2, "g");

//     lua_pushnumber(L, *(rgb_ini_copy_p + 2));
//     lua_setfield(L, -2, "b");

//     lua_settable(L, -3);
//     rgb_ini_copy_p += 3;
//   }
//   // Leaves one returned value on the stack, the mesh table.
//   return 1;
// }

// json_object *jsonrpc_SetColor2barColors(jrpc_context *context, json_object
// *params, json_object *id) {
//   int ncolors = lua_tonumber(L, 1);
//   if (!lua_istable(L, -1)) {
//     fprintf(stderr, "colorbar table is not present\n");
//     return 1;
//   }
//   if (ncolors > 0) {
//     float *colors = malloc(sizeof(float) * ncolors * 3);
//     for (size_t i = 1; i <= ncolors; i++) {
//       lua_pushnumber(L, i);
//       lua_gettable(L, -2);
//       GetColor(L, -1, &colors[i - 1]);
//     }

//     int return_code = SetColor2barColors(ncolors, colors);
//     lua_pushnumber(L, return_code);
//     free(colors);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_GetColor2barColors(jrpc_context *context, json_object
// *params, json_object *id) {
//   int i;
//   float *rgb_ini_copy_p = rgb2_ini;
//   lua_createtable(L, 0, nrgb2_ini);
//   for (i = 0; i < nrgb2_ini; i++) {
//     lua_pushnumber(L, i + 1);
//     lua_createtable(L, 0, 2);

//     lua_pushnumber(L, *rgb_ini_copy_p);
//     lua_setfield(L, -2, "r");

//     lua_pushnumber(L, *(rgb_ini_copy_p + 1));
//     lua_setfield(L, -2, "g");

//     lua_pushnumber(L, *(rgb_ini_copy_p + 2));
//     lua_setfield(L, -2, "b");

//     lua_settable(L, -3);
//     rgb_ini_copy_p += 3;
//   }
//   // Leaves one returned value on the stack, the mesh table.
//   return 1;
// }

// json_object *jsonrpc_SetDiffuselight(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetDiffuselight(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetDirectioncolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetDirectioncolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

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

// json_object *jsonrpc_GetForegroundcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_createtable(L, 0, 3);

//   lua_pushnumber(L, foregroundbasecolor[0]);
//   lua_setfield(L, -2, "r");

//   lua_pushnumber(L, foregroundbasecolor[1]);
//   lua_setfield(L, -2, "g");

//   lua_pushnumber(L, foregroundbasecolor[2]);
//   lua_setfield(L, -2, "b");

//   return 1;
// }

// json_object *jsonrpc_SetForegroundcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetForegroundcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetHeatoffcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetHeatoffcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetHeatoncolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetHeatoncolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetIsocolors(jrpc_context *context, json_object *params,
// json_object *id) {
//   float shininess = lua_tonumber(L, 1);
//   float transparency = lua_tonumber(L, 2);
//   int transparency_option = lua_tonumber(L, 3);
//   int opacity_change = lua_tonumber(L, 4);
//   float specular[3];
//   GetColor(L, 5, specular);
//   int n_colors = 0;
//   // count the number of colours
//   lua_pushnil(L); /* first key */
//   while (lua_next(L, 6) != 0) {
//     lua_pop(L, 1); // remove value (leave key for next iteration)
//     n_colors++;
//   }
//   int i;
//   float colors[MAX_ISO_COLORS][4];
//   for (i = 1; i <= n_colors; i++) {
//     if (!lua_istable(L, 6)) {
//       fprintf(stderr, "isocolor table is not present\n");
//       return 1;
//     }
//     lua_pushnumber(L, i);
//     lua_gettable(L, 6);
//     GetColor(L, -1, colors[i - 1]);
//   }
//   int return_code = SetIsocolors(shininess, transparency,
//   transparency_option,
//                                  opacity_change, specular, n_colors, colors);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetColortable(jrpc_context *context, json_object
// *params, json_object *id) {
//   // int ncolors = lua_tonumber(L, 1);
//   int ncolors = 0;
//   int i = 0;
//   // count the number of colours
//   lua_pushnil(L); /* first key */
//   while (lua_next(L, 1) != 0) {
//     lua_pop(L, 1); // remove value (leave key for next iteration)
//     ncolors++;
//   }
//   if (ncolors > 0) {
//     // initialise arrays using the above count info
//     float *colors = malloc(sizeof(float) * ncolors * 3);
//     // char *names = malloc(sizeof(char)*ncolors*255);
//     // char **names = malloc(sizeof(char*));
//     /* table is in the stack at index 't' */
//     lua_pushnil(L); /* first key */
//     while (lua_next(L, 1) != 0) {
//       /* uses 'key' (at index -2) and 'value' (at index -1) */
//       // strncpy(names[i], lua_tostring(L, -2), 255);
//       GetColor(L, -1, &colors[i]);
//       /* removes 'value'; keeps 'key' for next iteration */
//       lua_pop(L, 1);
//       i++;
//     }
//     free(colors);
//     // free(names);
//   }
//   return 0;
// }

// json_object *jsonrpc_SetLightpos0(jrpc_context *context, json_object *params,
// json_object *id) {
//   float a = lua_tonumber(L, 1);
//   float b = lua_tonumber(L, 2);
//   float c = lua_tonumber(L, 3);
//   float d = lua_tonumber(L, 4);
//   int return_code = SetLightpos0(a, b, c, d);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetLightpos1(jrpc_context *context, json_object *params,
// json_object *id) {
//   float a = lua_tonumber(L, 1);
//   float b = lua_tonumber(L, 2);
//   float c = lua_tonumber(L, 3);
//   float d = lua_tonumber(L, 4);
//   int return_code = SetLightpos1(a, b, c, d);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSensorcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetSensorcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSensornormcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetSensornormcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetBw(jrpc_context *context, json_object *params,
// json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   int return_code = SetBw(a, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSprinkleroffcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetSprinkleroffcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSprinkleroncolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetSprinkleroncolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetStaticpartcolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetStaticpartcolor(r, g, b);
//   lua_pushnumber(L, return_code);
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

// json_object *jsonrpc_SetVentcolor(jrpc_context *context, json_object *params,
// json_object *id) {
//   float r = lua_tonumber(L, 1);
//   float g = lua_tonumber(L, 2);
//   float b = lua_tonumber(L, 3);
//   int return_code = SetVentcolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetGridlinewidth(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetGridlinewidth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetIsolinewidth(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetIsolinewidth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetIsopointsize(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetIsopointsize(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetLinewidth(jrpc_context *context, json_object *params,
// json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetLinewidth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetPartpointsize(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetPartpointsize(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetPlot3dlinewidth(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetPlot3dlinewidth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetPlot3dpointsize(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetPlot3dpointsize(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSensorabssize(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetSensorabssize(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSensorrelsize(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetSensorrelsize(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSliceoffset(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetSliceoffset(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSmoothlines(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetSmoothlines(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSpheresegs(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetSpheresegs(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSprinklerabssize(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetSprinklerabssize(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetStreaklinewidth(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetStreaklinewidth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTicklinewidth(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetTicklinewidth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetUsenewdrawface(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetUsenewdrawface(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetVeclength(jrpc_context *context, json_object *params,
// json_object *id) {
//   float vf = lua_tonumber(L, 1);
//   int vec_uniform_length = lua_tonumber(L, 2);
//   int vec_uniform_spacing = lua_tonumber(L, 3);
//   int return_code = SetVeclength(vf, vec_uniform_length,
//   vec_uniform_spacing); lua_pushnumber(L, return_code); return 1;
// }

// json_object *jsonrpc_SetVectorlinewidth(jrpc_context *context, json_object
// *params, json_object *id) {
//   float a = lua_tonumber(L, 1);
//   float b = lua_tonumber(L, 2);
//   int return_code = SetVectorlinewidth(a, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetVectorpointsize(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetVectorpointsize(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetVentlinewidth(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetVentlinewidth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetVentoffset(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetVentoffset(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetWindowoffset(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetWindowoffset(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetWindowwidth(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetWindowwidth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetWindowheight(jrpc_context *context, json_object
// *params, json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetWindowheight(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // --  *** DATA LOADING ***

// json_object *jsonrpc_SetBoundzipstep(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetBoundzipstep(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// #ifdef pp_FED
// json_object *jsonrpc_SetFed(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetFed(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetFedcolorbar(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *name = lua_tostring(L, 1);
//   int return_code = SetFedcolorbar(name);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// #endif
// json_object *jsonrpc_SetIsozipstep(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetIsozipstep(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetNopart(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetNopart(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// #ifdef pp_FED
// json_object *jsonrpc_SetShowfedarea(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowfedarea(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// #endif

// json_object *jsonrpc_SetSliceaverage(jrpc_context *context, json_object
// *params, json_object *id) {
//   int flag = lua_tonumber(L, 1);
//   float interval = lua_tonumber(L, 2);
//   int vis = lua_tonumber(L, 3);
//   int return_code = SetSliceaverage(flag, interval, vis);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSlicedataout(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSlicedataout(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSlicezipstep(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSlicezipstep(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSmoke3dzipstep(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSmoke3dzipstep(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetUserrotate(jrpc_context *context, json_object
// *params, json_object *id) {
//   int index = lua_tonumber(L, 1);
//   int show_center = lua_tonumber(L, 2);
//   float x = lua_tonumber(L, 3);
//   float y = lua_tonumber(L, 4);
//   float z = lua_tonumber(L, 5);
//   int return_code = SetUserrotate(index, show_center, x, y, z);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // --  *** VIEW PARAMETERS ***
// json_object *jsonrpc_SetAperture(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetAperture(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetBlocklocation(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetBlocklocation(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetBoundarytwoside(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetBoundarytwoside(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetClip(jrpc_context *context, json_object *params,
// json_object *id) {
//   float v_near = lua_tonumber(L, 1);
//   float v_far = lua_tonumber(L, 2);
//   int return_code = SetClip(v_near, v_far);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetContourtype(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetContourtype(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetCullfaces(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetCullfaces(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTexturelighting(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetTexturelighting(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetEyeview(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetEyeview(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetEyex(jrpc_context *context, json_object *params,
// json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetEyex(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetEyey(jrpc_context *context, json_object *params,
// json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetEyey(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetEyez(jrpc_context *context, json_object *params,
// json_object *id) {
//   float v = lua_tonumber(L, 1);
//   int return_code = SetEyez(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetFontsize(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetFontsize(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetFrameratevalue(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetFrameratevalue(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowfacesSolid(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowfacesSolid(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowfacesOutline(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowfacesOutline(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSmoothgeomnormal(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSmoothgeomnormal(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetGeomvertexag(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetGeomvertexag(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetGversion(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetGversion(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetIsotran2(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetIsotran2(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetMeshvis(jrpc_context *context, json_object *params,
// json_object *id) {
//   int n = 0;
//   int i = 0;
//   // count the number of values
//   lua_pushnil(L);
//   while (lua_next(L, 1) != 0) {
//     lua_pop(L, 1); // remove value (leave key for next iteration)
//     n++;
//   }
//   if (n > 0) {
//     // initialise arrays using the above count info
//     int *vals = malloc(sizeof(int) * n);
//     /* table is in the stack at index 't' */
//     lua_pushnil(L); /* first key */
//     while (lua_next(L, 1) != 0) {
//       vals[i] = lua_tonumber(L, -2);
//       /* removes 'value'; keeps 'key' for next iteration */
//       lua_pop(L, 1);
//       i++;
//     }
//     int return_code = SetMeshvis(n, vals);
//     lua_pushnumber(L, return_code);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_SetMeshoffset(jrpc_context *context, json_object
// *params, json_object *id) {
//   int meshnum = lua_tonumber(L, 1);
//   int value = lua_tonumber(L, 2);
//   int return_code = SetMeshoffset(meshnum, value);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetNorthangle(jrpc_context *context, json_object
// *params, json_object *id) {
//   int vis = lua_tonumber(L, 1);
//   float x = lua_tonumber(L, 2);
//   float y = lua_tonumber(L, 3);
//   float z = lua_tonumber(L, 4);
//   int return_code = SetNorthangle(vis, x, y, z);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetOffsetslice(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetOffsetslice(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetOutlinemode(jrpc_context *context, json_object
// *params, json_object *id) {
//   int highlight = lua_tonumber(L, 1);
//   int outline = lua_tonumber(L, 2);
//   int return_code = SetOutlinemode(highlight, outline);
//   lua_pushnumber(L, return_code);
//   ;
//   return 1;
// }

// json_object *jsonrpc_SetP3dsurfacetype(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetP3dsurfacetype(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetP3dsurfacesmooth(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetP3dsurfacesmooth(v);
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

// json_object *jsonrpc_SetShowalltextures(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowalltextures(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowaxislabels(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowaxislabels(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowblocklabel(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowblocklabel(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowblocks(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowblocks(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowcadandgrid(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowcadandgrid(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowcadopaque(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowcadopaque(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowceiling(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowceiling(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowcolorbars(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowcolorbars(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowcvents(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 1);
//   int return_code = SetShowcvents(a, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowdummyvents(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowdummyvents(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowfloor(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowfloor(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowframe(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowframe(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowframelabel(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowframelabel(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowframerate(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowframerate(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowgrid(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowgrid(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowgridloc(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowgridloc(v);
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

// json_object *jsonrpc_SetShowhrrcutoff(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowhrrcutoff(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowiso(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowiso(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowisonormals(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowisonormals(v);
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

// json_object *jsonrpc_SetShowopenvents(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 1);
//   int return_code = SetShowopenvents(a, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowothervents(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowothervents(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowsensors(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   int return_code = SetShowsensors(a, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowsliceinobst(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowsliceinobst(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowsmokepart(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowsmokepart(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowsprinkpart(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowsprinkpart(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowstreak(jrpc_context *context, json_object
// *params, json_object *id) {
//   int show = lua_tonumber(L, 1);
//   int step = lua_tonumber(L, 2);
//   int showhead = lua_tonumber(L, 3);
//   int index = lua_tonumber(L, 4);
//   int return_code = SetShowstreak(show, step, showhead, index);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowterrain(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowterrain(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowthreshold(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   float c = lua_tonumber(L, 3);
//   int return_code = SetShowthreshold(a, b, c);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowticks(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowticks(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtimebar(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowtimebar(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtimelabel(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowtimelabel(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtitle(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowtitle(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtracersalways(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowtracersalways(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtriangles(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   int c = lua_tonumber(L, 3);
//   int d = lua_tonumber(L, 4);
//   int e = lua_tonumber(L, 5);
//   int f = lua_tonumber(L, 6);
//   int return_code = SetShowtriangles(a, b, c, d, e, f);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtransparent(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowtransparent(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtranparentvents(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowtransparentvents(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtrianglecount(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowtrianglecount(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowventflow(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   int c = lua_tonumber(L, 3);
//   int d = lua_tonumber(L, 4);
//   int e = lua_tonumber(L, 5);
//   int return_code = SetShowventflow(a, b, c, d, e);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowvents(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowvents(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowwalls(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowwalls(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSkipembedslice(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSkipembedslice(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// #ifdef pp_SLICEUP
// int lua_set_slicedup(jrpc_context *context, json_object *params,  json_object
// *id) {
//   int scalar = lua_tonumber(L, 1);
//   int vector = lua_tonumber(L, 1);
//   int return_code = set_slicedup(scalar, vector);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// #endif

// json_object *jsonrpc_SetSmokesensors(jrpc_context *context, json_object
// *params, json_object *id) {
//   int show = lua_tonumber(L, 1);
//   int test = lua_tonumber(L, 2);
//   int return_code = SetSmokesensors(show, test);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // int set_smoothblocksolid(int v); // SMOOTHBLOCKSOLID
// #ifdef pp_LANG
// int lua_set_startuplang(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *lang = lua_tostring(L, 1);
//   int return_code = set_startuplang(lang);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// #endif

// json_object *jsonrpc_SetStereo(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetStereo(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSurfinc(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSurfinc(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTerrainparams(jrpc_context *context, json_object
// *params, json_object *id) {
//   int r_min = lua_tonumber(L, 1);
//   int g_min = lua_tonumber(L, 2);
//   int b_min = lua_tonumber(L, 3);
//   int r_max = lua_tonumber(L, 4);
//   int g_max = lua_tonumber(L, 5);
//   int b_max = lua_tonumber(L, 6);
//   int vert_factor = lua_tonumber(L, 7);
//   int return_code =
//       SetTerrainparams(r_min, g_min, b_min, r_max, g_max, b_max,
//       vert_factor);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTitlesafe(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetTitlesafe(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTrainermode(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetTrainermode(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTrainerview(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetTrainerview(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTransparent(jrpc_context *context, json_object
// *params, json_object *id) {
//   int use_flag = lua_tonumber(L, 1);
//   float level = lua_tonumber(L, 2);
//   int return_code = SetTransparent(use_flag, level);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTreeparms(jrpc_context *context, json_object *params,
// json_object *id) {
//   int minsize = lua_tonumber(L, 1);
//   int visx = lua_tonumber(L, 2);
//   int visy = lua_tonumber(L, 3);
//   int visz = lua_tonumber(L, 4);
//   int return_code = SetTreeparms(minsize, visx, visy, visz);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTwosidedvents(jrpc_context *context, json_object
// *params, json_object *id) {
//   int internal = lua_tonumber(L, 1);
//   int external = lua_tonumber(L, 2);
//   int return_code = SetTwosidedvents(internal, external);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetVectorskip(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetVectorskip(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetVolsmoke(jrpc_context *context, json_object *params,
// json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   int c = lua_tonumber(L, 3);
//   int d = lua_tonumber(L, 4);
//   int e = lua_tonumber(L, 5);
//   float f = lua_tonumber(L, 6);
//   float g = lua_tonumber(L, 7);
//   float h = lua_tonumber(L, 8);
//   float i = lua_tonumber(L, 9);
//   float j = lua_tonumber(L, 10);
//   float k = lua_tonumber(L, 11);
//   float l = lua_tonumber(L, 12);
//   int return_code = SetVolsmoke(a, b, c, d, e, f, g, h, i, j, k, l);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetZoom(jrpc_context *context, json_object *params,
// json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   int return_code = SetZoom(a, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // *** MISC ***
// json_object *jsonrpc_SetCellcentertext(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetCellcentertext(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetInputfile(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *inputfile = lua_tostring(L, 1);
//   int return_code = SetInputfile(inputfile);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetLabelstartupview(jrpc_context *context, json_object
// *params, json_object *id) {
//   const char *viewname = lua_tostring(L, 1);
//   int return_code = SetLabelstartupview(viewname);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // DEPRECATED
// // int lua_set_pixelskip(jrpc_context *context, json_object *params,
// json_object *id) {
// //   int v = lua_tonumber(L, 1);
// //   int return_code = set_pixelskip(v);
// //   lua_pushnumber(L, return_code);
// //   return 1;
// // }

// json_object *jsonrpc_SetRenderclip(jrpc_context *context, json_object
// *params, json_object *id) {
//   int use_flag = lua_tonumber(L, 1);
//   int left = lua_tonumber(L, 2);
//   int right = lua_tonumber(L, 3);
//   int bottom = lua_tonumber(L, 4);
//   int top = lua_tonumber(L, 5);
//   int return_code = SetRenderclip(use_flag, left, right, bottom, top);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // DEPRECATED
// // int lua_set_renderfilelabel(jrpc_context *context, json_object *params,
// json_object *id) {
// //   int v = lua_tonumber(L, 1);
// //   int return_code = set_renderfilelabel(v);
// //   lua_pushnumber(L, return_code);
// //   return 1;
// // }

// json_object *jsonrpc_SetRenderfiletype(jrpc_context *context, json_object
// *params, json_object *id) {
//   int render = lua_tonumber(L, 1);
//   int movie = lua_tonumber(L, 2);
//   int return_code = SetRenderfiletype(render, movie);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // int lua_set_skybox(jrpc_context *context, json_object *params, json_object
// *id){
// //   return 0;
// // }

// // DEPRECATED
// // int lua_set_renderoption(jrpc_context *context, json_object *params,
// json_object *id) {
// //   int opt = lua_tonumber(L, 1);
// //   int rows = lua_tonumber(L, 1);
// //   int return_code = set_renderoption(opt, rows);
// //   lua_pushnumber(L, return_code);
// //   return 1;
// // }

// json_object *jsonrpc_GetUnitDefs(jrpc_context *context, json_object *params,
// json_object *id, f_units unitclass) {
//   lua_createtable(L, 0, 4);
//   // Loop through all of the units
//   int j;
//   for (j = 0; j < unitclass.nunits; j++) {
//     lua_pushstring(L, unitclass.units[j].unit);
//     lua_createtable(L, 0, 4);

//     lua_pushstring(L, unitclass.units[j].unit);
//     lua_setfield(L, -2, "unit");

//     lua_pushstring(L, "scale");
//     lua_createtable(L, 0, 2);
//     lua_pushnumber(L, unitclass.units[j].scale[0]);
//     lua_setfield(L, -2, "factor");
//     lua_pushnumber(L, unitclass.units[j].scale[1]);
//     lua_setfield(L, -2, "offset");
//     lua_settable(L, -3);

//     lua_pushstring(L, unitclass.units[j].rel_val);
//     lua_setfield(L, -2, "rel_val");

//     lua_pushboolean(L, unitclass.units[j].rel_defined);
//     lua_setfield(L, -2, "rel_defined");

//     lua_settable(L, -3);
//   }
//   return 1;
// }

// // TODO: implement iterators for this table
// json_object *jsonrpc_GetUnitclass(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *classname = lua_tostring(L, 1);
//   int i;
//   for (i = 0; i < nunitclasses_default; i++) {
//     // if the classname matches, put a table on the stack
//     if (strcmp(classname, unitclasses_default[i].unitclass) == 0) {
//       lua_createtable(L, 0, 4);
//       // Loop through all of the units
//       LuaGetUnitDefs(L, unitclasses_default[i]);
//       return 1;
//     }
//   }
//   return 0;
// }

// json_object *jsonrpc_GetUnits(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *classname = lua_tostring(L, 1);
//   int i;
//   for (i = 0; i < nunitclasses_default; i++) {
//     // if the classname matches, put a table on the stack
//     if (strcmp(classname, unitclasses_default[i].unitclass) == 0) {
//       // lua_createtable(L, 0, 4);
//       // // Loop through all of the units
//       // lua_get_units(L, unitclasses_default[i]);
//       lua_pushstring(
//           L,
//           unitclasses_default[i].units[unitclasses_default[i].unit_index].unit);
//       return 1;
//     }
//   }
//   return 0;
// }

// json_object *jsonrpc_SetUnits(jrpc_context *context, json_object *params,
// json_object *id) {
//   const char *unitclassname = lua_tostring(L, 1);
//   const char *unitname = lua_tostring(L, 2);

//   size_t unitclass_index;
//   bool unit_class_found = false;
//   size_t unit_index;
//   bool unit_index_found = false;
//   for (size_t i = 0; i < nunitclasses_default; i++) {
//     if (strcmp(unitclasses[i].unitclass, unitclassname) == 0) {
//       unitclass_index = i;
//       unit_class_found = true;
//       break;
//     }
//   }
//   if (!unit_class_found) {
//     return luaL_error(L, "unit class index not found");
//   }
//   for (size_t i = 0; i < unitclasses[unitclass_index].nunits; i++) {
//     if (strcmp(unitclasses[unitclass_index].units[i].unit, unitname) == 0) {
//       unit_index = i;
//       unit_index_found = true;
//       break;
//     }
//   }
//   if (!unit_index_found) {
//     return luaL_error(L, "unit index not found");
//   }
//   SetUnits(unitclass_index, unit_index);
//   return 0;
// }

// json_object *jsonrpc_SetUnitclasses(jrpc_context *context, json_object
// *params, json_object *id) {
//   int i = 0;
//   int n = 0;
//   if (!lua_istable(L, -1)) {
//     fprintf(stderr, "stack is not a table at index\n");
//     exit(1);
//   }
//   lua_pushnil(L);
//   while (lua_next(L, -2) != 0) {
//     lua_pop(L, 1);
//     n++;
//   }
//   if (n > 0) {
//     int *indices = malloc(sizeof(int) * n);
//     lua_pushnil(L);
//     while (lua_next(L, -2) != 0) {
//       indices[i] = lua_tonumber(L, -1);
//       lua_pop(L, 1);
//       i++;
//     }
//     int return_code = SetUnitclasses(n, indices);
//     lua_pushnumber(L, return_code);
//     free(indices);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_SetZaxisangles(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   int c = lua_tonumber(L, 3);
//   int return_code = SetZaxisangles(a, b, c);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetColorbartype(jrpc_context *context, json_object
// *params, json_object *id) {
//   int type = lua_tonumber(L, 1);
//   const char *label = lua_tostring(L, 2);
//   int return_code = SetColorbartype(type, label);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetExtremecolors(jrpc_context *context, json_object
// *params, json_object *id) {
//   int rmin = lua_tonumber(L, 1);
//   int gmin = lua_tonumber(L, 2);
//   int bmin = lua_tonumber(L, 3);
//   int rmax = lua_tonumber(L, 4);
//   int gmax = lua_tonumber(L, 5);
//   int bmax = lua_tonumber(L, 6);
//   int return_code = SetExtremecolors(rmin, gmin, bmin, rmax, gmax, bmax);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetFirecolor(jrpc_context *context, json_object *params,
// json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetFirecolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetFirecolormap(jrpc_context *context, json_object
// *params, json_object *id) {
//   int type = lua_tonumber(L, 1);
//   int index = lua_tonumber(L, 2);
//   int return_code = SetFirecolormap(type, index);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetFiredepth(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetFiredepth(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowextremedata(jrpc_context *context, json_object
// *params, json_object *id) {
//   int show_extremedata = lua_tonumber(L, 1);
//   int below = lua_tonumber(L, 2);
//   int above = lua_tonumber(L, 3);
//   int return_code = SetShowextremedata(show_extremedata, below, above);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSmokecolor(jrpc_context *context, json_object
// *params, json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetSmokecolor(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSmokecull(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSmokecull(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSmokeskip(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSmokeskip(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSmokealbedo(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSmokealbedo(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// #ifdef pp_GPU
// json_object *jsonrpc_SetSmokerthick(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetSmokerthick(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// #endif

// #ifdef pp_GPU
// json_object *jsonrpc_SetUsegpu(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetUsegpu(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// #endif

// // *** ZONE FIRE PARAMETRES ***
// json_object *jsonrpc_SetShowhazardcolors(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowhazardcolors(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowhzone(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowhzone(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowszone(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowszone(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowvzone(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowvzone(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowzonefire(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowzonefire(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// // *** TOUR INFO ***
// json_object *jsonrpc_SetShowpathnodes(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowpathnodes(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetShowtourroute(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowtourroute(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetTourcolorsSelectedpathline(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetTourcolorsSelectedpathline(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// json_object *jsonrpc_SetTourcolorsSelectedpathlineknots(jrpc_context
// *context, json_object *jsonrpc_params,  json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetTourcolorsSelectedpathlineknots(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// json_object *jsonrpc_SetTourcolorsSelectedknot(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetTourcolorsSelectedknot(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// json_object *jsonrpc_SetTourcolorsPathline(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetTourcolorsSelectedpathline(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// json_object *jsonrpc_SetTourcolorsPathknots(jrpc_context *context,
// json_object *params,  json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetTourcolorsPathknots(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// json_object *jsonrpc_SetTourcolorsText(jrpc_context *context, json_object
// *params, json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetTourcolorsText(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }
// json_object *jsonrpc_SetTourcolorsAvatar(jrpc_context *context, json_object
// *params, json_object *id) {
//   int r = lua_tonumber(L, 1);
//   int g = lua_tonumber(L, 2);
//   int b = lua_tonumber(L, 3);
//   int return_code = SetTourcolorsAvatar(r, g, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetViewalltours(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetViewalltours(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetViewtimes(jrpc_context *context, json_object *params,
// json_object *id) {
//   float start = lua_tonumber(L, 1);
//   float stop = lua_tonumber(L, 2);
//   int ntimes = lua_tonumber(L, 3);
//   int return_code = SetViewtimes(start, stop, ntimes);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetViewtourfrompath(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetViewtourfrompath(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetDevicevectordimensions(jrpc_context *context,
// json_object *params,  json_object *id) {
//   float baselength = lua_tonumber(L, 1);
//   float basediameter = lua_tonumber(L, 2);
//   float headlength = lua_tonumber(L, 3);
//   float headdiameter = lua_tonumber(L, 4);
//   int return_code = SetDevicevectordimensions(baselength, basediameter,
//                                               headlength, headdiameter);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetDevicebounds(jrpc_context *context, json_object
// *params, json_object *id) {
//   float min = lua_tonumber(L, 1);
//   float max = lua_tonumber(L, 2);
//   int return_code = SetDevicebounds(min, max);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetDeviceorientation(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   float b = lua_tonumber(L, 2);
//   int return_code = SetDeviceorientation(a, b);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetGridparms(jrpc_context *context, json_object *params,
// json_object *id) {
//   int vx = lua_tonumber(L, 1);
//   int vy = lua_tonumber(L, 2);
//   int vz = lua_tonumber(L, 3);
//   int px = lua_tonumber(L, 4);
//   int py = lua_tonumber(L, 5);
//   int pz = lua_tonumber(L, 6);
//   int return_code = SetGridparms(vx, vy, vz, px, py, pz);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetGsliceparms(jrpc_context *context, json_object
// *params, json_object *id) {
//   int i;
//   int vis_data = lua_tonumber(L, 1);
//   int vis_triangles = lua_tonumber(L, 2);
//   int vis_triangulation = lua_tonumber(L, 3);
//   int vis_normal = lua_tonumber(L, 4);
//   float xyz[3];
//   // TODO: use named fields (e.g. xyz)
//   for (i = 0; i < 3; i++) {
//     lua_pushnumber(L, i);
//     lua_gettable(L, 5);
//     xyz[i] = lua_tonumber(L, -1);
//     lua_pop(L, 1);
//     i++;
//   }
//   float azelev[2];
//   for (i = 0; i < 2; i++) {
//     lua_pushnumber(L, i);
//     lua_gettable(L, 6);
//     azelev[i] = lua_tonumber(L, -1);
//     lua_pop(L, 1);
//     i++;
//   }
//   int return_code = SetGsliceparms(vis_data, vis_triangles,
//   vis_triangulation,
//                                    vis_normal, xyz, azelev);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetLoadfilesatstartup(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetLoadfilesatstartup(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetMscale(jrpc_context *context, json_object *params,
// json_object *id) {
//   float a = lua_tonumber(L, 1);
//   float b = lua_tonumber(L, 2);
//   float c = lua_tonumber(L, 3);
//   int return_code = SetMscale(a, b, c);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetSliceauto(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushnil(L);
//   int n = 0;
//   while (lua_next(L, -2) != 0) {
//     lua_pop(L, 1);
//     n++;
//   }
//   if (n > 0) {
//     int i = 0;
//     int *vals = malloc(sizeof(int) * n);
//     lua_pushnil(L);
//     while (lua_next(L, -2) != 0) {
//       vals[i] = lua_tonumber(L, -1);
//       lua_pop(L, 1);
//       i++;
//     }
//     int return_code = SetSliceauto(n, vals);
//     lua_pushnumber(L, return_code);
//     free(vals);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_SetMsliceauto(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_pushnil(L);
//   int n = 0;
//   while (lua_next(L, -2) != 0) {
//     lua_pop(L, 1);
//     n++;
//   }
//   if (n > 0) {
//     int i = 0;
//     int *vals = malloc(sizeof(int) * n);
//     lua_pushnil(L);
//     while (lua_next(L, -2) != 0) {
//       vals[i] = lua_tonumber(L, -1);
//       lua_pop(L, 1);
//       i++;
//     }
//     int return_code = SetMsliceauto(n, vals);
//     lua_pushnumber(L, return_code);
//     free(vals);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_SetCompressauto(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetCompressauto(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetPropindex(jrpc_context *context, json_object *params,
// json_object *id) {
//   lua_pushnil(L);
//   int n = 0;
//   while (lua_next(L, -2) != 0) {
//     lua_pop(L, 1);
//     n++;
//   }
//   if (n > 0) {
//     int i = 0;
//     int *vals = malloc(sizeof(int) * n * PROPINDEX_STRIDE);
//     lua_pushnil(L);
//     while (lua_next(L, -2) != 0) {
//       lua_pushnumber(L, 1);
//       lua_gettable(L, -2);
//       *(vals + (i * PROPINDEX_STRIDE + 0)) = lua_tonumber(L, -1);
//       lua_pop(L, 1);

//       lua_pushnumber(L, 1);
//       lua_gettable(L, -2);
//       *(vals + (i * PROPINDEX_STRIDE + 1)) = lua_tonumber(L, -1);
//       lua_pop(L, 1);

//       lua_pop(L, 1);
//       i++;
//     }
//     int return_code = SetPropindex(n, vals);
//     lua_pushnumber(L, return_code);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

// json_object *jsonrpc_SetShowdevices(jrpc_context *context, json_object
// *params, json_object *id) {
//   lua_pushnil(L);
//   int n = 0;
//   while (lua_next(L, -2) != 0) {
//     lua_pop(L, 1);
//     n++;
//   }
//   if (n > 0) {
//     int i = 0;
//     const char **names = malloc(sizeof(char *) * n);
//     lua_pushnil(L);
//     while (lua_next(L, -2) != 0) {
//       names[i] = lua_tostring(L, -1);
//       lua_pop(L, 1);
//       i++;
//     }
//     int return_code = SetShowdevices(n, names);
//     lua_pushnumber(L, return_code);
//     free(names);
//     return 1;
//   }
//   else {
//     return 0;
//   }
// } // SHOWDEVICES

// json_object *jsonrpc_SetShowdevicevals(jrpc_context *context, json_object
// *params, json_object *id) {
//   int a = lua_tonumber(L, 1);
//   int b = lua_tonumber(L, 2);
//   int c = lua_tonumber(L, 3);
//   int d = lua_tonumber(L, 4);
//   int e = lua_tonumber(L, 5);
//   int f = lua_tonumber(L, 6);
//   int g = lua_tonumber(L, 7);
//   int h = lua_tonumber(L, 8);
//   int return_code = SetShowdevicevals(a, b, c, d, e, f, g, h);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // SHOWDEVICEVALS

// json_object *jsonrpc_SetShowmissingobjects(jrpc_context *context, json_object
// *params,  json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetShowmissingobjects(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // SHOWMISSINGOBJECTS

// json_object *jsonrpc_SetTourindex(jrpc_context *context, json_object *params,
// json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetTourindex(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // TOURINDEX

// json_object *jsonrpc_SetCParticles(jrpc_context *context, json_object
// *params, json_object *id) {
//   int min_flag = lua_tonumber(L, 1);
//   float min_value = lua_tonumber(L, 2);
//   int max_flag = lua_tonumber(L, 3);
//   float max_value = lua_tonumber(L, 4);
//   const char *label = NULL;
//   if (lua_gettop(L) == 5) {
//     label = lua_tostring(L, 5);
//   }
//   int return_code =
//       SetCParticles(min_flag, min_value, max_flag, max_value, label);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // C_PARTICLES

// json_object *jsonrpc_SetCSlice(jrpc_context *context, json_object *params,
// json_object *id) {
//   int min_flag = lua_tonumber(L, 1);
//   float min_value = lua_tonumber(L, 2);
//   int max_flag = lua_tonumber(L, 3);
//   float max_value = lua_tonumber(L, 4);
//   const char *label = NULL;
//   if (lua_gettop(L) == 5) {
//     label = lua_tostring(L, 5);
//   }
//   int return_code = SetCSlice(min_flag, min_value, max_flag, max_value,
//   label); lua_pushnumber(L, return_code); return 1;
// } // C_SLICE

// json_object *jsonrpc_SetCacheBoundarydata(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetCacheBoundarydata(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // CACHE_BOUNDARYDATA

// json_object *jsonrpc_SetCacheQdata(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetCacheQdata(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // CACHE_QDATA

// #ifdef pp_HIST
// json_object *jsonrpc_SetPercentilelevel(jrpc_context *context, json_object
// *params, json_object *id) {
//   float p_level_min = lua_tonumber(L, 1);
//   float p_level_max = lua_tonumber(L, 2);
//   int return_code = SetPercentilelevel(p_level_min, p_level_max);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // PERCENTILELEVEL
// #endif

// json_object *jsonrpc_SetTimeoffset(jrpc_context *context, json_object
// *params, json_object *id) {
//   int v = lua_tonumber(L, 1);
//   int return_code = SetTimeoffset(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // TIMEOFFSET

// json_object *jsonrpc_SetTload(jrpc_context *context, json_object *params,
// json_object *id) {
//   int begin_flag = lua_tonumber(L, 1);
//   float begin_val = lua_tonumber(L, 2);
//   int end_flag = lua_tonumber(L, 3);
//   float end_val = lua_tonumber(L, 4);
//   int skip_flag = lua_tonumber(L, 5);
//   float skip_val = lua_tonumber(L, 6);
//   int return_code =
//       SetTload(begin_flag, begin_val, end_flag, end_val, skip_flag,
//       skip_val);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // TLOAD

// json_object *jsonrpc_SetVSlice(jrpc_context *context, json_object *params,
// json_object *id) {
//   int min_flag = lua_tonumber(L, 1);
//   float min_value = lua_tonumber(L, 2);
//   int max_flag = lua_tonumber(L, 3);
//   float max_value = lua_tonumber(L, 4);
//   const char *label = lua_tostring(L, 5);
//   float line_min = lua_tonumber(L, 6);
//   float line_max = lua_tonumber(L, 7);
//   int line_num = lua_tonumber(L, 8);
//   int return_code = SetVSlice(min_flag, min_value, max_flag, max_value,
//   label,
//                               line_min, line_max, line_num);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

// json_object *jsonrpc_SetPatchdataout(jrpc_context *context, json_object
// *params, json_object *id) {
//   int output_flag = lua_tonumber(L, 1);
//   int tmin = lua_tonumber(L, 1);
//   int tmax = lua_tonumber(L, 2);
//   int xmin = lua_tonumber(L, 3);
//   int xmax = lua_tonumber(L, 4);
//   int ymin = lua_tonumber(L, 5);
//   int ymax = lua_tonumber(L, 6);
//   int zmin = lua_tonumber(L, 7);
//   int zmax = lua_tonumber(L, 8);
//   int return_code = SetPatchdataout(output_flag, tmin, tmax, xmin, xmax,
//   ymin,
//                                     ymax, zmin, zmax);
//   lua_pushnumber(L, return_code);
//   return 1;
// } // PATCHDATAOUT

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
  jrpc_register_procedure(server, &jsonrpc_SetSliceBounds, "set_slice_bounds",
                          NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSliceBoundMin,
  //   "set_slice_bound_min",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSliceBoundMax,
  //   "set_slice_bound_max",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetSliceBounds,
  //   "get_slice_bounds", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Hidewindow, "hidewindow", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Yieldscript, "yieldscript", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Tempyieldscript,
  //   "tempyieldscript", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_DisplayCb, "displayCB", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Renderclip, "renderclip", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Render, "renderC", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_RenderVar, "render_var", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Gsliceview, "gsliceview", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Showplot3ddata, "showplot3ddata", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Gslicepos, "gslicepos", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Gsliceorien, "gsliceorien",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Settourkeyframe,
  //   "settourkeyframe", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Settourview, "settourview", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Getframe, "getframe", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Setframe, "setframe", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Gettime, "gettime", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Settime, "settime", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Loaddatafile, "loaddatafile",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Loadinifile,
  //   "loadinifile", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Loadvdatafile, "loadvdatafile", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Loadboundaryfile,
  //   "loadboundaryfile",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Load3dsmoke, "load3dsmoke",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Loadvolsmoke,
  //   "loadvolsmoke", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Loadvolsmokeframe, "loadvolsmokeframe",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetRendertype, "set_rendertype",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_GetRendertype,
  //   "get_rendertype", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetMovietype, "set_movietype", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetMovietype, "get_movietype",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Makemovie, "makemovie",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Loadtour, "loadtour",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Loadparticles,
  //   "loadparticles", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Partclasscolor, "partclasscolor", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Partclasstype, "partclasstype",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Plot3dprops,
  //   "plot3dprops", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Loadplot3d, "loadplot3d", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Loadslice, "loadslice", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Loadsliceindex, "loadsliceindex", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Loadvslice, "loadvslice", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Loadiso, "loadiso", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Unloadall, "unloadall", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Unloadtour, "unloadtour", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Setrenderdir, "setrenderdir",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Getrenderdir,
  //   "getrenderdir", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetOrthoPreset, "set_ortho_preset", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Setviewpoint, "setviewpoint",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Getviewpoint,
  //   "getviewpoint", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_ExitSmokeview, "exit", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Getcolorbarflip, "getcolorbarflip", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Setcolorbarflip,
  //   "setcolorbarflip", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_Setwindowsize, "setwindowsize", NULL);
  //   // {"window.setwindowsize", lua_setwindowsize},
  //   jrpc_register_procedure(server, &jsonrpc_Setgridvisibility,
  //   "setgridvisibility",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Setgridparms, "setgridparms",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_Setcolorbarindex,
  //   "setcolorbarindex",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_Getcolorbarindex,
  //   "getcolorbarindex",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetSliceInObst,
  //   "set_slice_in_obst",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetSliceInObst,
  //   "get_slice_in_obst",
  //                           NULL);

  //   // colorbar
  //   jrpc_register_procedure(server, &jsonrpc_SetColorbar, "set_colorbar",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetNamedColorbar,
  //   "set_named_colorbar",
  //                           NULL);
  //   // {"get_named_colorbar", lua_get_named_colorbar},

  //   jrpc_register_procedure(server, &jsonrpc_SetColorbarVisibility,
  //                           "set_colorbar_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetColorbarVisibility,
  //                           "get_colorbar_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleColorbarVisibility,
  //                           "toggle_colorbar_visibility", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetColorbarVisibilityHorizontal,
  //                           "set_colorbar_visibility_horizontal", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetColorbarVisibilityHorizontal,
  //                           "get_colorbar_visibility_horizontal", NULL);
  //   {"toggle_colorbar_visibility_horizontal",
  //    LuaToggleColorbarVisibilityHorizontal},

  //       jrpc_register_procedure(server,
  //       &jsonrpc_SetColorbarVisibilityVertical,
  //                               "set_colorbar_visibility_vertical", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetColorbarVisibilityVertical,
  //                           "get_colorbar_visibility_vertical", NULL);
  //   jrpc_register_procedure(&server,
  //   &jsonrpc_ToggleColorbarVisibilityVertical,
  //                           "toggle_colorbar_visibility_vertical", NULL);

  //   // timebar
  //   jrpc_register_procedure(server, &jsonrpc_SetTimebarVisibility,
  //                           "set_timebar_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetTimebarVisibility,
  //                           "get_timebar_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleTimebarVisibility,
  //                           "toggle_timebar_visibility", NULL);

  //   // title
  //   jrpc_register_procedure(server, &jsonrpc_SetTitleVisibility,
  //                           "set_title_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetTitleVisibility,
  //                           "get_title_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleTitleVisibility,
  //                           "toggle_title_visibility", NULL);

  //   // smv_version
  //   jrpc_register_procedure(server, &jsonrpc_SetSmvVersionVisibility,
  //                           "set_smv_version_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetSmvVersionVisibility,
  //                           "get_smv_version_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleSmvVersionVisibility,
  //                           "toggle_smv_version_visibility", NULL);

  //   // chid
  //   jrpc_register_procedure(server, &jsonrpc_SetChidVisibility,
  //   "set_chid_visibility",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetChidVisibility,
  //   "get_chid_visibility",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleChidVisibility,
  //                           "toggle_chid_visibility", NULL);

  //   // blockages
  //   jrpc_register_procedure(server, &jsonrpc_BlockagesHideAll,
  //   "blockages_hide_all",
  //                           NULL);

  //   // outlines
  //   jrpc_register_procedure(server, &jsonrpc_OutlinesShow, "outlines_show",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_OutlinesHide,
  //   "outlines_hide", NULL);

  //   // surfaces
  //   jrpc_register_procedure(server, &jsonrpc_SurfacesHideAll,
  //   "surfaces_hide_all",
  //                           NULL);
  //   // devices
  //   jrpc_register_procedure(server, &jsonrpc_DevicesHideAll,
  //   "devices_hide_all", NULL);

  //   // axis
  //   jrpc_register_procedure(server, &jsonrpc_SetAxisVisibility,
  //   "set_axis_visibility",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetAxisVisibility,
  //   "get_axis_visibility",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleAxisVisibility,
  //                           "toggle_axis_visibility", NULL);

  //   // frame label
  //   jrpc_register_procedure(server, &jsonrpc_SetFramelabelVisibility,
  //                           "set_framelabel_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetFramelabelVisibility,
  //                           "get_framelabel_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleFramelabelVisibility,
  //                           "toggle_framelabel_visibility", NULL);

  //   // framerate
  //   jrpc_register_procedure(server, &jsonrpc_SetFramerateVisibility,
  //                           "set_framerate_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetFramerateVisibility,
  //                           "get_framerate_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleFramerateVisibility,
  //                           "toggle_framerate_visibility", NULL);

  //   // grid locations
  //   jrpc_register_procedure(server, &jsonrpc_SetGridlocVisibility,
  //                           "set_gridloc_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetGridlocVisibility,
  //                           "get_gridloc_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleGridlocVisibility,
  //                           "toggle_gridloc_visibility", NULL);

  //   // hrrpuv cutoff
  //   jrpc_register_procedure(server, &jsonrpc_SetHrrcutoffVisibility,
  //                           "set_hrrcutoff_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetHrrcutoffVisibility,
  //                           "get_hrrcutoff_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleHrrcutoffVisibility,
  //                           "toggle_hrrcutoff_visibility", NULL);

  //   // hrr label
  //   jrpc_register_procedure(server, &jsonrpc_SetHrrlabelVisibility,
  //                           "set_hrrlabel_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetHrrlabelVisibility,
  //                           "get_hrrlabel_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleHrrlabelVisibility,
  //                           "toggle_hrrlabel_visibility", NULL);

  //   // mesh label
  //   jrpc_register_procedure(server, &jsonrpc_SetMeshlabelVisibility,
  //                           "set_meshlabel_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetMeshlabelVisibility,
  //                           "get_meshlabel_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleMeshlabelVisibility,
  //                           "toggle_meshlabel_visibility", NULL);

  //   // slice average
  //   jrpc_register_procedure(server, &jsonrpc_SetSliceAverageVisibility,
  //                           "set_slice_average_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetSliceAverageVisibility,
  //                           "get_slice_average_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleSliceAverageVisibility,
  //                           "toggle_slice_average_visibility", NULL);

  //   // time
  //   jrpc_register_procedure(server, &jsonrpc_SetTimeVisibility,
  //   "set_time_visibility",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetTimeVisibility,
  //   "get_time_visibility",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleTimeVisibility,
  //                           "toggle_time_visibility", NULL);

  //   // user settable ticks
  //   jrpc_register_procedure(server, &jsonrpc_SetUserTicksVisibility,
  //                           "set_user_ticks_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetUserTicksVisibility,
  //                           "get_user_ticks_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleUserTicksVisibility,
  //                           "toggle_user_ticks_visibility", NULL);

  //   // version info
  //   jrpc_register_procedure(server, &jsonrpc_SetVersionInfoVisibility,
  //                           "set_version_info_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetVersionInfoVisibility,
  //                           "get_version_info_visibility", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ToggleVersionInfoVisibility,
  //                           "toggle_version_info_visibility", NULL);

  //   // set all
  //   jrpc_register_procedure(server, &jsonrpc_SetAllLabelVisibility,
  //                           "set_all_label_visibility", NULL);

  //   // set the blockage view method
  //   jrpc_register_procedure(server, &jsonrpc_BlockageViewMethod,
  //                           "blockage_view_method", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_BlockageOutlineColor,
  //                           "blockage_outline_color", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_BlockageLocations,
  //   "blockage_locations",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetColorbarColors,
  //   "set_colorbar_colors",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetColorbarColors,
  //   "get_colorbar_colors",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetColor2barColors,
  //                           "set_color2bar_colors", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetColor2barColors,
  //                           "get_color2bar_colors", NULL);

  //   // Camera API
  //   jrpc_register_procedure(server, &jsonrpc_CameraModEyex,
  //   "camera_mod_eyex", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_CameraSetEyex, "camera_set_eyex", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraGetEyex,
  //   "camera_get_eyex", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_CameraModEyey,
  //   "camera_mod_eyey", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_CameraSetEyey, "camera_set_eyey", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraGetEyey,
  //   "camera_get_eyey", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_CameraModEyez,
  //   "camera_mod_eyez", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_CameraSetEyez, "camera_set_eyez", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraGetEyez,
  //   "camera_get_eyez", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_CameraModAz, "camera_mod_az",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_CameraSetAz,
  //   "camera_set_az", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_CameraGetAz, "camera_get_az", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraModElev,
  //   "camera_mod_elev", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_CameraZoomToFit, "camera_zoom_to_fit",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraSetElev,
  //   "camera_set_elev", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_CameraGetElev, "camera_get_elev", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_CameraSetViewdir,
  //   "camera_set_viewdir",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraGetViewdir,
  //   "camera_get_viewdir",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_CameraGetZoom,
  //   "camera_get_zoom", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_CameraSetZoom, "camera_set_zoom", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_CameraGetRotationType,
  //                           "camera_get_rotation_type", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraGetRotationIndex,
  //                           "camera_get_rotation_index", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraSetRotationType,
  //                           "camera_set_rotation_type", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraGetProjectionType,
  //                           "camera_get_projection_type", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_CameraSetProjectionType,
  //                           "camera_set_projection_type", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_GetClippingMode,
  //   "get_clipping_mode",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetClippingMode,
  //   "set_clipping_mode",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSceneclipX,
  //   "set_sceneclip_x", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetSceneclipXMin, "set_sceneclip_x_min",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSceneclipXMax,
  //   "set_sceneclip_x_max",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSceneclipY,
  //   "set_sceneclip_y", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetSceneclipYMin, "set_sceneclip_y_min",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSceneclipYMax,
  //   "set_sceneclip_y_max",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSceneclipZ,
  //   "set_sceneclip_z", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetSceneclipZMin, "set_sceneclip_z_min",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSceneclipZMax,
  //   "set_sceneclip_z_max",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetAmbientlight,
  //   "set_ambientlight",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetBackgroundcolor,
  //   "get_backgroundcolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetBackgroundcolor,
  //   "set_backgroundcolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetBlockcolor, "set_blockcolor",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetBlockshininess,
  //   "set_blockshininess",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetBlockspecular,
  //   "set_blockspecular",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetBoundcolor, "set_boundcolor",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetDiffuselight,
  //   "set_diffuselight",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetDirectioncolor,
  //   "set_directioncolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetFlip, "get_flip", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetFlip, "set_flip", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetForegroundcolor,
  //   "get_foregroundcolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetForegroundcolor,
  //   "set_foregroundcolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetHeatoffcolor,
  //   "set_heatoffcolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetHeatoncolor,
  //   "set_heatoncolor", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetIsocolors, "set_isocolors", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetColortable, "set_colortable",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetLightpos0,
  //   "set_lightpos0", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetLightpos1, "set_lightpos1", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSensorcolor,
  //   "set_sensorcolor", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetSensornormcolor, "set_sensornormcolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetBw, "set_bw", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSprinkleroffcolor,
  //                           "set_sprinkleroffcolor", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSprinkleroncolor,
  //                           "set_sprinkleroncolor", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetStaticpartcolor,
  //   "set_staticpartcolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTimebarcolor,
  //   "set_timebarcolor",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetVentcolor, "set_ventcolor",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetGridlinewidth,
  //   "set_gridlinewidth",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetIsolinewidth,
  //   "set_isolinewidth",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetIsopointsize,
  //   "set_isopointsize",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetLinewidth, "set_linewidth",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetPartpointsize,
  //   "set_partpointsize",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetPlot3dlinewidth,
  //   "set_plot3dlinewidth",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetPlot3dpointsize,
  //   "set_plot3dpointsize",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSensorabssize,
  //   "set_sensorabssize",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSensorrelsize,
  //   "set_sensorrelsize",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSliceoffset,
  //   "set_sliceoffset", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetSmoothlines, "set_smoothlines", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSpheresegs, "set_spheresegs",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetSprinklerabssize,
  //                           "set_sprinklerabssize", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetStreaklinewidth,
  //   "set_streaklinewidth",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTicklinewidth,
  //   "set_ticklinewidth",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetUsenewdrawface,
  //   "set_usenewdrawface",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetVeclength, "set_veclength",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetVectorlinewidth,
  //   "set_vectorlinewidth",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetVectorpointsize,
  //   "set_vectorpointsize",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetVentlinewidth,
  //   "set_ventlinewidth",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetVentoffset, "set_ventoffset",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetWindowoffset,
  //   "set_windowoffset",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetWindowwidth,
  //   "set_windowwidth", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetWindowheight, "set_windowheight",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetBoundzipstep,
  //   "set_boundzipstep",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetIsozipstep, "set_isozipstep",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetNopart, "set_nopart",
  //   NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetSliceaverage,
  //   "set_sliceaverage",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSlicedataout,
  //   "set_slicedataout",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSlicezipstep,
  //   "set_slicezipstep",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSmoke3dzipstep,
  //   "set_smoke3dzipstep",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetUserrotate, "set_userrotate",
  //   NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetAperture, "set_aperture",
  //   NULL);
  //   // { "set_axissmooth", lua_set_axissmooth },
  //   jrpc_register_procedure(server, &jsonrpc_SetBlocklocation,
  //   "set_blocklocation",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetBoundarytwoside,
  //   "set_boundarytwoside",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetClip, "set_clip", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetContourtype,
  //   "set_contourtype", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetCullfaces, "set_cullfaces", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTexturelighting,
  //   "set_texturelighting",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetEyeview, "set_eyeview",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetEyex, "set_eyex",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetEyey, "set_eyey",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetEyez, "set_eyez",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_GetFontsize,
  //   "get_fontsize", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetFontsize, "set_fontsize", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetFrameratevalue,
  //   "set_frameratevalue",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowfacesSolid,
  //   "set_showfaces_solid",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowfacesOutline,
  //                           "set_showfaces_outline", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSmoothgeomnormal,
  //                           "set_smoothgeomnormal", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetGeomvertexag,
  //   "set_geomvertexag",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetGversion, "set_gversion",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetIsotran2,
  //   "set_isotran2", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetMeshvis, "set_meshvis", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetMeshoffset, "set_meshoffset",
  //   NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetNorthangle, "set_northangle",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetOffsetslice,
  //   "set_offsetslice", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetOutlinemode, "set_outlinemode", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetP3dsurfacetype,
  //   "set_p3dsurfacetype",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetP3dsurfacesmooth,
  //                           "set_p3dsurfacesmooth", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetScaledfont, "set_scaledfont",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetScaledfontHeight2d,
  //                           "set_scaledfont_height2d", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowalltextures,
  //   "set_showalltextures",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowaxislabels,
  //   "set_showaxislabels",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowblocklabel,
  //   "set_showblocklabel",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowblocks, "set_showblocks",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowcadandgrid,
  //   "set_showcadandgrid",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowcadopaque,
  //   "set_showcadopaque",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowceiling,
  //   "set_showceiling", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetShowcolorbars, "set_showcolorbars",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowcvents, "set_showcvents",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowdummyvents,
  //   "set_showdummyvents",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowfloor, "set_showfloor",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowframe,
  //   "set_showframe", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetShowframelabel, "set_showframelabel",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowframerate,
  //   "set_showframerate",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowgrid, "set_showgrid",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowgridloc,
  //   "set_showgridloc", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetShowhmstimelabel,
  //                           "set_showhmstimelabel", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowhrrcutoff,
  //   "set_showhrrcutoff",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowiso, "set_showiso",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowisonormals,
  //   "set_showisonormals",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowlabels, "set_showlabels",
  //   NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetShowopenvents,
  //   "set_showopenvents",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowothervents,
  //   "set_showothervents",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowsensors,
  //   "set_showsensors", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetShowsliceinobst, "set_showsliceinobst",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowsmokepart,
  //   "set_showsmokepart",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowsprinkpart,
  //   "set_showsprinkpart",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowstreak, "set_showstreak",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowterrain,
  //   "set_showterrain", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetShowthreshold, "set_showthreshold",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowticks, "set_showticks",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowtimebar,
  //   "set_showtimebar", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetShowtimelabel, "set_showtimelabel",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowtitle, "set_showtitle",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowtracersalways,
  //                           "set_showtracersalways", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowtriangles,
  //   "set_showtriangles",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowtransparent,
  //   "set_showtransparent",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowtranparentvents,
  //                           "set_showtransparentvents", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowtrianglecount,
  //                           "set_showtrianglecount", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowventflow,
  //   "set_showventflow",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowvents, "set_showvents",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowwalls,
  //   "set_showwalls", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetSkipembedslice, "set_skipembedslice",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetSmokesensors,
  //   "set_smokesensors",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetStereo, "set_stereo", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSurfinc, "set_surfinc",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetTerrainparams,
  //   "set_terrainparams",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTitlesafe, "set_titlesafe",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetTrainermode,
  //   "set_trainermode", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetTrainerview, "set_trainerview", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTransparent,
  //   "set_transparent", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetTreeparms, "set_treeparms", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTwosidedvents,
  //   "set_twosidedvents",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetVectorskip, "set_vectorskip",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetVolsmoke,
  //   "set_volsmoke", NULL); jrpc_register_procedure(server, &jsonrpc_SetZoom,
  //   "set_zoom", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetCellcentertext, "set_cellcentertext",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetInputfile, "set_inputfile",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetLabelstartupview,
  //                           "set_labelstartupview", NULL);
  //   // { "set_pixelskip", lua_set_pixelskip },
  //   jrpc_register_procedure(server, &jsonrpc_SetRenderclip, "set_renderclip",
  //   NULL);
  //   // { "set_renderfilelabel", lua_set_renderfilelabel },
  //   jrpc_register_procedure(server, &jsonrpc_SetRenderfiletype,
  //   "set_renderfiletype",
  //                           NULL);

  //   // { "set_skybox", lua_set_skybox },
  //   // { "set_renderoption", lua_set_renderoption },
  //   jrpc_register_procedure(server, &jsonrpc_GetUnits, "get_units", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetUnitclass, "get_unitclass",
  //   NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetPl3dBoundMin,
  //   "set_pl3d_bound_min",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetPl3dBoundMax,
  //   "set_pl3d_bound_max",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetUnits, "set_units", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetUnitclasses,
  //   "set_unitclasses", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetZaxisangles, "set_zaxisangles", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetColorbartype,
  //   "set_colorbartype",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetExtremecolors,
  //   "set_extremecolors",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetFirecolor, "set_firecolor",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetFirecolormap,
  //   "set_firecolormap",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetFiredepth, "set_firedepth",
  //   NULL);
  //   // { "set_golorbar", lua_set_gcolorbar },
  //   jrpc_register_procedure(server, &jsonrpc_SetShowextremedata,
  //   "set_showextremedata",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSmokecolor, "set_smokecolor",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetSmokecull,
  //   "set_smokecull", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetSmokeskip, "set_smokeskip", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSmokealbedo,
  //   "set_smokealbedo", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetShowhazardcolors,
  //                           "set_showhazardcolors", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowhzone, "set_showhzone",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetShowszone,
  //   "set_showszone", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetShowvzone, "set_showvzone", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowzonefire,
  //   "set_showzonefire",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowpathnodes,
  //   "set_showpathnodes",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowtourroute,
  //   "set_showtourroute",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTourcolorsSelectedpathline,
  //                           "set_tourcolors_selectedpathline", NULL);
  //   jrpc_register_procedure(&server,
  //   &jsonrpc_SetTourcolorsSelectedpathlineknots,
  //                           "set_tourcolors_selectedpathlineknots", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTourcolorsSelectedknot,
  //                           "set_tourcolors_selectedknot", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTourcolorsPathline,
  //                           "set_tourcolors_pathline", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTourcolorsPathknots,
  //                           "set_tourcolors_pathknots", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTourcolorsText,
  //   "set_tourcolors_text",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTourcolorsAvatar,
  //                           "set_tourcolors_avatar", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetViewalltours,
  //   "set_viewalltours",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetViewtimes, "set_viewtimes",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetViewtourfrompath,
  //                           "set_viewtourfrompath", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetDevicevectordimensions,
  //                           "set_devicevectordimensions", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetDevicebounds,
  //   "set_devicebounds",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetDeviceorientation,
  //                           "set_deviceorientation", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetGridparms, "set_gridparms",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetGsliceparms,
  //   "set_gsliceparms", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetLoadfilesatstartup,
  //                           "set_loadfilesatstartup", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetMscale, "set_mscale", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetSliceauto, "set_sliceauto",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetMsliceauto,
  //   "set_msliceauto", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetCompressauto, "set_compressauto",
  //                           NULL);
  //   // { "set_part5propdisp", lua_set_part5propdisp },
  //   // { "set_part5color", lua_set_part5color },
  //   jrpc_register_procedure(server, &jsonrpc_SetPropindex, "set_propindex",
  //   NULL);
  //   // { "set_shooter", lua_set_shooter },
  //   jrpc_register_procedure(server, &jsonrpc_SetShowdevices,
  //   "set_showdevices", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetShowdevicevals, "set_showdevicevals",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetShowmissingobjects,
  //                           "set_showmissingobjects", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetTourindex, "set_tourindex",
  //   NULL);
  //   // { "set_userticks", lua_set_userticks },
  //   jrpc_register_procedure(server, &jsonrpc_SetCParticles,
  //   "set_c_particles", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetCSlice, "set_c_slice", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SetCacheBoundarydata,
  //                           "set_cache_boundarydata", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SetCacheQdata,
  //   "set_cache_qdata", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_SetTimeoffset, "set_timeoffset",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetTload, "set_tload",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetVSlice, "set_v_slice",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SetPatchdataout,
  //   "set_patchdataout",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_ShowSmoke3dShowall,
  //                           "show_smoke3d_showall", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ShowSmoke3dHideall,
  //                           "show_smoke3d_hideall", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ShowSlicesShowall,
  //   "show_slices_showall",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_ShowSlicesHideall,
  //   "show_slices_hideall",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_AddTitleLine, "add_title_line",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_ClearTitleLines,
  //   "clear_title_lines",
  //                           NULL);

  //   jrpc_register_procedure(server, &jsonrpc_GetNglobalTimes,
  //   "get_nglobal_times",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetGlobalTime,
  //   "get_global_time", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_GetNpartinfo, "get_npartinfo", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_GetSlice, "get_slice", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SliceGetLabel,
  //   "slice_get_label", NULL); jrpc_register_procedure(server,
  //   &jsonrpc_SliceGetFilename, "slice_get_filename",
  //                           NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SliceGetData, "slice_get_data",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_SliceDataMapFrames,
  //                           "slice_data_map_frames", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SliceDataMapFramesCountLess,
  //                           "slice_data_map_frames_count_less", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SliceDataMapFramesCountLessEq,
  //                           "slice_data_map_frames_count_less_eq", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SliceDataMapFramesCountGreater,
  //                           "slice_data_map_frames_count_greater", NULL);
  //   jrpc_register_procedure(server,
  //   &jsonrpc_SliceDataMapFramesCountGreaterEq,
  //                           "slice_data_map_frames_count_greater_eq", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_SliceGetTimes,
  //   "slice_get_times", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_GetPart, "get_part", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetPartNpoints,
  //   "get_part_npoints", NULL);

  //   jrpc_register_procedure(server, &jsonrpc_GetQdataSum, "get_qdata_sum",
  //   NULL); jrpc_register_procedure(server, &jsonrpc_GetQdataSumBounded,
  //                           "get_qdata_sum_bounded", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetQdataMaxBounded,
  //                           "get_qdata_max_bounded", NULL);
  //   jrpc_register_procedure(server, &jsonrpc_GetQdataMean, "get_qdata_mean",
  //   NULL);
  return 0;
}

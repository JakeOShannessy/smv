#define INMAIN
#include "options.h"
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "datadefs.h"
#include "dmalloc.h"
#include "shared_structures.h"

#include "smokeviewdefs.h"
#include "string_util.h"
#include <math.h>
#include "readlabel.h"

#include <json-c/json_object.h>

#ifndef _WIN32
#include <libgen.h>
#endif

int ReadSMV_Init(smv_case *scase);
int ReadSMV_Parse(smv_case *scase, bufferstreamdata *stream);
void ReadSMVDynamic(smv_case *scase, char *file);
void ReadSMVOrig(smv_case *scase);

smv_case global_scase = {
    .tourcoll = {.ntourinfo = 0,
                 .tourinfo = NULL,
                 .tour_ntimes = 1000,
                 .tour_t = NULL,
                 .tour_t2 = NULL,
                 .tour_dist = NULL,
                 .tour_dist2 = NULL,
                 .tour_dist3 = NULL,
                 .tour_tstart = 0.0,
                 .tour_tstop = 100.0},
    .fuel_hoc = -1.0,
    .fuel_hoc_default = -1.0,
    .have_cface_normals = CFACE_NORMALS_NO,
    .gvecphys = {0.0, 0.0, -9.8},
    .gvecunit = {0.0, 0.0, -1.0},
    .global_tbegin = 1.0,
    .global_tend = 0.0,
    .tload_begin = 0.0,
    .tload_end = 0.0,
    .load_hrrpuv_cutoff = 200.0,
    .global_hrrpuv_cutoff = 200.0,
    .global_hrrpuv_cutoff_default = 200.0,
    .smoke_albedo = 0.3,
    .smoke_albedo_base = 0.3,
    .xbar = 1.0,
    .ybar = 1.0,
    .zbar = 1.0,
    .show_slice_in_obst = ONLY_IN_GAS,
    .use_iblank = 1,
    .visOtherVents = 1,
    .visOtherVentsSAVE = 1,
    .hvac_duct_color = {63, 0, 15},
    .hvac_node_color = {63, 0, 15},
    .nrgb2 = 8,
    .pref = 101325.0,
    .pamb = 0.0,
    .tamb = 293.15,
    .nrgb = NRGB,
    .linewidth = 2.0,
    .ventlinewidth = 2.0,
    .obst_bounding_box = {1.0, 0.0, 1.0, 0.0, 1.0, 0.0},
    .hvaccoll = {.hvacductvar_index = -1, .hvacnodevar_index = -1, 0}};

/// @brief Given a file path, get the filename excluding the final extension.
/// This allocates a new copy which can be deallocated with free().
/// @param input_file a file path
/// @return an allocated string containing the basename or NULL on failure.
char *GetBaseName(const char *input_file) {
  if(input_file == NULL) return NULL;
#ifdef _WIN32
  char *result = malloc(_MAX_FNAME + 1);
  errno_t err =
      _splitpath_s(input_file, NULL, 0, NULL, 0, result, _MAX_FNAME, NULL, 0);
  if(err) return NULL;
#else
  // POSIX basename can modify it's contents, so we'll make some copies.
  char *input_file_temp = strdup(input_file);
  // Get the filename (final component of the path, including any extensions).
  char *bname = basename(input_file_temp);
  // If a '.' exists, set it to '\0' to trim the extension.
  char *dot = strrchr(bname, '.');
  if(dot) *dot = '\0';
  char *result = strdup(bname);
  free(input_file_temp);
#endif
  return result;
}

int SetGlobalFilenames(smv_case *scase) {
  int len_casename = strlen(scase->fdsprefix);

  FREEMEMORY(scase->paths.log_filename);
  NewMemory((void **)&scase->paths.log_filename, len_casename + strlen(".smvlog") + 1);
  STRCPY(scase->paths.log_filename, scase->fdsprefix);
  STRCAT(scase->paths.log_filename, ".smvlog");

  FREEMEMORY(scase->paths.caseini_filename);
  NewMemory((void **)&scase->paths.caseini_filename, len_casename + strlen(".ini") + 1);
  STRCPY(scase->paths.caseini_filename, scase->fdsprefix);
  STRCAT(scase->paths.caseini_filename, ".ini");

  FREEMEMORY(scase->paths.expcsv_filename);
  NewMemory((void **)&scase->paths.expcsv_filename, len_casename + strlen("_exp.csv") + 1);
  STRCPY(scase->paths.expcsv_filename, scase->fdsprefix);
  STRCAT(scase->paths.expcsv_filename, "_exp.csv");

  FREEMEMORY(scase->paths.dEcsv_filename);
  NewMemory((void **)&scase->paths.dEcsv_filename, len_casename + strlen("_dE.csv") + 1);
  STRCPY(scase->paths.dEcsv_filename, scase->fdsprefix);
  STRCAT(scase->paths.dEcsv_filename, "_dE.csv");

  FREEMEMORY(scase->paths.html_filename);
  NewMemory((void **)&scase->paths.html_filename, len_casename + strlen(".html") + 1);
  STRCPY(scase->paths.html_filename, scase->fdsprefix);
  STRCAT(scase->paths.html_filename, ".html");

  FREEMEMORY(scase->paths.smv_orig_filename);
  NewMemory((void **)&scase->paths.smv_orig_filename, len_casename + strlen(".smo") + 1);
  STRCPY(scase->paths.smv_orig_filename, scase->fdsprefix);
  STRCAT(scase->paths.smv_orig_filename, ".smo");

  FREEMEMORY(scase->paths.hrr_filename);
  NewMemory((void **)&scase->paths.hrr_filename, len_casename + strlen("_hrr.csv") + 1);
  STRCPY(scase->paths.hrr_filename, scase->fdsprefix);
  STRCAT(scase->paths.hrr_filename, "_hrr.csv");

  FREEMEMORY(scase->paths.htmlvr_filename);
  NewMemory((void **)&scase->paths.htmlvr_filename, len_casename + strlen("_vr.html") + 1);
  STRCPY(scase->paths.htmlvr_filename, scase->fdsprefix);
  STRCAT(scase->paths.htmlvr_filename, "_vr.html");

  FREEMEMORY(scase->paths.htmlobst_filename);
  NewMemory((void **)&scase->paths.htmlobst_filename,
            len_casename + strlen("_obst.json") + 1);
  STRCPY(scase->paths.htmlobst_filename, scase->fdsprefix);
  STRCAT(scase->paths.htmlobst_filename, "_obst.json");

  FREEMEMORY(scase->paths.htmlslicenode_filename);
  NewMemory((void **)&scase->paths.htmlslicenode_filename,
            len_casename + strlen("_slicenode.json") + 1);
  STRCPY(scase->paths.htmlslicenode_filename, scase->fdsprefix);
  STRCAT(scase->paths.htmlslicenode_filename, "_slicenode.json");

  FREEMEMORY(scase->paths.htmlslicecell_filename);
  NewMemory((void **)&scase->paths.htmlslicecell_filename,
            len_casename + strlen("_slicecell.json") + 1);
  STRCPY(scase->paths.htmlslicecell_filename, scase->fdsprefix);
  STRCAT(scase->paths.htmlslicecell_filename, "_slicecell.json");

  FREEMEMORY(scase->paths.event_filename);
  NewMemory((void **)&scase->paths.event_filename, len_casename + strlen("_events.csv") + 1);
  STRCPY(scase->paths.event_filename, scase->fdsprefix);
  STRCAT(scase->paths.event_filename, "_events.csv");

  if (scase->paths.ffmpeg_command_filename == NULL) {
    NewMemory((void **)&scase->paths.ffmpeg_command_filename,
              (unsigned int)(len_casename + 12));
    STRCPY(scase->paths.ffmpeg_command_filename, scase->fdsprefix);
    STRCAT(scase->paths.ffmpeg_command_filename, "_ffmpeg");
#ifdef WIN32
    STRCAT(scase->paths.ffmpeg_command_filename, ".bat");
#else
    STRCAT(scase->paths.ffmpeg_command_filename, ".sh");
#endif
  }
  if (scase->paths.stop_filename == NULL) {
    NewMemory((void **)&scase->paths.stop_filename,
              (unsigned int)(len_casename + strlen(".stop") + 1));
    STRCPY(scase->paths.stop_filename, scase->fdsprefix);
    STRCAT(scase->paths.stop_filename, ".stop");
  }
  if (scase->paths.smvzip_filename == NULL) {
    NewMemory((void **)&scase->paths.smvzip_filename,
              (unsigned int)(len_casename + strlen(".smvzip") + 1));
    STRCPY(scase->paths.smvzip_filename, scase->fdsprefix);
    STRCAT(scase->paths.smvzip_filename, ".smvzip");
  }
  if (scase->paths.sliceinfo_filename == NULL) {
    NewMemory((void **)&scase->paths.sliceinfo_filename,
              strlen(scase->fdsprefix) + strlen(".sinfo") + 1);
    STRCPY(scase->paths.sliceinfo_filename, scase->fdsprefix);
    STRCAT(scase->paths.sliceinfo_filename, ".sinfo");
  }
  if (scase->paths.deviceinfo_filename == NULL) {
    NewMemory((void **)&scase->paths.deviceinfo_filename,
              strlen(scase->fdsprefix) + strlen("_device.info") + 1);
    STRCPY(scase->paths.deviceinfo_filename, scase->fdsprefix);
    STRCAT(scase->paths.deviceinfo_filename, "_device.info");
  }

  // if smokezip created part2iso files then concatenate .smv entries found in
  // the .isosmv file to the end of the .smv file creating a new .smv file. Then
  // read in that .smv file.

  {
    FILE *stream_iso = NULL;

    NewMemory((void **)&scase->paths.iso_filename, len_casename + strlen(".isosmv") + 1);
    STRCPY(scase->paths.iso_filename, scase->fdsprefix);
    STRCAT(scase->paths.iso_filename, ".isosmv");
    stream_iso = fopen(scase->paths.iso_filename, "r");
    if (stream_iso != NULL) {
      fclose(stream_iso);
    }
    else {
      FREEMEMORY(scase->paths.iso_filename);
    }
  }

  if (scase->paths.trainer_filename == NULL) {
    NewMemory((void **)&scase->paths.trainer_filename, (unsigned int)(len_casename + 6));
    STRCPY(scase->paths.trainer_filename, scase->fdsprefix);
    STRCAT(scase->paths.trainer_filename, ".svd");
  }
  if (scase->paths.test_filename == NULL) {
    NewMemory((void **)&scase->paths.test_filename, (unsigned int)(len_casename + 6));
    STRCPY(scase->paths.test_filename, scase->fdsprefix);
    STRCAT(scase->paths.test_filename, ".svd");
  }
  return 0;
}

int PrintJson(smv_case *scase) {
  struct json_object *jobj = json_object_new_object();
  json_object_object_add(jobj, "version", json_object_new_int(1));
  json_object_object_add(jobj, "chid",
                         json_object_new_string(scase->paths.chidfilebase));
  if(scase->paths.fds_filein != NULL) {
    json_object_object_add(jobj, "input_file",
                           json_object_new_string(scase->paths.fds_filein));
  }
  if(scase->fds_title != NULL) {
    json_object_object_add(jobj, "title",
                           json_object_new_string(scase->fds_title));
  }
  if(scase->fds_version != NULL) {
  json_object_object_add(jobj, "fds_version",
                         json_object_new_string(scase->fds_version));
  }
  struct json_object *mesh_array = json_object_new_array();
  for(int i = 0; i < scase->meshescoll.nmeshes; i++) {
    meshdata *mesh = &scase->meshescoll.meshinfo[i];
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
                           json_object_new_double(mesh->x0));
    json_object_object_add(mesh_dimensions, "x_max",
                           json_object_new_double(mesh->x1));
    json_object_object_add(mesh_dimensions, "y_min",
                           json_object_new_double(mesh->y0));
    json_object_object_add(mesh_dimensions, "y_max",
                           json_object_new_double(mesh->y1));
    json_object_object_add(mesh_dimensions, "z_min",
                           json_object_new_double(mesh->z0));
    json_object_object_add(mesh_dimensions, "z_max",
                           json_object_new_double(mesh->z1));
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

    json_object_array_add(mesh_array, mesh_obj);
  }
  json_object_object_add(jobj, "meshes", mesh_array);

  // TODO: the parse rejects CSV files that it doesn't find in it's own working
  // directory.
  struct json_object *csv_files = json_object_new_array();
  for(int i = 0; i < scase->csvcoll.ncsvfileinfo; i++) {
    csvfiledata *csv_file = &scase->csvcoll.csvfileinfo[i];
    struct json_object *csv_obj = json_object_new_object();
    json_object_object_add(csv_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(csv_obj, "filename",
                           json_object_new_string(csv_file->file));
    json_object_object_add(csv_obj, "type",
                           json_object_new_string(csv_file->c_type));
    json_object_array_add(csv_files, csv_obj);
  }
  json_object_object_add(jobj, "csv_files", csv_files);

  // Add devices to JSON
  struct json_object *devices = json_object_new_array();
  for(int i = 0; i < scase->devicecoll.ndeviceinfo; i++) {
    devicedata *device = &scase->devicecoll.deviceinfo[i];
    struct json_object *device_obj = json_object_new_object();
    json_object_object_add(device_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(device_obj, "id",
                           json_object_new_string(device->deviceID));
    json_object_object_add(device_obj, "csvlabel",
                           json_object_new_string(device->csvlabel));
    json_object_object_add(device_obj, "label",
                           json_object_new_string(device->labelptr));
    json_object_object_add(device_obj, "quantity",
                           json_object_new_string(device->quantity));
    if(device->have_xyz) {
      struct json_object *device_position = json_object_new_object();
      json_object_object_add(device_position, "x",
                             json_object_new_double(device->xyz[0]));
      json_object_object_add(device_position, "y",
                             json_object_new_double(device->xyz[1]));
      json_object_object_add(device_position, "z",
                             json_object_new_double(device->xyz[2]));
      json_object_object_add(device_obj, "position", device_position);
    }
    if(device->act_times != NULL) {
      struct json_object *state_changes = json_object_new_array();
      for(int j = 0; j < device->nstate_changes; j++) {
        struct json_object *state_change = json_object_new_object();
        json_object_object_add(state_change, "time",
                               json_object_new_double(device->act_times[j]));
        json_object_object_add(state_change, "value",
                               json_object_new_int(device->state_values[j]));
        json_object_array_add(state_changes, state_change);
      }
      json_object_object_add(device_obj, "state_changes", state_changes);
    }
    json_object_array_add(devices, device_obj);
  }
  json_object_object_add(jobj, "devices", devices);

  // Add slices to JSON
  struct json_object *slices = json_object_new_array();
  for(int i = 0; i < scase->slicecoll.nsliceinfo; i++) {
    slicedata *slice = &scase->slicecoll.sliceinfo[i];
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
  json_object_object_add(jobj, "slices", slices);

  // Add surfaces to JSON
  struct json_object *surfaces = json_object_new_array();
  for(int i = 0; i < scase->surfcoll.nsurfinfo; i++) {
    surfdata *surf = &scase->surfcoll.surfinfo[i];
    struct json_object *surf_obj = json_object_new_object();
    json_object_object_add(surf_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(surf_obj, "id",
                           json_object_new_string(surf->surfacelabel));
    json_object_array_add(surfaces, surf_obj);
  }
  json_object_object_add(jobj, "surfaces", surfaces);

  // Add materials to JSON
  struct json_object *materials = json_object_new_array();
  for(int i = 0; i < scase->surfcoll.nsurfinfo; i++) {
    surfdata *surf = &scase->surfcoll.surfinfo[i];
    struct json_object *surf_obj = json_object_new_object();
    json_object_object_add(surf_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(surf_obj, "id",
                           json_object_new_string(surf->surfacelabel));
    json_object_array_add(materials, surf_obj);
  }
  json_object_object_add(jobj, "surfaces", materials);

  const char *json_output =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  printf("%s\n", json_output);
  json_object_put(jobj);
  return 0;
}

extern CCC float *block_ambient2;

int RunBenchmark(char *input_file, const char *fdsprefix) {
  initMALLOC();
  // float mat_specular_orig[4];
  // mat_specular_orig[0] = 0.5f;
  // mat_specular_orig[1] = 0.5f;
  // mat_specular_orig[2] = 0.2f;
  // mat_specular_orig[3] = 1.0f;
  // mat_specular2 = GetColorPtr(&global_scase, mat_specular_orig);

  float mat_ambient_orig[4];
  mat_ambient_orig[0] = 0.5f;
  mat_ambient_orig[1] = 0.5f;
  mat_ambient_orig[2] = 0.2f;
  mat_ambient_orig[3] = 1.0f;
  mat_ambient2 = GetColorPtr(&global_scase, mat_ambient_orig);

  float ventcolor_orig[4];
  ventcolor_orig[0] = 1.0;
  ventcolor_orig[1] = 0.0;
  ventcolor_orig[2] = 1.0;
  ventcolor_orig[3] = 1.0;
  ventcolor = GetColorPtr(&global_scase, ventcolor_orig);

  float block_ambient_orig[4];
  block_ambient_orig[0] = 1.0;
  block_ambient_orig[1] = 0.8;
  block_ambient_orig[2] = 0.4;
  block_ambient_orig[3] = 1.0;
  block_ambient2 = GetColorPtr(&global_scase, block_ambient_orig);

  // float block_specular_orig[4];
  // block_specular_orig[0] = 0.0;
  // block_specular_orig[1] = 0.0;
  // block_specular_orig[2] = 0.0;
  // block_specular_orig[3] = 1.0;
  // block_specular2 = GetColorPtr(&global_scase, block_specular_orig);

  smv_case *scase;
  NEWMEMORY(scase, sizeof(smv_case));
  memset(scase, 0, sizeof(smv_case));
  NEWMEMORY(scase->fdsprefix, (strlen(fdsprefix) + 1) * sizeof(char));
  STRCPY(scase->fdsprefix, fdsprefix);
  SetGlobalFilenames(scase);

  INIT_PRINT_TIMER(parse_time);
  fprintf(stderr, "reading:\t%s\n", input_file);
  {
    bufferstreamdata *smv_streaminfo = GetSMVBuffer(input_file);
    if(smv_streaminfo == NULL) {
      fprintf(stderr, "could not open %s\n", input_file);
      return 1;
    }
    INIT_PRINT_TIMER(ReadSMV_time);
    int return_code = 0;
    return_code = ReadSMV_Init(scase);
    return_code = ReadSMV_Parse(scase, smv_streaminfo);
    STOP_TIMER(ReadSMV_time);
    fprintf(stderr, "ReadSMV:\t%8.3f ms\n", ReadSMV_time * 1000);
    if(smv_streaminfo != NULL) {
      FCLOSE(smv_streaminfo);
    }
    if(return_code) return return_code;
  }
  show_timings = 1;
  ReadSMVOrig(scase);
  INIT_PRINT_TIMER(ReadSMVDynamic_time);
  ReadSMVDynamic(scase, input_file);
  STOP_TIMER(ReadSMVDynamic_time);
  fprintf(stderr, "ReadSMVDynamic:\t%8.3f ms\n", ReadSMVDynamic_time * 1000);
  STOP_TIMER(parse_time);
  fprintf(stderr, "Total Time:\t%8.3f ms\n", parse_time * 1000);
  PrintJson(scase);
  // FreeVars();
  return 0;
}

int main(int argc, char **argv) {

  bool print_help = false;
  bool print_version = false;

  int c;

  opterr = 0;

  while((c = getopt(argc, argv, "hV")) != -1)
    switch(c) {
    case 'h':
      print_help = true;
      break;
    case 'V':
      print_version = true;
      break;
    case '?':
      if(isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      return 1;
    default:
      abort();
    }
  if(print_help) {
    printf("smvq-%s\n", PROGVERSION);
    printf("\nUsage:  smvq [OPTIONS] <FILE>\n");
    printf("\nOptions:\n");
    printf("  -h Print help\n");
    printf("  -V Print version\n");
    return 0;
  }
  if(print_version) {
    printf("smvq - smv query processor (v%s)\n", PROGVERSION);
    return 0;
  }
  char *input_file = argv[optind];

  if(input_file == NULL) {
    fprintf(stderr, "No input file specified.\n");
    return 1;
  }
  char *fdsprefix = GetBaseName(input_file);
  int result = RunBenchmark(input_file, fdsprefix);
  return result;
}

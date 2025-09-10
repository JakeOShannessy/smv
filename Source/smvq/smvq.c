#define INMAIN
#include "options.h"
// _DEFAULT_SOURCE needs to be switched on to access strdup.
#define _DEFAULT_SOURCE

#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>

#include "datadefs.h"
#include "dmalloc.h"
#include "shared_structures.h"

#include "readlabel.h"
#include "readsmvfile.h"
#include "smokeviewdefs.h"
#include "string_util.h"

#include "paths.h"

#include "csvs/smv_csvs.h"
#include <json-c/json_object.h>

#ifdef _WIN32
#include <direct.h>
#include <dirent_win.h>
#include <io.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
// disable warning about strdup
#pragma warning(disable : 4996)
#else
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#endif

int PrintStepsJson(const char *path) {
  struct step_iter iter = steps_iter_new(path);
  for(;;) {
    struct simulation_step step = {0};
    int ret = steps_iter_next(&iter, &step);
    if(ret) break;
    struct json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "index", json_object_new_int(step.index));
    json_object_object_add(jobj, "wall_time",
                           json_object_new_string(step.wall_time));
    json_object_object_add(jobj, "step_size",
                           json_object_new_double(step.step_size));
    json_object_object_add(jobj, "simulation_time",
                           json_object_new_double(step.simulation_time));
    json_object_object_add(jobj, "cpu_time",
                           json_object_new_double(step.cpu_time));
    const char *json_output =
        json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    printf("%s\n", json_output);
    // TODO: consider just reusing this rather than freeing it here
    json_object_put(jobj);
  }
  steps_iter_close(&iter);
  return 0;
}

int PrintHrrJson(const char *path) {
  struct float_iter iter = float_iter_new(path, "Time", "HRR");
  for(;;) {
    struct data_value step = {0};
    int ret = float_iter_next(&iter, &step);
    if(ret) break;
    struct json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "x", json_object_new_double(step.x));
    json_object_object_add(jobj, "y", json_object_new_double(step.y));
    const char *json_output =
        json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    printf("%s\n", json_output);
    // TODO: consider just reusing this rather than freeing it here
    json_object_put(jobj);
  }
  float_iter_close(&iter);
  return 0;
}

int PrintCsvJsonArray(const char *path, const char *csv_x, const char *csv_y) {
  struct float_iter iter = float_iter_new(path, csv_x, csv_y);
  struct json_object *jobj = json_object_new_object();
  struct json_object *values = json_object_new_array();
  for(;;) {
    struct json_object *dv = json_object_new_object();
    struct data_value step = {0};
    int ret = float_iter_next(&iter, &step);
    if(ret) break;
    json_object_object_add(dv, "x", json_object_new_double(step.x));
    json_object_object_add(dv, "y", json_object_new_double(step.y));
    json_object_array_add(values, dv);
  }
  json_object_object_add(jobj, "values", values);
  json_object_object_add(jobj, "name", json_object_new_string(csv_y));
  json_object_object_add(
      jobj, "x_units",
      json_object_new_string(iter.state.col_units[iter.state.x_col_offset]));
  json_object_object_add(jobj, "x_name",
                         json_object_new_string(iter.state.x_name));
  json_object_object_add(
      jobj, "y_units",
      json_object_new_string(iter.state.col_units[iter.state.y_col_offset]));
  json_object_object_add(jobj, "y_name",
                         json_object_new_string(iter.state.y_name));

  const char *json_output =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  printf("%s\n", json_output);
  json_object_put(jobj);
  float_iter_close(&iter);
  return 0;
}

int PrintJson(smv_case *scase) {
  struct json_object *jobj = json_object_new_object();
  json_object_object_add(jobj, "version", json_object_new_int(1));
  json_object_object_add(jobj, "chid",
                         json_object_new_string(scase->chidfilebase));
  if(scase->fds_filein != NULL) {
    json_object_object_add(jobj, "input_file",
                           json_object_new_string(scase->fds_filein));
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
struct csv_opts {
  char *csv_type;
  char *csv_x;
  char *csv_y;
};

struct opts {
  bool print_help;
  bool print_version;
  bool print_steps;
  char *print_out_value;
  struct csv_opts *csv_opts;
};

static bool IsValidCsvOpts(struct csv_opts *csv_opts) {
  return csv_opts->csv_type != NULL && csv_opts->csv_x != NULL &&
         csv_opts->csv_y != NULL;
}

static bool IsValidOpts(struct opts *opts) {
  int nflags = 0;
  if(opts->print_help) nflags++;
  if(opts->print_version) nflags++;
  if(opts->print_steps) nflags++;
  if(opts->print_out_value != NULL) nflags++;
  if(opts->csv_opts != NULL) nflags++;
  if(nflags > 1) {
    fprintf(stderr, "ERROR: multiple options selected\n");
    return false;
  }
  if(opts->csv_opts != NULL && !IsValidCsvOpts(opts->csv_opts)) {
    fprintf(stderr, "ERROR: CSV options specificied, but invalid. -f, -x, "
                    "and -y must be specified together.\n");
    return false;
  }
  return true;
}

char *DirPath(const char *path) {
#ifdef _WIN32
  char *dir = strdup(path);
  PathRemoveFileSpecA(dir);
#else
  char *dtemp = strdup(path);
  char *dir = strdup(dirname(dtemp));
  free(dtemp);
#endif
  return dir;
}

int RunSmvq(char *filepath, struct opts *opts) {
  // Create and initialize an smv_case struct
  smv_case *scase = CreateScase();
  // Parse a file at the given path into scase
  int result = ScaseParseFromPath(filepath, scase);
  // Error handling on parse failure
  if(result) {
    fprintf(stderr, "failed to read smv file: %s\n", filepath);
    return result;
  }
  if(opts->print_steps) {
    char *p = CasePathStepCsv(scase);
    PrintStepsJson(p);
    FreeMemory(p);
  }
  else if(opts->print_out_value != NULL) {
    enum OutValue out_value;
    if(!strcmp(opts->print_out_value, "run-data")) {
      out_value = RUN_DATA;
    }
    else if(!strcmp(opts->print_out_value, "pressure-error")) {
      out_value = PRESSURE_ERROR;
    }
    else if(!strcmp(opts->print_out_value, "velocity-error")) {
      out_value = VELOCITY_ERROR;
    }
    else if(!strcmp(opts->print_out_value, "pressure-iterations")) {
      out_value = PRESSURE_ITERATIONS;
    }
    else if(!strcmp(opts->print_out_value, "progress")) {
      out_value = PROGRESS;
    }
    else {
      fprintf(stderr, "%s is not a valid parse_out command\n",
              opts->print_out_value);
      return 1;
    }
    char *dir = DirPath(filepath);
    char outfile[256];
    strcpy(outfile, scase->fdsprefix);
    strcat(outfile, ".out");
    char *path = CombinePaths(dir, outfile);
    PrintOutData(path, out_value);
    FreeMemory(path);
    free(dir);
  }
  else if(opts->csv_opts != NULL) {
    fprintf(stderr, "print_csv: %s %s %s\n", opts->csv_opts->csv_type,
            opts->csv_opts->csv_x, opts->csv_opts->csv_y);
    char *dir = DirPath(filepath);
    char *csv_filename = NULL;
    // TODO: should be using CHID not fdsprefix
    NewMemory(
        (void **)&csv_filename,
        (strlen(scase->fdsprefix) + strlen(opts->csv_opts->csv_type) + 5 + 1) *
            sizeof(char));
    strcpy(csv_filename, scase->fdsprefix);
    strcat(csv_filename, "_");
    strcat(csv_filename, opts->csv_opts->csv_type);
    strcat(csv_filename, ".csv");
    char *csv_path = CombinePaths(dir, csv_filename);
    FreeMemory(csv_filename);
    fprintf(stderr, "csv_path: %s\n", csv_path);
    PrintCsvJsonArray(csv_path, opts->csv_opts->csv_x, opts->csv_opts->csv_y);
    FreeMemory(csv_path);
    free(dir);
  }
  else {
    PrintJson(scase);
  }
  DestroyScase(scase);
  return 0;
}

#if defined(_WIN32) && defined(pp_UNICODE_PATHS)
int wmain(int argc, wchar_t **argv)
#else
int main(int argc, char **argv)
#endif
{
  initMALLOC();

  struct opts opts = {0};
  struct csv_opts csv_opts = {0};

  bool print_help = false;
  bool print_version = false;

  int c;

  opterr = 0;
#if defined(_WIN32) && defined(pp_UNICODE_PATHS)
  while((c = getopt_w(argc, argv, L"hVsp:f:x:y:")) != -1)
#elif defined(_WIN32)
  while((c = getopt_a(argc, argv, "hVsp:f:x:y:")) != -1)
#else
  while((c = getopt(argc, argv, "hVsp:f:x:y:")) != -1)
#endif
    switch(c) {
    case 'h':
      opts.print_help = true;
      break;
    case 'V':
      opts.print_version = true;
      break;
    case 's':
      opts.print_steps = true;
      break;
    case 'p':
      opts.print_out_value = optarg;
      break;
    case 'f':
      if(opts.csv_opts == NULL) opts.csv_opts = &csv_opts;
      csv_opts.csv_type = optarg;
      break;
    case 'x':
      csv_opts.csv_x = optarg;
      break;
    case 'y':
      csv_opts.csv_y = optarg;
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
  if(IsValidCsvOpts(&csv_opts)) {
    opts.csv_opts = &csv_opts;
  }
  if(!IsValidOpts(&opts)) {
    fprintf(stderr, "invalid commandline parameters");
    return 1;
  }
  if(opts.print_help) {
    printf("smvq-%s\n", PROGVERSION);
    printf("\nUsage:  smvq [OPTIONS] <FILE>\n");
    printf("\nOptions:\n");
    printf("  -h Print help\n");
    printf("  -V Print version\n");
    printf("  -s Print time steps\n");
    printf("  -p <value> Print a given out value\n");
    printf(
        "  -f <value> Print a CSV entry of a given type (hrr, devc, etc.)\n");
    printf(
        "  -x <value> The name of x-value of the CSV data vector to export\n");
    printf(
        "  -y <value> The name of y-value of the CSV data vector to export\n");
    return 0;
  }

  if(opts.print_version) {
    printf("smvq - smv query processor (v%s)\n", PROGVERSION);
    return 0;
  }

#if defined(_WIN32) && defined(pp_UNICODE_PATHS)
  char *input_file = convert_utf16_to_utf8(argv[optind]);
#else
  char *input_file = argv[optind];
#endif

  int result = RunSmvq(input_file, &opts);
#if defined(_WIN32) && defined(pp_UNICODE_PATHS)
  FREEMEMORY(input_file);
#endif
  return result;
}

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <json-c/json_object.h>

#include "smv_csvs.h"

#define TABVALUE 4 // by default a tab counts as 4 spaces
#define MATCH 1
#define NOTMATCH 0
#define DATESTRING_BUFFERSIZE 256
#define MAX_LINE_LENGTH 4096

static char *DatetimeToString(char *time, struct tm tmdatetime) {
  strftime(time, DATESTRING_BUFFERSIZE - 1, "%Y-%m-%dT%H:%M:%SZ", &tmdatetime);
  //   time_t time_of_day = mktime(&tmdatetime);
  if(strlen(time) == 0) {
    fprintf(stderr, "empty timestring\n");
  }
  return time;
}

typedef struct TStepMeshProp {
  char *key;
  float *value;
  char *units;
  float *location;
} TStepMeshProp;

typedef struct TimestepMeshInfo {
  int number;
  TStepMeshProp props[];
} TimestepMeshInfo;
//       Mesh    1
//       Max CFL number:  0.27E-02 at ( 27, 14, 48)
//       Max divergence:  0.13E-05 at ( 27, 32, 48)
//       Min divergence: -0.12E-05 at ( 21, 29, 48)
//       Max VN number:   0.18E-02 at ( 26, 33, 28)

struct OutParserState {
  struct Timestep *timesteps;
  struct Timestep *first_empty_timestep;
  OutData outData;
  int timestepArrLength;

  int ntimesteps;
  float last_time, start_time, end_time;
  int currentLine;
  int currentCharacter;
};

static int CountSpaces(int *cnt, char *buffer) {
  int spaces = 0;
  int i = 0;
  int cont = 1;
  while(cont) {
    if(buffer[i] == ' ') {
      spaces += 1;
    }
    else if(buffer[i] == '\t') {
      spaces += TABVALUE;
    }
    else {
      cont = 0;
    }
    i++;
  }
  *cnt = spaces;
  return i - 1;
}

static int Match(char *buffer, const char *key) {
  if(strcmp(buffer, key) != 0) return NOTMATCH;
  return MATCH;
}

static int ParseMonth(char *string) {
  if(Match(string, "January")) return 0;
  if(Match(string, "February")) return 1;
  if(Match(string, "March")) return 2;
  if(Match(string, "April")) return 3;
  if(Match(string, "May")) return 4;
  if(Match(string, "June")) return 5;
  if(Match(string, "July")) return 6;
  if(Match(string, "August")) return 7;
  if(Match(string, "September")) return 8;
  if(Match(string, "October")) return 9;
  if(Match(string, "November")) return 10;
  if(Match(string, "December")) return 11;
  fprintf(stderr, "%s\n", "date parsing error\n");
  exit(1);
}

static int TimeStepLine(struct OutParserState *pstate, char *buffer) {
  int t;
  char month[100];
  int day, year, hour, minute, second;
  sscanf(buffer, "Time Step %d %s  %d, %d  %d:%d:%d", &t, month, &day, &year,
         &hour, &minute, &second);

  (*pstate->first_empty_timestep).number = t;
  struct tm str_time;

  str_time.tm_year = year - 1900;
  str_time.tm_mon = ParseMonth(month);
  str_time.tm_mday = day;
  str_time.tm_hour = hour;
  str_time.tm_min = minute;
  str_time.tm_sec = second;
  str_time.tm_isdst = 0;

  (*pstate->first_empty_timestep).time = str_time;
  char datetime[100];
  DatetimeToString(datetime, str_time);

  return 0;
}

static int ParseSecondLine(struct OutParserState *pstate, char *buffer) {
  float step_size, total_time;
  sscanf(buffer, "Step Size: %g s, Total Time: %f s", &step_size, &total_time);
  (*pstate->first_empty_timestep).simtime = total_time;
  (*pstate->first_empty_timestep).stepsize = step_size;
  pstate->last_time = total_time;
  return 0;
}

static int ParseOtherSecondLine(struct OutParserState *pstate, char *buffer) {
  float step_size, total_time;
  sscanf(buffer, "Time step: %g s, Total time: %f s", &step_size, &total_time);
  (*pstate->first_empty_timestep).simtime = total_time;
  (*pstate->first_empty_timestep).stepsize = step_size;
  pstate->last_time = total_time;
  return 0;
}

static int ParseThirdLine(struct OutParserState *pstate, char *buffer) {
  int pressure_iterations;
  sscanf(buffer, "Pressure Iterations:  %d", &pressure_iterations);
  (*pstate->first_empty_timestep).pressure_iterations = pressure_iterations;
  return 0;
}

static int ParseFourthLine(struct OutParserState *pstate, char *buffer) {
  float max_vel_err;
  sscanf(buffer, "Maximum Velocity Error:  %f", &max_vel_err);
  (*pstate->first_empty_timestep).max_vel_err = max_vel_err;
  return 0;
}

static int ParseFifthLine(struct OutParserState *pstate, char *buffer) {
  float max_pres_err;
  sscanf(buffer, "Maximum Pressure Error:  %f", &max_pres_err);
  (*pstate->first_empty_timestep).max_pres_err = max_pres_err;
  return 0;
}

static int RunTimeDiagnostics(struct OutParserState *pstate,
                              int indentationLevel, FILE *stream) {
  // the indentation level will not change.
  int current_spaces;
  while(!feof(stream)) {
    char buffer[MAX_LINE_LENGTH];
    pstate->currentLine++;
    if(fgets(buffer, MAX_LINE_LENGTH, stream) == NULL) break;
    buffer[strcspn(buffer, "\n")] = 0;
    size_t l = strlen(buffer);
    int read = CountSpaces(&current_spaces, buffer);
    if(l == 0 || current_spaces == l) continue;
    if((current_spaces > indentationLevel)) {
      // TODO: parse other information too
      if(Match(&buffer[read], "Time Step ")) {
        TimeStepLine(pstate, &buffer[read]);

        pstate->currentLine++;
        if(fgets(buffer, MAX_LINE_LENGTH, stream) == NULL) break;
        buffer[strcspn(buffer, "\n")] = 0;
        int read = CountSpaces(&current_spaces, buffer);

        if(Match(&buffer[read], "Time step:")) {
          ParseOtherSecondLine(pstate, &buffer[read]);
        }
        else if(Match(&buffer[read], "Step Size")) {
          ParseSecondLine(pstate, &buffer[read]);
        }
        else {
          fprintf(stderr, "failed to pass second line of timestep, line: %d\n",
                  pstate->currentLine);
          fprintf(stderr, "%s\n", buffer);
          exit(1);
        }

        pstate->currentLine++;
        if(fgets(buffer, MAX_LINE_LENGTH, stream) == NULL) break;
        buffer[strcspn(buffer, "\n")] = 0;
        read = CountSpaces(&current_spaces, buffer);
        ParseThirdLine(pstate, &buffer[read]);

        pstate->currentLine++;
        if(fgets(buffer, MAX_LINE_LENGTH, stream) == NULL) break;
        buffer[strcspn(buffer, "\n")] = 0;
        read = CountSpaces(&current_spaces, buffer);
        ParseFourthLine(pstate, &buffer[read]);

        pstate->currentLine++;
        if(fgets(buffer, MAX_LINE_LENGTH, stream) == NULL) break;
        buffer[strcspn(buffer, "\n")] = 0;
        read = CountSpaces(&current_spaces, buffer);
        ParseFifthLine(pstate, &buffer[read]);

        pstate->first_empty_timestep += 1;
        pstate->ntimesteps++;
        if(pstate->ntimesteps >= pstate->timestepArrLength) {
          pstate->timestepArrLength = 2 * pstate->timestepArrLength;
          pstate->timesteps =
              realloc(pstate->timesteps,
                      pstate->timestepArrLength * sizeof(struct Timestep));
          if(pstate->timesteps == NULL) {
            fprintf(stderr, "memory allocation failed");
            exit(1);
          }
          else {
            pstate->first_empty_timestep =
                &pstate->timesteps[pstate->ntimesteps];
          }
        }
        continue;
      }
      continue;
    }
    else {
      // if the number of preceding spaces return to that of the title
      // or even less, then we have finished parsing this section.
      return 0;
    }
  }
  return 0;
}

static int ParseMiscParameters(struct OutParserState *pstate,
                               int indentationLevel, FILE *stream) {
  // the indentation level will not change.
  int current_spaces;
  while(!feof(stream)) {
    char buffer[MAX_LINE_LENGTH];

    if(fgets(buffer, MAX_LINE_LENGTH, stream) == NULL) break;
    buffer[strcspn(buffer, "\n")] = 0;
    size_t l = strlen(buffer);
    int read = CountSpaces(&current_spaces, buffer);
    if(l == 0 || current_spaces == l) continue;
    if((current_spaces > indentationLevel)) {
      if(Match(&buffer[read], "Simulation Start Time")) {
        int matched = sscanf(&buffer[read], "Simulation Start Time (s) %f",
                             &pstate->start_time);
        if(matched == 1) {
          continue;
        }
        else {
          exit(1);
        }
      }
      if(Match(&buffer[read], "Simulation End Time")) {
        int matched = sscanf(&buffer[read], "Simulation End Time (s) %f",
                             &pstate->end_time);
        if(matched == 1) {
          continue;
        }
        else {
          exit(1);
        }
      }
      continue;
    }
    else {
      // if the number of preceding spaces return to that of the title
      // or even less, then we have finished parsing this section.
      return 0;
    }
  }
  return 0;
}

// return 1 if we understand the block and wish to contunue
static int ParseBlockTitle(struct OutParserState *pstate, char *buffer,
                           int indentationLevel, FILE *stream) {
  //   char version[255];
  //   int v1, v2, v3;
  if(Match(buffer, "Version")) {
    sscanf(buffer, "Version : FDS %d.%d.%d", &pstate->outData.version.major,
           &pstate->outData.version.minor,
           &pstate->outData.version.maintenance);
    return 1;
  }
  if(Match(buffer, "Run Time Diagnostics")) {
    RunTimeDiagnostics(pstate, indentationLevel, stream);
    return 1;
  }
  if(Match(buffer, "Miscellaneous Parameters")) {
    ParseMiscParameters(pstate, indentationLevel, stream);
    return 1;
  }
  return 0;
}

/* ------------------ ParseOutFile ------------------------ */

struct OutFileData *ParseOutFile(const char *file) {
  struct OutParserState *pstate = malloc(sizeof(struct OutParserState));
  memset(pstate, 0, sizeof(struct OutParserState));
  // int indentationLevel; the current indentation level we are parsing in the
  // number of spaces (TODO: govern TAB behaviour). This starts at 0 (i.e. 0
  // spaces preceding the first character of text on a line).
  int indentation_level = 0;
  // int skipping; boolean; are we attempting to parse the current block or
  // are we just ignoring it. This starts as false as we don't
  // want to ignore the single block at indentation level 0,
  // as this is the whole file
  int skipping = 0;
  FILE *stream;
  int initial_size = 120;
  pstate->timestepArrLength = initial_size;
  pstate->timesteps = malloc(initial_size * sizeof(struct Timestep));
  pstate->first_empty_timestep = pstate->timesteps;
  // realloc(buf, size)
  // open the file
  if((stream = fopen(file, "r")) == NULL) return NULL;

  // int currentSpaces; the current number of counted spaces
  int current_spaces;
  // this parsing loop deals with top level blocks
  while(!feof(stream)) {
    // TODO: read lines of arbitrary length
    char buffer[MAX_LINE_LENGTH];

    // read a line into the buffer
    pstate->currentLine++;
    if(fgets(buffer, MAX_LINE_LENGTH, stream) == NULL) break;
    // trim off the end of line
    buffer[strcspn(buffer, "\n")] = 0;
    // take the length of the resulting string
    size_t l = strlen(buffer);
    // count the number of preceding spaces on this line
    // and place it into the currentSpaces variable
    // read is the number of characters read
    int read = CountSpaces(&current_spaces, buffer);
    // if the line is empty or full of only space, slip to the next line
    if(l == 0 || current_spaces == l) continue;
    // if the current spaces is greater than the indentation level (i.e. we've
    // moved into a subblock) and we are supposed to be skipping this block,
    // then skip to the next line.
    if((current_spaces > indentation_level) && skipping == 1) {
      continue;
    }
    else {
      // otherwise attempt to parse the title of this block
      skipping =
          1 - ParseBlockTitle(pstate, &buffer[read], indentation_level, stream);
      indentation_level = current_spaces;
    }
  }
  fclose(stream);
  struct OutFileData *out = malloc(sizeof(struct OutFileData));
  memset(out, 0, sizeof(struct OutFileData));
  out->ntimesteps = pstate->ntimesteps;
  out->timesteps = pstate->timesteps;
  out->outData = pstate->outData;
  out->last_time = pstate->last_time;
  out->start_time = pstate->start_time;
  out->end_time = pstate->end_time;
  return out;
}

static int ReadFloatVal(struct OutFileData *pstate, enum OutValue command) {
  struct json_object *jobj = json_object_new_object();
  struct json_object *values = json_object_new_array();
  json_object_object_add(jobj, "values", values);
  json_object_object_add(jobj, "x_units", json_object_new_string("s"));
  json_object_object_add(jobj, "x_name",
                         json_object_new_string("Simulation Time"));
  char *name = NULL;
  char *y_units = NULL;
  char *y_name = NULL;
  switch(command) {
  case PRESSURE_ERROR:
    name = "Pressure Error";
    y_name = "Pressure Error";
    y_units = "1/s^2";
    break;
  case VELOCITY_ERROR:
    name = "Velocity Error";
    y_name = "Velocity Error";
    y_units = "m/s";
    break;
  case PRESSURE_ITERATIONS:
    name = "Pressure Iterations";
    y_name = "Pressure Iterations";
    y_units = "-";
    break;
  default:
    fprintf(stderr, "invalid command");
    json_object_put(jobj);
    return 2;
  }
  json_object_object_add(jobj, "name", json_object_new_string(name));
  json_object_object_add(jobj, "y_units", json_object_new_string(y_units));
  json_object_object_add(jobj, "y_name", json_object_new_string(y_name));
  for(int i = 0; i < pstate->ntimesteps; i++) {
    struct json_object *dv = json_object_new_object();
    char time[DATESTRING_BUFFERSIZE];
    DatetimeToString(time, pstate->timesteps[i].time);
    json_object_object_add(
        dv, "x", json_object_new_double(pstate->timesteps[i].simtime));
    switch(command) {
    case PRESSURE_ERROR:
      json_object_object_add(
          dv, "y", json_object_new_double(pstate->timesteps[i].max_pres_err));
      break;
    case VELOCITY_ERROR:
      json_object_object_add(
          dv, "y", json_object_new_double(pstate->timesteps[i].max_vel_err));
      break;
    case PRESSURE_ITERATIONS:
      json_object_object_add(
          dv, "y",
          json_object_new_double(pstate->timesteps[i].pressure_iterations));
      break;
    default:
      fprintf(stderr, "invalid command");
      json_object_put(jobj);
      return 2;
    }
    json_object_array_add(values, dv);
  }

  const char *json_output =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  printf("%s\n", json_output);
  json_object_put(jobj);
  return 1;
}

static int ReadRundata(struct OutFileData *pstate) {
  struct json_object *jobj = json_object_new_object();
  struct json_object *values = json_object_new_array();
  for(int i = 0; i < pstate->ntimesteps; i++) {
    fprintf(stderr, "%i\n", i);
    struct json_object *dv = json_object_new_object();
    // int ret = float_iter_next(&iter, &step);
    // if(ret) break;
    char time[DATESTRING_BUFFERSIZE];
    DatetimeToString(time, pstate->timesteps[i].time);
    json_object_object_add(dv, "x", json_object_new_string(time));
    json_object_object_add(
        dv, "y", json_object_new_double(pstate->timesteps[i].simtime));
    json_object_array_add(values, dv);
  }
  json_object_object_add(jobj, "values", values);
  json_object_object_add(jobj, "name", json_object_new_string("Run Data"));
  json_object_object_add(jobj, "x_units", json_object_new_string("s"));
  json_object_object_add(jobj, "x_name",
                         json_object_new_string("Simulation Time"));
  json_object_object_add(jobj, "y_units", json_object_new_string("-"));
  json_object_object_add(jobj, "y_name", json_object_new_string("Wall Time"));

  const char *json_output =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  printf("%s\n", json_output);
  json_object_put(jobj);
  return 1;
}

static int ReadProgress(struct OutFileData *pstate) {
  // time_t time_of_day;
  char start_wall[DATESTRING_BUFFERSIZE];
  strftime(start_wall, DATESTRING_BUFFERSIZE - 1, "%Y-%m-%dT%H:%M:%SZ",
           &pstate->timesteps[0].time);
  // time_of_day = mktime(&timesteps[i].time);

  // time_t time_of_day;
  char last_wall[DATESTRING_BUFFERSIZE];
  strftime(last_wall, DATESTRING_BUFFERSIZE - 1, "%Y-%m-%dT%H:%M:%SZ",
           &pstate->timesteps[pstate->ntimesteps - 1].time);
  // time_of_day = mktime(&timesteps[i].time);

  struct json_object *jobj = json_object_new_object();
  struct json_object *sim = json_object_new_object();
  json_object_object_add(sim, "StartTime",
                         json_object_new_double(pstate->start_time));
  json_object_object_add(sim, "EndTime",
                         json_object_new_double(pstate->end_time));
  json_object_object_add(sim, "LastTime",
                         json_object_new_double(pstate->last_time));
  struct json_object *wall = json_object_new_object();
  json_object_object_add(wall, "StartTime", json_object_new_string(start_wall));
  json_object_object_add(wall, "LastTime", json_object_new_string(last_wall));

  json_object_object_add(jobj, "Sim", sim);
  json_object_object_add(jobj, "Wall", wall);
  json_object_object_add(wall, "Format", json_object_new_string("0.0.3"));

  const char *json_output =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  printf("%s\n", json_output);
  json_object_put(jobj);
  return 1;
}

int PrintOutData(const char *path, enum OutValue command) {
  if(path == NULL) {
    fprintf(stderr, "file not specified\n");
    return 1;
  }
  struct OutFileData *out_file = ParseOutFile(path);
  if(out_file == NULL) {
    fprintf(stderr, "reading of file %s failed.\n", path);
    return 1;
  }
  switch(command) {
  case RUN_DATA:
    ReadRundata(out_file);
    break;
  case PROGRESS:
    ReadProgress(out_file);
    break;
  case PRESSURE_ERROR:
  case VELOCITY_ERROR:
  case PRESSURE_ITERATIONS:
    ReadFloatVal(out_file, command);
    break;
  default:
    fprintf(stderr, "not a valid parse_out command\n");
    free(out_file);
    return 1;
  }
  free(out_file);
  return 0;
}

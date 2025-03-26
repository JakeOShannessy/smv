#ifndef SMV_CSVS_H_DEFINED
#define SMV_CSVS_H_DEFINED

#include <csv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct simulation_step {
  /**
   * @brief The index of the time step, starting from 1.
   */
  size_t index;
  char wall_time[1024];
  float step_size;
  float simulation_time;
  float cpu_time;
};

struct steps_parser_state {
  struct simulation_step current_step;
  struct simulation_step *step_buffer;
  size_t step_buffer_off;
  size_t step_buffer_cap;
  size_t step_buffer_len;
  int row_index;
  int col_index;
};

#define BUF_SIZE 1024
struct step_iter {
  struct csv_parser p;
  struct steps_parser_state state;
  char buf[BUF_SIZE];
  FILE *fp;
};

struct data_value {
  float x;
  float y;
};

struct float_parser_state {
  char *x_name;
  int x_col_offset;
  char *y_name;
  int y_col_offset;

  size_t n_col_units;
  char **col_units;

  struct data_value current_step;
  struct data_value *buffer;
  size_t buffer_off;
  size_t buffer_cap;
  size_t buffer_len;
  int row_index;
  int col_index;
};

struct float_iter {
  struct csv_parser p;
  struct float_parser_state state;
  char buf[BUF_SIZE];
  FILE *fp;
};

struct data_vector {
  char *name;
  char *x_units;
  char *x_name;
  char *y_units;
  char *y_name;
  size_t nvalues;
  struct data_value *values;
};

struct step_iter steps_iter_new(const char *path);
int steps_iter_next(struct step_iter *iter, struct simulation_step *next);
int steps_iter_close(struct step_iter *iter);

struct float_iter float_iter_new(const char *path, const char *x_name,
                                 const char *y_name);
int float_iter_next(struct float_iter *iter, struct data_value *next);
int float_iter_close(struct float_iter *iter);

struct Timestep {
  int number;
  struct tm time;
  float stepsize;
  float simtime;
  float max_vel_err;
  float max_pres_err;
  int pressure_iterations;
  // everything else may or may not exist
};
// Time Step    2000   May  6, 2016  16:47:19
//       Step Size:    0.251E-01 s, Total Time:      50.25 s
//       Pressure Iterations:      1
//       Maximum Velocity Error:  0.30E-01 on Mesh   7 at (  25  17  36)
//       ---------------------------------------------------------------

typedef struct Version {
  int major;
  int minor;
  int maintenance;
} Version;

typedef struct OutData {
  Version version;
} OutData;

struct OutFileData {
  int ntimesteps;
  struct Timestep *timesteps;
  OutData outData;
  float last_time;
  float start_time;
  float end_time;
};

enum OutValue {
  RUN_DATA,
  PRESSURE_ERROR,
  VELOCITY_ERROR,
  PRESSURE_ITERATIONS,
  PROGRESS,
};

struct OutFileData *ParseOutFile(const char *file);
int PrintOutData(const char *path, enum OutValue);
#endif

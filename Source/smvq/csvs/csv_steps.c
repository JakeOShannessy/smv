
#include <csv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smv_csvs.h"

// This is called when a field is parsed
static void FieldCallback(void *s, size_t i, void *p) {
  struct steps_parser_state *state = (struct steps_parser_state *)p;
  if(state->row_index < 2) return;
  char b[1024];
  strncpy(b, s, i);
  b[i] = '\0';
  switch(state->col_index) {
  case 0:
    state->current_step.index = atoi(b);
    break;
  case 1:
    strncpy(state->current_step.wall_time, b, 1024);
    break;
  case 2:
    state->current_step.step_size = (float)atof(b);
    break;
  case 3:
    state->current_step.simulation_time = (float)atof(b);
    break;
  case 4:
    state->current_step.cpu_time = (float)atof(b);
    break;
  default:
    break;
  }
  state->col_index++;
}
// This is called when a record is parsed
static void RecordCallback(int c, void *p) {
  struct steps_parser_state *state = (struct steps_parser_state *)p;
  state->row_index++;
  state->col_index = 0;
  if((state->step_buffer_cap - state->step_buffer_len) > 1) {
    struct simulation_step *next = state->step_buffer + state->step_buffer_len;
    memcpy(next, &state->current_step, sizeof(struct simulation_step));
    state->step_buffer_len++;
  }
  else {
    if(state->step_buffer == NULL) {
      // TODO: change default start size
      state->step_buffer_cap = 1;
      state->step_buffer =
          malloc(state->step_buffer_cap * sizeof(struct simulation_step));
    }
    else {
      // Double the capacity of the buffer
      state->step_buffer_cap *= 2;
      state->step_buffer =
          realloc(state->step_buffer,
                  state->step_buffer_cap * sizeof(struct simulation_step));
    }
  }
}

struct step_iter steps_iter_new(const char *path) {
  struct step_iter iter = {0};
  iter.fp = fopen(path, "r");
  if(iter.fp == NULL) {
    fprintf(stderr, "Error: could not read file %s\n", path);
  }
  csv_init(&iter.p, 0);
  return iter;
}

// TODO: add error handling
int steps_iter_next(struct step_iter *iter, struct simulation_step *next) {
  for(;;) {
    if((iter->state.step_buffer_len - iter->state.step_buffer_off) > 0) {
      struct simulation_step *n =
          iter->state.step_buffer + iter->state.step_buffer_off;
      memcpy(next, n, sizeof(struct simulation_step));
      iter->state.step_buffer_off++;
      return 0;
    }
    else if(iter->state.step_buffer_off > 0) {
      // We've emptied the buffer and can clear it
      iter->state.step_buffer_off = 0;
      iter->state.step_buffer_len = 0;
    }
    size_t bytes_read = fread(iter->buf, 1, BUF_SIZE, iter->fp);
    if(bytes_read == 0) return 3;
    if(csv_parse(&iter->p, iter->buf, bytes_read, FieldCallback, RecordCallback,
                 (void *)&iter->state) != bytes_read) {
      fprintf(stderr, "Error: %s\n", csv_strerror(csv_error(&iter->p)));
      return 2;
    }
  }
  return 0;
}

int steps_iter_close(struct step_iter *iter) {
  csv_fini(&iter->p, FieldCallback, RecordCallback, NULL);
  csv_free(&iter->p);
  fclose(iter->fp);
  return 0;
}

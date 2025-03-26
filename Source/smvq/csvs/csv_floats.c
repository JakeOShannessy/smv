#define _DEFAULT_SOURCE
#include "smv_csvs.h"
#include <csv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
// disable warning about strdup
#pragma warning (disable:4996)
#endif

// This is called when a field is parsed
static void FieldCallback(void *s, size_t i, void *p) {
  struct float_parser_state *state = (struct float_parser_state *)p;
  char b[1024];
  strncpy(b, s, i);
  b[i] = '\0';
  if(state->row_index == 0) {
    state->col_units =
        realloc(state->col_units, (state->n_col_units + 1) * sizeof(char *));
    state->col_units[state->n_col_units] = strdup(b);
    state->n_col_units++;
    return;
  }
  else if(state->row_index == 1) {
    if(strcmp(state->x_name, b) == 0) {
      state->x_col_offset = state->col_index;
    }
    else if(strcmp(state->y_name, b) == 0) {
      state->y_col_offset = state->col_index;
    }
  }
  else {
    if(state->x_col_offset < 0) {
      fprintf(stderr, "ERROR: could not find csv vector %s\n", state->x_name);
      return;
    }
    if(state->y_col_offset < 0) {
      fprintf(stderr, "ERROR: could not find csv vector %s\n", state->y_name);
      return;
    }
    if(state->col_index == state->x_col_offset) {
      state->current_step.x = (float)atof(b);
    }
    if(state->col_index == state->y_col_offset) {
      state->current_step.y = (float)atof(b);
    }
  }
  state->col_index++;
}

// This is called when a record is parsed
static void RecordCallback(int c, void *p) {
  struct float_parser_state *state = (struct float_parser_state *)p;
  state->row_index++;
  state->col_index = 0;
  if((state->buffer_cap - state->buffer_len) > 1) {
    struct data_value *next = state->buffer + state->buffer_len;
    memcpy(next, &state->current_step, sizeof(struct data_value));
    state->buffer_len++;
  }
  else {
    if(state->buffer == NULL) {
      // TODO: change default start size
      state->buffer_cap = 1;
      state->buffer = malloc(state->buffer_cap * sizeof(struct data_value));
    }
    else {
      // Double the capacity of the buffer
      state->buffer_cap *= 2;
      state->buffer =
          realloc(state->buffer, state->buffer_cap * sizeof(struct data_value));
    }
  }
}

struct float_iter float_iter_new(const char *path, const char *x_name,
                                 const char *y_name) {
  struct float_iter iter = {
      .state = {.x_col_offset = -1, .y_col_offset = -1, 0}};
  iter.state.x_name = strdup(x_name);
  iter.state.y_name = strdup(y_name);
  iter.fp = fopen(path, "r");
  if(iter.fp == NULL) {
    fprintf(stderr, "Error: could not read file %s\n", path);
  }
  csv_init(&iter.p, 0);
  return iter;
}

// TODO: add error handling
int float_iter_next(struct float_iter *iter, struct data_value *next) {
  for(;;) {
    if((iter->state.buffer_len - iter->state.buffer_off) > 0) {
      struct data_value *n = iter->state.buffer + iter->state.buffer_off;
      memcpy(next, n, sizeof(struct data_value));
      iter->state.buffer_off++;
      return 0;
    }
    else if(iter->state.buffer_off > 0) {
      // We've emptied the buffer and can clear it
      iter->state.buffer_off = 0;
      iter->state.buffer_len = 0;
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

int float_iter_close(struct float_iter *iter) {
  csv_fini(&iter->p, FieldCallback, RecordCallback, NULL);
  csv_free(&iter->p);
  fclose(iter->fp);
  free(iter->state.x_name);
  free(iter->state.y_name);
  for(int i = 0; i < iter->state.n_col_units; i++) {
    free(iter->state.col_units[i]);
  }
  free(iter->state.col_units);
  return 0;
}

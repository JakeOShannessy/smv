#include "jsonrpc.h"

char *strdup(const char *s) {
  size_t slen = strlen(s);
  char *result = malloc(slen + 1);
  if (result == NULL) {
    return NULL;
  }
  memcpy(result, s, slen + 1);
  return result;
}

static void jrpc_procedure_destroy(struct jrpc_procedure *procedure) {
  if (procedure->name) {
    free(procedure->name);
    procedure->name = NULL;
  }
  if (procedure->data) {
    free(procedure->data);
    procedure->data = NULL;
  }
}

int jrpc_register_procedure(struct jrpc_server *server,
                            jrpc_function function_pointer, char *name,
                            void *data) {
  int i = server->procedure_count++;
  if (!server->procedures)
    server->procedures = malloc(sizeof(struct jrpc_procedure));
  else {
    struct jrpc_procedure *ptr =
        realloc(server->procedures,
                sizeof(struct jrpc_procedure) * server->procedure_count);
    if (!ptr) return -1;
    server->procedures = ptr;
  }
  if ((server->procedures[i].name = strdup(name)) == NULL) return -1;
  server->procedures[i].function = function_pointer;
  server->procedures[i].data = data;
  return 0;
}

void jrpc_server_destroy(struct jrpc_server *server) {
  for (int i = 0; i < server->procedure_count; i++) {
    jrpc_procedure_destroy(&(server->procedures[i]));
  }
  free(server->procedures);
}

int jrpc_deregister_procedure(struct jrpc_server *server, char *name) {
  /* Search the procedure to deregister */
  int i;
  int found = 0;
  if (server->procedures) {
    for (i = 0; i < server->procedure_count; i++) {
      if (found)
        server->procedures[i - 1] = server->procedures[i];
      else if (!strcmp(name, server->procedures[i].name)) {
        found = 1;
        jrpc_procedure_destroy(&(server->procedures[i]));
      }
    }
    if (found) {
      server->procedure_count--;
      if (server->procedure_count) {
        struct jrpc_procedure *ptr =
            realloc(server->procedures,
                    sizeof(struct jrpc_procedure) * server->procedure_count);
        if (!ptr) {
          perror("realloc");
          return -1;
        }
        server->procedures = ptr;
      }
      else {
        server->procedures = NULL;
      }
    }
  }
  else {
    fprintf(stderr, "server : procedure '%s' not found\n", name);
    return -1;
  }
  return 0;
}

// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

static int send_response(struct jrpc_connection *conn, const char *response) {
  if (conn->debug_level > 1) fprintf(stderr, "JSON Response:\n%s\n", response);
  // (send(conn->fd, sq, strlen(sq), 0) < 0)
  send(conn->fd, response, strlen(response), MSG_NOSIGNAL);
  send(conn->fd, "\n", 1, MSG_NOSIGNAL);
  return 0;
}

static int send_error(struct jrpc_connection *conn, int code, char *message,
                      json_object *id) {
  int return_value = 0;
  json_object *result_root = json_object_new_object();
  json_object *error_root = json_object_new_object();
  json_object_object_add(error_root, "code", json_object_new_int(code));
  json_object_object_add(error_root, "message",
                         json_object_new_string(message));
  json_object_object_add(result_root, "error", error_root);
  json_object_object_add(result_root, "id", id);
  const char *str_result =
      json_object_to_json_string_ext(result_root, JSON_C_TO_STRING_PRETTY);
  return_value = send_response(conn, str_result);
  json_object_put(result_root);
  free(message);
  return return_value;
}

static int send_result(struct jrpc_connection *conn, json_object *result,
                       json_object *id) {
  int return_value = 0;
  json_object *result_root = json_object_new_object();
  if (result) json_object_object_add(result_root, "result", result);
  json_object *jsonrpc_label = json_object_new_string("2.0");
  json_object_object_add(result_root, "jsonrpc", jsonrpc_label);
  json_object_object_add(result_root, "id", id);

  const char *str_result =
      json_object_to_json_string_ext(result_root, JSON_C_TO_STRING_PRETTY);
  return_value = send_response(conn, str_result);
  json_object_put(result_root);
  return return_value;
}

static int invoke_procedure(struct jrpc_server *server,
                            struct jrpc_connection *conn, const char *name,
                            json_object *params, json_object *id) {
  json_object *returned = NULL;
  int procedure_found = 0;
  jrpc_context ctx;
  ctx.error_code = 0;
  ctx.error_message = NULL;
  int i = server->procedure_count;
  while (i--) {
    if (!strcmp(server->procedures[i].name, name)) {
      procedure_found = 1;
      ctx.data = server->procedures[i].data;
      returned = server->procedures[i].function(&ctx, params, id);
      break;
    }
  }
  if (!procedure_found)
    return send_error(conn, JRPC_METHOD_NOT_FOUND, strdup("Method not found."),
                      id);
  else {
    if (ctx.error_code)
      return send_error(conn, ctx.error_code, ctx.error_message, id);
    else
      return send_result(conn, returned, id);
  }
}

static int eval_request(struct jrpc_server *server,
                        struct jrpc_connection *conn, json_object *jobj) {
  json_object *method_object = json_object_object_get(jobj, "method");
  if (method_object == NULL) {
    json_object_put(method_object);
    goto err;
  };
  const char *method = json_object_get_string(method_object);
  if (method == NULL) {
    json_object_put(method_object);
    goto err;
  };
  {
    json_object *params = json_object_object_get(jobj, "params");
    if (json_object_is_type(params, json_type_null) ||
        json_object_is_type(params, json_type_array) ||
        json_object_is_type(params, json_type_object)) {
      json_object *id_object = json_object_object_get(jobj, "id");
      if (json_object_is_type(id_object, json_type_null) ||
          json_object_is_type(id_object, json_type_string) ||
          json_object_is_type(id_object, json_type_int)) {
        // We have to copy ID because using it on the reply and deleting the
        // response Object will also delete ID
        json_object *id_copy = NULL;
        if (id_object != NULL)
          json_object_deep_copy(id_object, &id_copy,
                                json_c_shallow_copy_default);
        if (server->debug_level)
          fprintf(stderr, "Method Invoked: %s\n", method);
        return invoke_procedure(server, conn, method, params, id_copy);
      }
    }
  }
err:
  send_error(conn, JRPC_INVALID_REQUEST,
             strdup("The JSON sent is not a valid Request object."), NULL);
  return -1;
}

struct jrpc_server jrpc_server_create() {
  struct jrpc_server server = {0};
  server.socket.sun_family = AF_UNIX;
  server.remote.sun_family = AF_UNIX;
  server.debug_level = 1;
  return server;
}

int jrpc_server_listen(struct jrpc_server *server) {
  if ((server->fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  strcpy(server->socket.sun_path, SOCK_PATH);
  unlink(server->socket.sun_path);
  int len = strlen(server->socket.sun_path) + sizeof(server->socket.sun_family);
  if (bind(server->fd, (struct sockaddr *)&server->socket, len) == -1) {
    perror("bind");
    exit(1);
  }

  if (listen(server->fd, 5) == -1) {
    perror("listen");
    exit(1);
  }
  return 0;
}

int process_connection(struct jrpc_server *server,
                       struct jrpc_connection *conn) {

  int done = 0;
  char *extra_chars = NULL;
  json_tokener *tok = json_tokener_new();
  // This is the loop that handles JSON objects. It goes once around for each
  // object passed on the socket.
  do {
    json_object *jobj = NULL;
    int stringlen = 0;
    enum json_tokener_error jerr;
    // This is the loop for reading from the socket. It goes around until an
    // object is parsed.
    do {
      if (extra_chars) {
        fprintf(stderr, "Parsing Extra Chars: >%s<\n", extra_chars);
        jobj = json_tokener_parse_ex(tok, extra_chars, strlen(extra_chars));
        extra_chars = NULL;
      }
      else {
        memset(conn->buffer, 0, conn->buffer_size * sizeof(char));
        int n = recv(conn->fd, conn->buffer, sizeof(conn->buffer), 0);
        if (n <= 0) {
          if (n < 0) perror("recv");
          fprintf(stderr, "Connection Closed %d\n", n);
          done = 1;
        }
        stringlen = strlen(conn->buffer);
        if (stringlen == 0) break;
        jobj = json_tokener_parse_ex(tok, conn->buffer, stringlen);
      }
    } while ((jerr = json_tokener_get_error(tok)) == json_tokener_continue &&
             done == 0);
    if (jerr != json_tokener_success) {
      fprintf(stderr, "Error: %s\n", json_tokener_error_desc(jerr));
      // Handle errors, as appropriate for your application.
    }
    if (json_tokener_get_parse_end(tok) < stringlen) {
      // Handle extra characters after parsed object as desired.
      // e.g. issue an error, parse another object from that point, etc...
      extra_chars = &conn->buffer[json_tokener_get_parse_end(tok)];
    }
    if (jobj == NULL) break;
    // Success, use jobj here.
    eval_request(server, conn, jobj);
    const char *sq =
        json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);

    fprintf(stderr, "%s\n", sq);
  } while (!done);
  fprintf(stderr, "Connection done.\n");

  close(conn->fd);
  return 0;
}

struct jrpc_connection connection_create(size_t n) {
  struct jrpc_connection conn = {0};
  conn.buffer = malloc(n * sizeof(char));
  conn.buffer_size = n;
  conn.debug_level = 2;
  return conn;
}
void connection_destroy(struct jrpc_connection *conn) { free(conn->buffer); }

json_object *subtract(jrpc_context *context, json_object *params,
                      json_object *id) {
  int a = json_object_get_int(json_object_array_get_idx(params, 0));
  int b = json_object_get_int(json_object_array_get_idx(params, 1));
  json_object *result_root = json_object_new_int(a - b);
  return result_root;
}

#include "jsonrpc_internal.h"
#include <json-c/json_object.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef _WIN32
#define UNLINK _unlink
#define STRDUP _strdup
#else
#define UNLINK unlink
#define STRDUP strdup
#endif

#ifdef _WIN32
//
//  This application opens a file specified by the user and uses
//  a temporary file to convert the file to upper case letters.
//  Note that the given source file is assumed to be an ASCII text file
//  and the new file created is overwritten each time the application is
//  run.
//

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#define BUFSIZE 1024

void PrintError(LPCTSTR errDesc);

TCHAR *CreateTempPath() {
  HANDLE h_file = INVALID_HANDLE_VALUE;
  HANDLE h_temp_file = INVALID_HANDLE_VALUE;

  BOOL f_success = FALSE;
  UINT u_ret_val = 0;

  DWORD dw_bytes_read = 0;
  DWORD dw_bytes_written = 0;

  TCHAR *sz_temp_file_name = malloc(MAX_PATH * sizeof(TCHAR));
  TCHAR lp_temp_path_buffer[MAX_PATH];

  //  Gets the temp path env string (no guarantee it's a valid path).
  DWORD dw_ret_val = 0;
  dw_ret_val = GetTempPath(MAX_PATH,             // length of the buffer
                           lp_temp_path_buffer); // buffer for path
  if(dw_ret_val > MAX_PATH || (dw_ret_val == 0)) {
    PrintError(TEXT("GetTempPath failed"));
    if(!CloseHandle(h_file)) {
      PrintError(TEXT("CloseHandle(h_file) failed"));
      return NULL;
    }
    return NULL;
  }

  //  Generates a temporary file name.
  u_ret_val = GetTempFileName(lp_temp_path_buffer, // directory for tmp files
                              TEXT("smv_socket"),  // temp file name prefix
                              0,                   // create unique name
                              sz_temp_file_name);  // buffer for name
  if(u_ret_val == 0) {
    PrintError(TEXT("GetTempFileName failed"));
    if(!CloseHandle(h_file)) {
      PrintError(TEXT("CloseHandle(h_file) failed"));
      return NULL;
    }
    return NULL;
  }
  return sz_temp_file_name;
}

//  ErrorMessage support function.
//  Retrieves the system error message for the GetLastError() code.
//  Note: caller must use LocalFree() on the returned LPCTSTR buffer.
LPCTSTR ErrorMessage(DWORD error) {
  LPVOID lp_msg_buf;

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lp_msg_buf, 0, NULL);

  return ((LPCTSTR)lp_msg_buf);
}

//  PrintError support function.
//  Simple wrapper function for error output.
void PrintError(LPCTSTR errDesc) {
  LPCTSTR err_msg = ErrorMessage(GetLastError());
  _tprintf(TEXT("\n** ERROR ** %s: %s\n"), errDesc, err_msg);
  LocalFree((LPVOID)err_msg);
}
#else
char *CreateTempPath() {
  const char *template_basis = "/tmp/smv_socket.XXXXXX";
  const char *file_add = "/smv.sock";
  char *template =
      malloc((strlen(template_basis) + strlen(file_add) + 1) * sizeof(char));
  strcpy(template, template_basis);
  char *path = mkdtemp(template);
  strcat(template, file_add);
  fprintf(stderr, "template: %s\n", template);
  fprintf(stderr, "path: %s\n", path);
  if(errno != 0) {
    fprintf(stderr, "Value of errno: %d\n", errno);
    fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
    free(template);
    return NULL;
  }
  return path;
}
#endif

int sockClose(SOCKET sock) {

  int status = 0;

#ifdef _WIN32
  status = shutdown(sock, SD_SEND);
  if(status == 0) {
    status = closesocket(sock);
  }
  else {
    sock_error("shutdown");
  }
#else
  status = shutdown(sock, SHUT_WR);
  if(status == 0) {
    status = close(sock);
  }
#endif

  return status;
}

void CbInit(circular_buffer *cb, size_t capacity, size_t sz) {
  cb->buffer = malloc(capacity * sz);
  if(cb->buffer == NULL) {
    // memory allocation failure
    fprintf(stderr, "Error: memory allocation error\n");
  }
  cb->buffer_end = (char *)cb->buffer + capacity * sz;
  cb->capacity = capacity;
  cb->count = 0;
  cb->sz = sz;
  cb->head = cb->buffer;
  cb->tail = cb->buffer;
}

void CbFree(circular_buffer *cb) {
  free(cb->buffer);
  cb->buffer_end = NULL;
  cb->capacity = 0;
  cb->count = 0;
  cb->sz = 0;
  cb->head = NULL;
  cb->tail = NULL;
}

void CbPushBack(circular_buffer *cb, const void *item) {
  if(cb->count == cb->capacity) {
    // TODO: handle error
    // The buffer is at capacity
  }
  memcpy(cb->head, item, cb->sz);
  cb->head = (char *)cb->head + cb->sz;
  if(cb->head == cb->buffer_end) cb->head = cb->buffer;
  cb->count++;
}

void CbPopFront(circular_buffer *cb, void *item) {
  if(cb->count == 0) {
    // There are no items, return NULL
    item = NULL;
    return;
  }
  memcpy(item, cb->tail, cb->sz);
  cb->tail = (char *)cb->tail + cb->sz;
  if(cb->tail == cb->buffer_end) cb->tail = cb->buffer;
  cb->count--;
}

void push_rpc(struct jrpc_server *server, json_object *jobj) {
  pthread_mutex_lock(&server->rpc_mutex);
  if(server->rpc_buffer.buffer == NULL) {
    CbInit(&server->rpc_buffer, 100, sizeof(json_object *));
  }
  CbPushBack(&server->rpc_buffer, &jobj);
  fprintf(stderr, "rpc_len(socket_thread): %zu\n", server->rpc_buffer.count);
  pthread_mutex_unlock(&server->rpc_mutex);
}

static int processing_rpc_recursion = 0;
int process_rpcs(struct jrpc_server *server) {

  if(server->rpc_buffer.buffer == NULL) return 0;

  int completed_requests = 0;
  // TODO: sometimes this gets called recursviely
  if(processing_rpc_recursion == 0) pthread_mutex_lock(&server->rpc_mutex);
  processing_rpc_recursion++;
  while(server->rpc_buffer.count > 0) {
    json_object *jobj = NULL;
    CbPopFront(&server->rpc_buffer, &jobj);
    if(jobj == NULL) break;
    // Do RPC actions
    eval_request(server, server->conn, jobj);
    completed_requests++;
  }
  processing_rpc_recursion--;
  if(processing_rpc_recursion == 0) pthread_mutex_unlock(&server->rpc_mutex);
  return completed_requests;
}

DLLEXPORT void *kickoff_socket(void *kickoff_info) {
  struct kickoff_info *koi = kickoff_info;
  struct jrpc_server *s_server = koi->server;
  char *sock_path;
  if(koi->sock_path != NULL) {
    sock_path = STRDUP(koi->sock_path);
  }
  else {
    sock_path = CreateTempPath();
  }
  jrpc_server_listen(s_server, sock_path);
  free(sock_path);
  for(;;) {
    fprintf(stderr, "Waiting for a connection...\n");
    struct jrpc_connection conn = jrpc_server_connect(s_server);
    s_server->conn = &conn;
    fprintf(stderr, "Connected.\n");
    process_connection(s_server, &conn);
    fprintf(stderr, "Connection processed.");
    s_server->conn = NULL;
    connection_destroy(&conn);
  }
  return NULL;
}

static void jrpc_procedure_destroy(struct jrpc_procedure *procedure) {
  if(procedure->name) {
    free(procedure->name);
    procedure->name = NULL;
  }
  if(procedure->data) {
    free(procedure->data);
    procedure->data = NULL;
  }
}

DLLEXPORT int jrpc_register_procedure(struct jrpc_server *server,
                                      jrpc_function function_pointer,
                                      char *name, void *data) {
  int i = server->procedure_count++;
  if(!server->procedures)
    server->procedures = malloc(sizeof(struct jrpc_procedure));
  else {
    struct jrpc_procedure *ptr =
        realloc(server->procedures,
                sizeof(struct jrpc_procedure) * server->procedure_count);
    if(!ptr) return -1;
    server->procedures = ptr;
  }
  if((server->procedures[i].name = STRDUP(name)) == NULL) return -1;
  server->procedures[i].function = function_pointer;
  server->procedures[i].data = data;
  return 0;
}

DLLEXPORT void jrpc_server_destroy(struct jrpc_server *server) {
  for(int i = 0; i < server->procedure_count; i++) {
    jrpc_procedure_destroy(&(server->procedures[i]));
  }
  free(server->procedures);
  // Analogous to `unlink`
  // DeleteFileA(SERVER_SOCKET);
#ifdef _WIN32
  WSACleanup();
#endif
}

DLLEXPORT void jrpc_client_destroy(struct jrpc_client *client) {
  // Analogous to `unlink`
  // DeleteFileA(SERVER_SOCKET);
#ifdef _WIN32
  WSACleanup();
#endif
}

DLLEXPORT void jrpc_client_destroy_ptr(struct jrpc_client *client) {
  // Analogous to `unlink`
  // DeleteFileA(SERVER_SOCKET);
  free(client);
#ifdef _WIN32
  WSACleanup();
#endif
}

DLLEXPORT int jrpc_deregister_procedure(struct jrpc_server *server,
                                        char *name) {
  /* Search the procedure to deregister */
  int i;
  int found = 0;
  if(server->procedures) {
    for(i = 0; i < server->procedure_count; i++) {
      if(found)
        server->procedures[i - 1] = server->procedures[i];
      else if(!strcmp(name, server->procedures[i].name)) {
        found = 1;
        jrpc_procedure_destroy(&(server->procedures[i]));
      }
    }
    if(found) {
      server->procedure_count--;
      if(server->procedure_count) {
        struct jrpc_procedure *ptr =
            realloc(server->procedures,
                    sizeof(struct jrpc_procedure) * server->procedure_count);
        if(!ptr) {
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

static int send_response(struct jrpc_connection *conn, const char *response) {
  if(conn->debug_level > 1) fprintf(stderr, "JSON Response:\n%s\n", response);
  send(conn->fd, response, (int)strlen(response), 0);
  send(conn->fd, "\n", 1, 0);
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
  return return_value;
}

// TODO: if id is not defined, do not provide resposne
static int send_result(struct jrpc_connection *conn, json_object *result,
                       json_object *id) {
  int return_value = 0;
  json_object *result_root = json_object_new_object();
  if(result) json_object_object_add(result_root, "result", result);
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
  while(i--) {
    if(!strcmp(server->procedures[i].name, name)) {
      procedure_found = 1;
      ctx.data = server->procedures[i].data;
      returned = server->procedures[i].function(&ctx, params, id);
      break;
    }
  }
  if(!procedure_found) {
    char msg[1000];
    snprintf(msg, 1000, "Method not found (%s).", name);
    return send_error(conn, JRPC_METHOD_NOT_FOUND, msg, id);
  }
  else {
    if(ctx.error_code) {
      int status = send_error(conn, ctx.error_code, ctx.error_message, id);
      if(ctx.error_message) {
        // free(ctx.error_message);
      }
      return status;
    }
    else {
      return send_result(conn, returned, id);
    }
  }
}

static int eval_request(struct jrpc_server *server,
                        struct jrpc_connection *conn, json_object *jobj) {
  json_object *method_object = json_object_object_get(jobj, "method");
  if(method_object == NULL) {
    json_object_put(method_object);
    goto err;
  };
  const char *method = json_object_get_string(method_object);
  if(method == NULL) {
    json_object_put(method_object);
    goto err;
  };
  {
    json_object *params = json_object_object_get(jobj, "params");
    if(json_object_is_type(params, json_type_null) ||
       json_object_is_type(params, json_type_array) ||
       json_object_is_type(params, json_type_object)) {
      json_object *id_object = json_object_object_get(jobj, "id");
      if(json_object_is_type(id_object, json_type_null) ||
         json_object_is_type(id_object, json_type_string) ||
         json_object_is_type(id_object, json_type_int)) {
        // We have to copy ID because using it on the reply and deleting the
        // response Object will also delete ID
        json_object *id_copy = NULL;
        if(id_object != NULL)
          json_object_deep_copy(id_object, &id_copy,
                                json_c_shallow_copy_default);
        if(server->debug_level) fprintf(stderr, "Method Invoked: %s\n", method);
        return invoke_procedure(server, conn, method, params, id_copy);
      }
    }
  }
err:
  send_error(conn, JRPC_INVALID_REQUEST,
             "The JSON sent is not a valid Request object.", NULL);
  return -1;
}

struct jrpc_server jrpc_server_create() {
  struct jrpc_server server = {0};
  server.socket.sun_family = AF_UNIX;
  server.remote.sun_family = AF_UNIX;
  server.debug_level = 1;
  // TODO: what happens when the buffer limit is exceeded?
  CbInit(&server.rpc_buffer, 100, sizeof(json_object *));
  server.conn = NULL;
  pthread_mutex_t rpc_mutex = PTHREAD_MUTEX_INITIALIZER;
  server.rpc_mutex = rpc_mutex;
#ifdef _WIN32
  int rc = WSAStartup(MAKEWORD(2, 2), &server.wsa_data);
  if(rc != 0) {
    fprintf(stderr, "WSAStartup() error: %d\n", rc);
    exit(1);
  }
#endif
  return server;
}

int jrpc_server_listen(struct jrpc_server *server, const char *sock_path) {
  if((server->fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    sock_error("socket");
    exit(1);
  }
  fprintf(stdout, "sock_path: %s\n", sock_path);
  strcpy(server->socket.sun_path, sock_path);
  UNLINK(server->socket.sun_path);
  if(bind(server->fd, (struct sockaddr *)&server->socket,
          sizeof(struct sockaddr_un)) == -1) {
    sock_error("bind");
    exit(1);
  }

  if(listen(server->fd, 5) == -1) {
    sock_error("listen");
    exit(1);
  }
  return 0;
}

struct jrpc_connection jrpc_server_connect(struct jrpc_server *server) {
  struct jrpc_connection conn = connection_create(100);
  // TODO: consider handling multiple remotes
#ifdef _WIN32
  int slen = (int)sizeof(server->remote);
#else
  socklen_t slen = (socklen_t)sizeof(server->remote);
#endif
  if((conn.fd = accept(server->fd, (struct sockaddr *)&server->remote,
                       &slen)) == -1) {
    sock_error("accept");
    exit(1);
  }
  return conn;
}

DLLEXPORT struct jrpc_client jrpc_client_create() {
  struct jrpc_client client = {0};
  client.socket.sun_family = AF_UNIX;

  client.debug_level = 1;
  fprintf(stderr, "struct created\n");
#ifdef _WIN32
  WSADATA wsa_data = {0};
  fprintf(stderr, "pre WSAStartup\n");
  int rc = WSAStartup(MAKEWORD(2, 2), &client.wsa_data);
  fprintf(stderr, "post WSAStartup\n");
  if(rc != 0) {
    fprintf(stderr, "WSAStartup() error: %d\n", rc);
    exit(1);
  }
#endif
  fprintf(stderr, "returning client\n");
  return client;
}

DLLEXPORT struct jrpc_client *jrpc_client_create_ptr() {
  struct jrpc_client *client = malloc(sizeof(struct jrpc_client));

  *client = jrpc_client_create();
  return client;
}

DLLEXPORT void print_something() { fprintf(stderr, "something\n"); }

DLLEXPORT struct jrpc_connection jrpc_client_connect(struct jrpc_client *client,
                                                     const char *sock_path) {
  struct jrpc_connection conn = connection_create(100);
  connection_connect(client, &conn, sock_path);
  return conn;
}

DLLEXPORT struct jrpc_connection *
JrpcClientConnectPtr(struct jrpc_client *client, const char *sock_path) {
  fprintf(stderr, "connect sock path: %s\n", sock_path);
  struct jrpc_connection *conn = malloc(sizeof(struct jrpc_connection));
  *conn = jrpc_client_connect(client, sock_path);
  return conn;
}

// TODO: this can be removed but can be kept as the old symbol name was
// "jrpc_client_connect_ptr"
DLLEXPORT struct jrpc_connection *
jrpc_client_connect_ptr(struct jrpc_client *client, const char *sock_path) {
  fprintf(stderr, "connect sock path: %s\n", sock_path);
  struct jrpc_connection *conn = malloc(sizeof(struct jrpc_connection));
  *conn = jrpc_client_connect(client, sock_path);
  return conn;
}

DLLEXPORT char *PopOrBlockS(struct jrpc_connection *conn) {
  json_object *jobj = pop_or_block(conn);
  const char *sq =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  char *r = STRDUP(sq);
  json_object_put(jobj);
  return r;
}

// TODO: this can be removed but can be kept as the old symbol name was
// "pop_or_block_s"
DLLEXPORT char *pop_or_block_s(struct jrpc_connection *conn) {
  json_object *jobj = pop_or_block(conn);
  const char *sq =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  char *r = STRDUP(sq);
  json_object_put(jobj);
  return r;
}

/**
 * @brief Parse a JSON object from the front of a stream or block until a full
 * object is sent.
 *
 * @param server
 * @param conn
 * @return json_object* returns NULL if connection is closed.
 */
DLLEXPORT json_object *pop_or_block(struct jrpc_connection *conn) {

  int done = 0;
  json_tokener *tok = json_tokener_new();
  // This is the loop that handles JSON objects. It goes once around for each
  // object passed on the socket.
  json_object *jobj = NULL;
  size_t stringlen = 0;
  // TODO: we give this an initial value so that we return an error if the
  // connection sends nothing.
  enum json_tokener_error jerr;
  // This is the loop for reading from the socket. It goes around until an
  // object is parsed.
  do {
    if(conn->extra_chars) {
      jobj = json_tokener_parse_ex(tok, conn->extra_chars,
                                   (int)conn->extra_chars_n);
      conn->extra_chars = NULL;
      conn->extra_chars_n = 0;
    }
    else {
      memset(conn->buffer, 0, conn->buffer_size * sizeof(char));
      int n = recv(conn->fd, conn->buffer, sizeof(conn->buffer), 0);
      if(n <= 0) {
        if(n < 0) sock_error("recv");
        // TODO: what if we use extra_chars then receive nothing?
        fprintf(stderr, "Connection Closed %d\n", n);
        conn->extra_chars = NULL;
        conn->extra_chars_n = 0;
        return NULL;
      }
      stringlen = strlen(conn->buffer);
      char g[100];
      strncpy(g, conn->buffer, stringlen);
      g[stringlen] = '\0';
      // fprintf(stderr, ">>[%03d/%03d]: %s\n", stringlen, n, conn->buffer);
      // if stringlen is less than n, it's because there was a '\0' in the
      // string indicating we should start again.
      if(stringlen < n) {
        // parse the end into the existing object
        jobj = json_tokener_parse_ex(tok, conn->buffer, (int)stringlen);
        // TODO: if we're in a failure state we should restart
        jerr = json_tokener_get_error(tok);
        if(jerr != json_tokener_success && stringlen > 0) {
          json_tokener_reset(tok);
        }
        if(n != 1) {
          conn->extra_chars = &conn->buffer[stringlen + 1];
          conn->extra_chars_n = n - stringlen - 1;
        }
      }
      else {
        jobj = json_tokener_parse_ex(tok, conn->buffer, n);
      }
    }
  } while((jerr = json_tokener_get_error(tok)) == json_tokener_continue &&
          done == 0);
  if(jerr != json_tokener_success) {
    fprintf(stderr, "Error: %s\n", json_tokener_error_desc(jerr));
    fprintf(stderr, "Buffer: %s\n", conn->buffer);
    // Handle errors, as appropriate for your application.
  }
  if(json_tokener_get_parse_end(tok) < stringlen) {
    if(conn->extra_chars != NULL) {
      // Handle extra characters after parsed object as desired.
      // e.g. issue an error, parse another object from that point, etc...
      conn->extra_chars = &conn->buffer[json_tokener_get_parse_end(tok)];
      conn->extra_chars_n = strlen(conn->extra_chars);
    }
  }
  return jobj;
}

int process_connection(struct jrpc_server *server,
                       struct jrpc_connection *conn) {
  // This is the loop that handles JSON objects. It goes once around for each
  // object passed on the socket.
  for(;;) {
    json_object *jobj = pop_or_block(conn);
    if(jobj == NULL) break;
    // Success, add jobj to the buffer
    push_rpc(server, jobj);
  }
  fprintf(stderr, "Connection done (server).\n");

  return 0;
}

struct jrpc_connection connection_create(size_t n) {
  struct jrpc_connection conn = {0};
  conn.buffer = malloc(n * sizeof(char));
  conn.buffer_size = n;
  conn.debug_level = 0;
  return conn;
}

void connection_connect(struct jrpc_client *client,
                        struct jrpc_connection *conn, const char *sock_path) {
  if((conn->fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    sock_error("socket");
    exit(1);
  }

  strcpy(client->socket.sun_path, sock_path);
  size_t len =
      strlen(client->socket.sun_path) + sizeof(client->socket.sun_family);
  if(connect(conn->fd, (struct sockaddr *)&client->socket, (int)len) == -1) {
    sock_error("connect");
    exit(1);
  }
}

DLLEXPORT void connection_destroy(struct jrpc_connection *conn) {
  fprintf(stderr, "destroying connection\n");
  sockClose(conn->fd);
  free(conn->buffer);
}
void connection_clear(struct jrpc_connection *conn) {
  memset(conn->buffer, 0, conn->buffer_size * sizeof(char));
}

void sock_error(const char *error_category) {
#ifdef _WIN32
  wchar_t *s = NULL;
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, WSAGetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&s, 0,
                 NULL);
  fprintf(stderr, "%s: [%d]: %S\n", error_category, WSAGetLastError(), s);
#else
  perror(error_category);
#endif
}

json_object *request_create(int id, const char *method, json_object *params) {
  json_object *request_object = json_object_new_object();

  json_object_object_add(request_object, "jsonrpc",
                         json_object_new_string("2.0"));
  json_object_object_add(request_object, "id", json_object_new_int(id));
  json_object_object_add(request_object, "method",
                         json_object_new_string(method));
  json_object_object_add(request_object, "params", params);
  return request_object;
}

void send_request(struct jrpc_connection *conn, json_object *request_object) {
  const char *str =
      json_object_to_json_string_ext(request_object, JSON_C_TO_STRING_PRETTY);
  // fprintf(stderr, "sending %s\n", str);
  int r = send(conn->fd, str, (int)(strlen(str) + 1), 0);
  if(r == -1) {
    sock_error("send");
    exit(1);
  }
}

void parse_response(struct jrpc_connection *conn, json_object *request_object) {
  const char *str =
      json_object_to_json_string_ext(request_object, JSON_C_TO_STRING_PRETTY);
  if(send(conn->fd, str, (int)strlen(str) + 1, 0) == -1) {
    sock_error("send");
    exit(1);
  }
}
DLLEXPORT int jrpc_send_request(struct jrpc_connection *conn,
                                const char *method, json_object *params) {
  int request_id = conn->next_id;
  conn->next_id++;
  json_object *request_object = request_create(request_id, method, params);
  send_request(conn, request_object);
  json_object_put(request_object);
  return request_id;
}

DLLEXPORT int jrpc_send_request_s(struct jrpc_connection *conn,
                                  const char *method, const char *params_s) {
  json_tokener *tok = json_tokener_new();
  json_object *params =
      json_tokener_parse_ex(tok, params_s, (int)strlen(params_s));
  return jrpc_send_request(conn, method, params);
}

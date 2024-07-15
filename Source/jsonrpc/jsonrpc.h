#ifndef JSONRPC_H_DEFINED
#define JSONRPC_H_DEFINED

#ifdef _WIN32
// clang-format off
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <afunix.h>
// clang-format on
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>

#define DLLEXPORT __declspec(dllexport)

// SHARED

/*
 *
 * http://www.jsonrpc.org/specification
 *
 * code	message	meaning
 * -32700	Parse error	Invalid JSON was received by the server.
 * An error occurred on the server while parsing the JSON text.
 * -32600	Invalid Request	The JSON sent is not a valid Request object.
 * -32601	Method not found	The method does not exist / is not
 * available. -32602	Invalid params	Invalid method parameter(s). -32603
 * Internal error	Internal JSON-RPC error. -32000 to -32099	Server
 * error	Reserved for implementation-defined server-errors.
 */

#define JRPC_PARSE_ERROR -32700
#define JRPC_INVALID_REQUEST -32600
#define JRPC_METHOD_NOT_FOUND -32601
#define JRPC_INVALID_PARAMS -32603
#define JRPC_INTERNAL_ERROR -32693

struct jrpc_connection {
  int fd;
  unsigned int buffer_size;
  char *buffer;
  int debug_level;
  char *extra_chars;
  // Only used when acting as a client.
  int next_id;
};
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>
DLLEXPORT void connection_destroy(struct jrpc_connection *conn);
DLLEXPORT json_object *pop_or_block(struct jrpc_connection *conn);
#ifndef _WIN32
char *strdup(const char *s);
#endif

// SERVER SIDE

typedef struct {
  void *data;
  int error_code;
  char *error_message;
} jrpc_context;

typedef json_object *(*jrpc_function)(jrpc_context *context,
                                      json_object *params, json_object *id);

struct jrpc_procedure {
  char *name;
  jrpc_function function;
  void *data;
};
typedef struct circular_buffer {
  void *buffer;     // data buffer
  void *buffer_end; // end of data buffer
  size_t capacity;  // maximum number of items in the buffer
  size_t count;     // number of items in the buffer
  size_t sz;        // size of each item in the buffer
  void *head;       // pointer to head
  void *tail;       // pointer to tail
} circular_buffer;
struct jrpc_server {
#ifdef _WIN32
  WSADATA wsa_data;
#endif
  struct sockaddr_un socket;
  struct sockaddr_un remote;
  int fd;
  int procedure_count;
  struct jrpc_procedure *procedures;
  int debug_level;
  circular_buffer rpc_buffer;
  // Currently we only serve one connection at a time. For this use case there
  // isn't much use in expanding it.
  struct jrpc_connection *conn;
  pthread_mutex_t rpc_mutex;
};
struct kickoff_info {
  struct jrpc_server *server;
  char *sock_path;
};
DLLEXPORT struct jrpc_server jrpc_server_create();
DLLEXPORT struct jrpc_connection
jrpc_server_connect(struct jrpc_server *server);
DLLEXPORT int jrpc_register_procedure(struct jrpc_server *server,
                                      jrpc_function function_pointer,
                                      char *name, void *data);

DLLEXPORT void jrpc_server_destroy(struct jrpc_server *server);
DLLEXPORT int jrpc_deregister_procedure(struct jrpc_server *server, char *name);
DLLEXPORT void *kickoff_socket(void *server_in);
DLLEXPORT int process_rpcs(struct jrpc_server *server);

// CLIENT SIDE
struct jrpc_client {
#ifdef _WIN32
  WSADATA wsa_data;
#endif
  struct sockaddr_un socket;
  int debug_level;
};

DLLEXPORT void print_something();
DLLEXPORT struct jrpc_client *jrpc_client_create_ptr();
DLLEXPORT struct jrpc_client jrpc_client_create();
DLLEXPORT struct jrpc_connection jrpc_client_connect(struct jrpc_client *client,
                                                     const char *sock_path);
DLLEXPORT void jrpc_send_request(struct jrpc_connection *conn,
                                 const char *method, json_object *params);
DLLEXPORT void jrpc_send_request_s(struct jrpc_connection *conn,
                                   const char *method, const char *params_s);
DLLEXPORT void jrpc_client_destroy(struct jrpc_client *client);
DLLEXPORT void jrpc_client_destroy_ptr(struct jrpc_client *client);
#endif

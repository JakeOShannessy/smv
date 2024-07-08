#ifndef JSONRPC_H_DEFINED
#define JSONRPC_H_DEFINED

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#include "jsonrpc.h"

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>

#define SOCK_PATH "echo_socket"

typedef struct circular_buffer
{
  void *buffer;     // data buffer
  void *buffer_end; // end of data buffer
  size_t capacity;  // maximum number of items in the buffer
  size_t count;     // number of items in the buffer
  size_t sz;        // size of each item in the buffer
  void *head;       // pointer to head
  void *tail;       // pointer to tail
} circular_buffer;

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

typedef struct
{
  void *data;
  int error_code;
  char *error_message;
} jrpc_context;

typedef json_object *(*jrpc_function)(jrpc_context *context,
                                      json_object *params, json_object *id);

struct jrpc_procedure
{
  char *name;
  jrpc_function function;
  void *data;
};

struct jrpc_server
{
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

struct jrpc_connection
{
  int fd;
  unsigned int buffer_size;
  char *buffer;
  int debug_level;
};

static void jrpc_procedure_destroy(struct jrpc_procedure *procedure);

int jrpc_register_procedure(struct jrpc_server *server,
                            jrpc_function function_pointer, char *name,
                            void *data);

void jrpc_server_destroy(struct jrpc_server *server);

int jrpc_deregister_procedure(struct jrpc_server *server, char *name);
static void *get_in_addr(struct sockaddr *sa);

static int send_response(struct jrpc_connection *conn, const char *response);

static int send_error(struct jrpc_connection *conn, int code, char *message,
                      json_object *id);

static int send_result(struct jrpc_connection *conn, json_object *result,
                       json_object *id);

static int invoke_procedure(struct jrpc_server *server,
                            struct jrpc_connection *conn, const char *name,
                            json_object *params, json_object *id);

static int eval_request(struct jrpc_server *server,
                        struct jrpc_connection *conn, json_object *jobj);

struct jrpc_server jrpc_server_create();

int jrpc_server_listen(struct jrpc_server *server);

int process_connection(struct jrpc_server *server,
                       struct jrpc_connection *conn);

struct jrpc_connection connection_create(size_t n);
void connection_destroy(struct jrpc_connection *conn);

json_object *move_x(jrpc_context *context, json_object *params,
                    json_object *id);
char *strdup(const char *s);
void push_rpc(struct jrpc_server *server, json_object *jobj);
int process_rpcs(struct jrpc_server *server);
void *kickoff_socket(void *server_in);
#endif

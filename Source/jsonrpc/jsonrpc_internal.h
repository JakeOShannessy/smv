#ifndef JSONRPC_INTERNAL_H_DEFINED
#define JSONRPC_INTERNAL_H_DEFINED

#ifdef _WIN32
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <afunix.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#include "jsonrpc.h"

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>

#define SOCK_PATH "echo_socket"

#define DLLEXPORT __declspec(dllexport)

static void jrpc_procedure_destroy(struct jrpc_procedure *procedure);

int jrpc_register_procedure(struct jrpc_server *server,
                            jrpc_function function_pointer, char *name,
                            void *data);

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
struct jrpc_client jrpc_client_create();

int jrpc_server_listen(struct jrpc_server *server);

int process_connection(struct jrpc_server *server,
                       struct jrpc_connection *conn);

struct jrpc_connection connection_create(size_t n);
void connection_destroy(struct jrpc_connection *conn);
void connection_clear(struct jrpc_connection *conn);
json_object *move_x(jrpc_context *context, json_object *params,
                    json_object *id);
#ifndef _WIN32
char *strdup(const char *s);
#endif
void push_rpc(struct jrpc_server *server, json_object *jobj);
int sockClose(int sock);
json_object *pop_or_block(struct jrpc_connection *conn);
void sock_error(const char *error_category);
json_object *request_create(int id, const char *method, json_object *params);
void send_request(struct jrpc_connection *conn, json_object *request_object);
void parse_response(struct jrpc_connection *conn, json_object *request_object);
void connection_connect(struct jrpc_client *client, struct jrpc_connection *conn);
#endif

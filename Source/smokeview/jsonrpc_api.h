#ifndef JSONRPC_API_H_DEFINED
#define JSONRPC_API_H_DEFINED
#include "jsonrpc.h"

#include "c_api.h"
#include <json-c/json_object.h>

int register_procedures(struct jrpc_server *server);

#endif

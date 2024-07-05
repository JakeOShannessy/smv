#include "jsonrpc.h"

int main(void) {
  struct jrpc_server server = jrpc_server_create();
  jrpc_server_listen(&server);
  jrpc_register_procedure(&server, &subtract, "subtract", NULL);
  struct jrpc_connection conn = connection_create(100);
  int n = 0;
  for (;;) {
    fprintf(stderr, "Waiting for a connection...\n");
    socklen_t slen = sizeof(server.remote);
    if ((conn.fd = accept(server.fd, (struct sockaddr *)&server.remote,
                          &slen)) == -1) {
      perror("accept");
      exit(1);
    }
    fprintf(stderr, "Connected.\n");
    process_connection(&server, &conn);
    fprintf(stderr, "Connection processed. %d\n", n);
    n++;
  }
  connection_destroy(&conn);
  return 0;
}

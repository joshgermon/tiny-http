#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CONN 5
#define MAX_INCOMING_LEN 1024
#define PORT "3490"

int main() {
  // Server socket
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct addrinfo *addrnode;
  int server_socket;
  int yes = 1;

  // Incoming socket
  struct sockaddr_storage incoming_addr;
  socklen_t addr_size = sizeof incoming_addr;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int status;
  if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 1;
  }

  for (addrnode = servinfo; addrnode != NULL; addrnode = addrnode->ai_next) {
    server_socket = socket(addrnode->ai_family, addrnode->ai_socktype,
                           addrnode->ai_protocol);
    if (server_socket == -1) {
      perror("server: socket failed to create");
      continue;
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
            sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(server_socket, addrnode->ai_addr, addrnode->ai_addrlen) != 0) {
      close(server_socket);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (addrnode == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(server_socket, MAX_CONN) != 0) {
    perror("listen");
    exit(1);
  }

  printf("Server is listening for connections on port %s\n", PORT);

  for (;;) {
    printf("Awaiting new connection...\n");

    int conn_socket =
        accept(server_socket, (struct sockaddr *)&incoming_addr, &addr_size);
    if (conn_socket == -1) {
      perror("accept");
      continue;
    }

    printf("Received new incoming connection.\n");

    char buffer[MAX_INCOMING_LEN];
    recv(conn_socket, buffer, MAX_INCOMING_LEN, 0);
    printf("Received: %s", buffer);

    const char *http_res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World\r\n";
    int http_res_len = strlen(http_res);
    int bytes_sent = send(conn_socket, http_res, http_res_len, 0);
    if (bytes_sent <= 0) {
      printf("Failed to sent bytes\n");
      exit(1);
    }
    printf("Sent response back\n");
    close(conn_socket);
  }

  close(server_socket);
  freeaddrinfo(addrnode);
  return 0;
}

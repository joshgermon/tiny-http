#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CONN 5
#define MAX_INCOMING_LEN 1024
#define PORT "3490"

#define BAD_REQUEST_ERRNO 1
#define SERVER_FAILURE_ERRNO 2

struct http_request {
  char *method; // GET, POST etc;
  char *resource_path; // /index.html
  char *version; // HTTP/1.1
  hash_table *headers; // { 'Content-Type': 'application/json' }
  char *body;
  int body_length;
};


int parse_http_request(const char *request, int request_len) {
  // HTTP Method First word to whitespace
  // Path = Next to white space
  // HTTP Version = Next to whitespace
  for (int i = 0; i < request_len; i++) {
    if(request[i] == '\n' && request[i-1] == '\r') {
      // First encounter is HTTP status line
      // i.e GET /index.html HTTP/1.1
      // That means request[0..i] is status line
      //
      // Then next encounter is first header,
      // Could start another loop from [i+1]
      // exit when encountered before we read through the body
      //
      // Body == end of headers to end of data
    }
  }

  // -1 return = 4XX Bad request
  // -2 return = 5XX server error
  return 0;
}

int send_http_response(int fd) {
  const char *http_res =
      "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World\r\n";
  int http_res_len = strlen(http_res);
  int bytes_sent = send(fd, http_res, http_res_len, 0);
  if (bytes_sent <= 0) {
    printf("Failed to sent bytes\n");
    exit(1);
  }
  printf("Sent response back\n");
  return 0;
}

int handle_incoming_req(int fd) {
  char buffer[MAX_INCOMING_LEN];
  recv(fd, buffer, MAX_INCOMING_LEN, 0);

  printf("Received new incoming connection.\n");
  printf("Request:\r\n %s\n", buffer);

  // Parse HTTP status line, headers and boy
  send_http_response(fd);

  return 0;
}

int bind_new_http_server() {
  // Server socket
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct addrinfo *addrnode;
  int server_socket;
  int yes = 1;

  // Need to make sure struct is clear ** REVIEW
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

  // Returns socket
  return server_socket;
}

int listen_and_serve(int server_socket) {
  // Incoming socket
  struct sockaddr_storage incoming_addr;
  socklen_t addr_size = sizeof incoming_addr;

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

    handle_incoming_req(conn_socket);

    close(conn_socket);
  }

  close(server_socket);
  return 0;
}

int main() {
  int server_socket = bind_new_http_server();

  listen_and_serve(server_socket);

  return 0;
}

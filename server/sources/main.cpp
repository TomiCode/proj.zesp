#include "main.h"
#include "client.h"
#include "messages.h"

std::vector<Client*> server_clients;

void client_messages_handler(struct msg_header *header, Client *client)
{
  printf("Message got from client, type: %hhu, size: %u.\n", header->type, header->size);
}

int main(int argc, char** argv)
{
  int srv_fd;
  if (!(srv_fd = socket(AF_INET, SOCK_STREAM, 0))) {
    perror("socket");
    exit(EXIT_FAIL);
  }

  int sock_opt;
  if (setsockopt(srv_fd, SOL_SOCKET, SO_REUSEPORT, &sock_opt, sizeof(sock_opt))) {
    perror("setsockopt");
    exit(EXIT_FAIL);
  }

  struct sockaddr_in address;
  int addrlen = sizeof(address);
  address.sin_port = htons(LISTEN_PORT);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(srv_fd, (struct sockaddr*)&address, sizeof(address))) {
    perror("bind");
    exit(EXIT_FAIL);
  }

  if (listen(srv_fd, LISTEN_MAX_CONN)) {
    perror("listen");
    exit(EXIT_FAIL);
  }
  printf("Hello, server is waiting for connections on :%d.\n", LISTEN_PORT);

  struct msg_header handshake_msg = { msg_type::server_handshake };
  int accept_socket;
  while(1) {
    if ((accept_socket = accept(srv_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))) {
      Client *client_ptr = new Client(1024, accept_socket);
      server_clients.push_back(client_ptr->handshake(&handshake_msg));
    }
  }

  return 0;
}

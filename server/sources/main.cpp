#include "main.h"
#include "client.h"
#include "messages.h"
#include "database.h"

Database users_db("users.wtf");
std::vector<Client*> server_clients;

void client_message(struct msg_header *header, Client *client)
{
  printf("Message got from client, type: %hhu, size: %u.\n", header->type, header->size);
  switch(header->type) {
    case msg_type::auth_login:
      {
        struct msg_login *login = (struct msg_login*)header;
        printf("Processing login for user %s.\n", login->username);

        struct msg_auth_response response = {{msg_type::auth_response}};
        response.header.size = sizeof(struct msg_auth_response) - sizeof(struct msg_header);

        if (users_db.authorize(login->username, login->password)) {
          client->setLogin(login->username);
          response.status = auth_status::logged_in;
        }
        else
          response.status = auth_status::invalid;
        client->send(&response);
      }
      break;
    default:
      printf("Message got from client, type: %hhu, size: %u.\n", header->type, header->size);
  }
}

void client_disconnect(Client *client)
{

}

int main(int argc, char** argv)
{
  int srv_fd;
  if (!(srv_fd = socket(AF_INET, SOCK_STREAM, 0))) {
    perror("socket");
    exit(EXIT_FAIL);
  }

  int sock_opt;
  setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));
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

  users_db.check();
  printf("Hello, server is waiting for connections on :%d.\n", LISTEN_PORT);

  struct msg_server_handshake handshake = {{msg_type::server_handshake, 0}, "Hello on the server! Please login or register."};
  handshake.header.size = sizeof(struct msg_server_handshake) - sizeof(struct msg_header);
  int accept_socket;

  while(1) {
    if ((accept_socket = accept(srv_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))) {
      Client *client_ptr = new Client(1024, accept_socket);
      server_clients.push_back(client_ptr->handshake(&handshake));
    }
  }

  return 0;
}

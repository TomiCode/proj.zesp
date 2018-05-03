#include "main.h"
#include "client.h"
#include "messages.h"
#include "database.h"

Database users_db("users.wtf");
std::vector<Client*> server_clients;

// Called when a client receives a complete message
void event_on_client_message(Client* sender, struct msg_header* header)
{
  printf("Message got from client, type: %hhu, size: %u.\n", header->type, header->size);
  switch(header->type) {
    case msg_type::auth_login:
      {
        struct msg_auth_request *login = (struct msg_auth_request*)header;
        printf("Processing login for user %s.\n", login->username);

        struct msg_auth_response response = {{msg_type::auth_response}};
        response.header.size = sizeof(struct msg_auth_response) - sizeof(struct msg_header);

        if (users_db.authorize(login->username, login->password)) {
          sender->set_login(login->username);
          response.status = auth_status::logged_in;
        }
        else
          response.status = auth_status::invalid;
        sender->send(&response);
      }
      break;
    case msg_type::auth_register:
      {
        struct msg_auth_request *auth_register = (struct msg_auth_request*)header;
        printf("Register new user %s, but first checking if exists..\n", auth_register->username);

        struct msg_auth_response response = {{msg_type::auth_response}};
        response.header.size = sizeof(struct msg_auth_response) - sizeof(struct msg_header);
        if (users_db.exists(auth_register->username)) {
          response.status = auth_status::exists;
        }
        else {
          if (!users_db.create(auth_register->username, auth_register->password)) {
            printf("Error occured while user %s creation.\n", auth_register->username);
            response.status = auth_status::error;
          }
          else
            response.status = auth_status::created;
        }
        sender->send(&response);
      }
      break;
    case msg_type::global_message:
      {
        struct msg_global_message *msg = (struct msg_global_message*)header;
        printf("Global message send by %s.\n", sender->name());

        struct msg_global_message response = {{msg_type::global_message}};
        response.header.size = sizeof(struct msg_global_message) - sizeof(struct msg_header);

        snprintf(response.message, 256, "%s: %s", sender->name(), msg->message);
        for (auto &cli : server_clients) {
          if (cli->is_active()) cli->send(&response);
        }
      }
      break;
    default:
      printf("This message was not handled by the server.\n");
  }
}

// Called when a client disconnects from the server
void event_on_client_disconnect(Client *sender)
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

  int accept_socket;
  struct msg_server_handshake handshake = {{msg_type::server_handshake, 0}, "Hello on the server! Please login or register."};
  handshake.header.size = sizeof(struct msg_server_handshake) - sizeof(struct msg_header);

  while(true) {
    if ((accept_socket = accept(srv_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))) {
      Client *client = new Client(1024, accept_socket);
      server_clients.push_back(client->init());
      client->send(&handshake);
    }
  }

  return 0;
}

#include <iostream>
#include <memory>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "messages.h"
#include "client.h"
#include "database.h"
#include "channel.h"

#define EXIT_FAIL -1
#define LISTEN_MAX_CONN 16
#define LISTEN_PORT 1337

Database users_db("users.wtf");

std::vector<Client*> server_clients;
std::vector<Channel*> server_channels;

// Called when a client receives a complete message
void event_on_client_message(Client *sender, msg_header *header)
{
  printf("Message got from client, type: %hhu, size: %u.\n", header->type, header->size);
  switch(header->type) {
    case msg_type::auth_login:
      {
        auto *login = (msg_auth_request *)header;
        printf("Processing login for user %s.\n", login->username);

        msg_auth_response response = {{msg_type::auth_response}};
        response.header.size = sizeof(msg_auth_response) - sizeof(msg_header);

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
        auto *auth_register = (msg_auth_request *)header;
        printf("Register new user %s, but first checking if exists..\n", auth_register->username);

        msg_auth_response response = {{msg_type::auth_response}};
        response.header.size = sizeof(msg_auth_response) - sizeof(msg_header);
        if (users_db.exists(auth_register->username)) {
          response.status = auth_status::exists;
        }
        else {
          if (!users_db.create(auth_register->username, auth_register->password)) {
            printf("Error occurred while user %s creation.\n", auth_register->username);
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
        auto *msg = (msg_global_message *)header;
        printf("Global message send by %s.\n", sender->name());

        msg_global_message response = {{msg_type::global_message}};
        response.header.size = sizeof(msg_global_message) - sizeof(msg_header);

        snprintf(response.message, sizeof(response.message), "<%s> %s", sender->name(), msg->message);
        for (auto &cli : server_clients) {
          if (cli->is_active())
            cli->send(&response);
        }
      }
      break;
    case msg_type::private_message:
      {
        auto *msg = (msg_private_message *)header;
        printf("Private message from %s to %s.\n", sender->name(), msg->address);

        msg_private_message response = {{msg_type::private_message}};
        response.header.size = sizeof(msg_private_message) - sizeof(msg_header);

        snprintf(response.content, sizeof(response.content), "%s: %s", sender->name(), msg->content);
        strcpy(response.address, msg->address);
        sender->send(&response);

        strcpy(response.address, sender->name());
        for (auto &client : server_clients) {
          if (strcmp(client->name(), msg->address) == 0) {
            client->send(&response);
            break;
          }
        }
      }
      break;
      case msg_type::channel_request:
        {
          auto *msg = (msg_channel *)header;
          printf("Client %s requested channel %s.\n", sender->name(), msg->name);

          for (auto &channel : server_channels) {
            if (strcmp(channel->name(), msg->name) == 0) {
              if (msg->subscribe)
                channel->join(sender);
              else
                channel->leave(sender);
              return;
            }
          }
          if (msg->subscribe) {
            auto *add_channel = new Channel(msg->name);
            server_channels.push_back(add_channel);
            add_channel->join(sender);
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
  auto it = server_clients.begin();
  while(it != server_clients.end()) {
    if (*it == sender) {
      it = server_clients.erase(it);
      printf("Removed client from vector.");
      // delete(sender);
      break;
    }
    else it++;
  }
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

  if (bind(srv_fd, (struct sockaddr *)&address, sizeof(address))) {
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
  msg_server_handshake handshake = {{msg_type::server_handshake, 0}, "Hello on the server! Please login or register."};
  handshake.header.size = sizeof(msg_server_handshake) - sizeof(msg_header);

  while(true) {
    if ((accept_socket = accept(srv_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen))) {
      Client *client = new Client(1024, accept_socket);
      server_clients.push_back(client->init());
      client->send(&handshake);
    }
  }

  return 0;
}

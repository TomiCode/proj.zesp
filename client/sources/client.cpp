#include "client.h"
#include "ui.h"
#include "messages.h"
#include "receiver.h"

Client::Client()
  : state(client_state_t::invalid)
{
  this->ui = new Ui(this);
  this->receiver = new Receiver(this);
}

Client::~Client()
{
  delete this->ui;
  delete this->receiver;
}

uint32_t Client::hash(const char *str)
{
  uint32_t base = 0x1925F;
  for(; *str != '\0'; str++)
    base = (base ^ *str) << 4;
  return base;
}

bool Client::parseParams(char *args, const char *fmt, ...)
{
  if (args == NULL) {
    this->ui->write("[exec] Command expected %d parameter(s).\n", strlen(fmt));
    return false;
  }

  char *lastBuffer = args;
  va_list params;

  va_start(params, fmt);
  for (; *fmt != '\0'; fmt++) {
    if (*lastBuffer == '\0') break;

    for (; *args != '\0'; args++) {
      if (*args == ' ') {
        *(args++) = '\0';
        break;
      }
    }

    if (*fmt == 's') {
      char **param = va_arg(params, char**);
      *param = lastBuffer;
    }
    else if (*fmt == 'i') {
      int *param = va_arg(params, int*);
      *param = ::atoi(lastBuffer);
    }
    lastBuffer = args;
  }
  va_end(params);

  if (*fmt != '\0') {
    this->ui->write("[exec] To few command parameters.\n");
    return false;
  }
  return true;
}

void Client::handleMessage(struct msg_header *header)
{
  switch(header->type) {
    case msg_type::server_handshake:
      {
        struct msg_server_handshake *handshake = (struct msg_server_handshake*)header;
        this->ui->write("[motd] %s\n", handshake->description);
      }
      break;
    case msg_type::server_message:
      {
        struct msg_server *message = (struct msg_server*)header;
        this->ui->write("[server] %s\n", message->content);
      }
      break;
    case msg_type::auth_response:
      {
        struct msg_auth_response *response = (struct msg_auth_response*)header;
        if (response->status == auth_status::invalid) {
          this->ui->write("[login] Invalid password or user does not exists.\n");
        }
        else if (response->status == auth_status::logged_in) {
          this->state = client_state_t::logged;
          this->ui->write("[login] Successfully logged in.\n");
        }
      }
      break;
    default:
      this->ui->write("[?] message received, type: %04x.\n", header->type);
  }
}

bool Client::init(void)
{
  if (!this->ui->init())
    return false;
  if (!(this->socket = ::socket(AF_INET, SOCK_STREAM, 0)))
    return false;

  this->ui->register_command("connect", &Client::cmd_connect);
  this->ui->register_command("register", &Client::cmd_register);
  this->ui->register_command("login", &Client::cmd_login);

  this->state = client_state_t::notconnected;
  this->ui->write("Hello to super epic chat client v1.3.3.7\n");
  
  return true;
}

void Client::send(struct msg_header *header)
{
  ::send(this->socket, header, sizeof(struct msg_header) + header->size, 0);
}

bool Client::process_msg(char *str)
{
  return true;
}

bool Client::run(void)
{
  this->ui->process();
  return true;
}

bool Client::guard_state(const char *prefix, client_state_t validState)
{
  if (this->state == validState)
    return false;

  switch(this->state) {
    case client_state_t::notconnected:
      this->ui->write("[%s] You are not connected.\n", prefix);
      break;
    case client_state_t::connected:
      this->ui->write("[%s] You are connected to server!\n", prefix);
      break;
    case client_state_t::logged:
      this->ui->write("[%s] You are logged in.\n", prefix);
      break;
    default:
      this->ui->write("[%s] You can not execute the command.\n", prefix);
  }
  return true;
}

void Client::cmd_connect(char *args)
{
  char *ip_addr; int port;
  if (this->guard_state("connect", client_state_t::notconnected))
      return;
  if (!this->parseParams(args, "si", &ip_addr, &port))
    return;

  struct sockaddr_in address = {0};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);

  if (!inet_pton(AF_INET, ip_addr, &address.sin_addr)) {
    this->ui->write("connect: invalid or unsupported address.\n");
    return;
  }

  if (connect(this->socket, (struct sockaddr*)&address, sizeof(address))) {
    this->ui->write("connect: can not connect to server at :%d.\n", port);
    return;
  }
  this->receiver->start(this->socket);
  this->state = client_state_t::connected;
}

void Client::cmd_register(char *args)
{
  if (this->guard_state("register", client_state_t::connected))
    return;
}

void Client::cmd_login(char *args)
{
  if (this->guard_state("login", client_state_t::connected))
    return;

  char *username, *passwd;
  if (!this->parseParams(args, "ss", &username, &passwd)) return;

  struct msg_login login = {{msg_type::auth_login}};
  login.header.size = sizeof(msg_login) - sizeof(msg_header);
  strcpy(login.username, username);
  strcpy(login.password, passwd);

  this->send((struct msg_header*)&login);
}

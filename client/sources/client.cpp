#include "client.h"
#include "ui.h"

Client::Client()
  : state(client_state_t::invalid)
{
  this->ui = new Ui(this);
}

Client::~Client()
{
  delete this->ui;
}

uint32_t Client::hash(const char *str)
{
  uint32_t base = 0x1925F;
  for(int i = 0; i < strlen(str); i++) {
    base = (base ^ *(str + i)) << 4;
  }
  return base;
}

bool Client::parseParams(char *args, const char *fmt, ...)
{
  if (args == NULL) return false;

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

  if (*fmt != '\0') return false;
  return true;
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

  return true;
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

void Client::cmd_connect(char *args)
{
  char *ip_addr; int port;
  if (!this->parseParams(args, "si", &ip_addr, &port)) {
    this->ui->write("usage: connect <ip address> <port>\n");
    return;
  }

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
  this->state = client_state_t::connected;
}

void Client::cmd_register(char *args)
{

}

void Client::cmd_login(char *args)
{

}

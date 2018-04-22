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

bool Client::init(void)
{
  if (!this->ui->init())
    return false;
  if (!(this->socket = ::socket(AF_INET, SOCK_STREAM, 0)))
    return false;

  this->ui->register_command("connect", &Client::cmd_connect);

  return true;
}

uint32_t Client::hash(const char *str)
{
  uint32_t base = 0x1925F;
  for(int i = 0; i < strlen(str); i++) {
    base = (base ^ *(str + i)) << 4;
  }
  return base;
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

bool Client::parse_params(char *ptr, const char *fmt, ...)
{
  if (ptr == NULL) return false;

  char *last_buffer = ptr;
  va_list params;

  va_start(params, fmt);
  for (; *fmt != '\0'; fmt++) {
    if (*last_buffer == '\0') break;

    for (; *ptr != '\0'; ptr++) {
      if (*ptr == ' ') {
        *(ptr++) = '\0';
        break;
      }
    }

    if (*fmt == 's') {
      char **param = va_arg(params, char**);
      *param = last_buffer;
    }
    else if (*fmt == 'i') {
      int *param = va_arg(params, int*);
      *param = ::atoi(last_buffer);
    }
    last_buffer = ptr;
  }
  va_end(params);

  if (*fmt != '\0') return false;
  return true;
}

void Client::cmd_connect(char *params)
{
  char *ip_addr;
  int port;
  if (!this->parse_params(params, "si", &ip_addr, &port)) {
    this->ui->write("not enought params.\n");
  }
  this->ui->write("parsed ip_addr: %s, port: %d.\n", ip_addr, port);
  return;

  if (params == NULL) {
    this->ui->write("usage: connect <ip address>\n");
    return;
  }

  struct sockaddr_in address = {0};
  address.sin_family = AF_INET;
  address.sin_port = htons(CONNECT_PORT);

  if (!inet_pton(AF_INET, ip_addr, &address.sin_addr)) {
    this->ui->write("connect: invalid or unsupported address.\n");
    return;
  }

  if (connect(this->socket, (struct sockaddr*)&address, sizeof(address))) {
    this->ui->write("connect: can not connect to remote server.\n");
    return;
  }
}

void Client::cmd_register(char *params)
{

}

void Client::cmd_login(char *params)
{

}

#include "client.h"
#include "ui.h"

Client::Client()
  : state(client_state_t::invalid)
{
  this->ui_manager = new Ui(this);
}

Client::~Client()
{
  delete this->ui_manager;
}

bool Client::init(void)
{
  if (!this->ui_manager->init())
    return false;
  if (!(this->socket = ::socket(AF_INET, SOCK_STREAM, 0)))
    return false;

  this->ui_manager->register_command("connect", &Client::cmd_connect);

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
  this->ui_manager->process();
  return true;
}

void Client::cmd_connect(char *params)
{
  if (params == NULL) {
    this->ui_manager->write("usage: connect <ip address>\n");
    return;
  }

  struct sockaddr_in address = {0};
  address.sin_family = AF_INET;
  address.sin_port = htons(CONNECT_PORT);

  if (!inet_pton(AF_INET, params, &address.sin_addr)) {
    this->ui_manager->write("connect: invalid or unsupported address.\n");
    return;
  }

  if (connect(this->socket, (struct sockaddr*)&address, sizeof(address))) {
    this->ui_manager->write("connect: can not connect to remote server.\n");
    return;
  }
}

#include <arpa/inet.h>
#include <sys/socket.h>

#include "ui.h"
#include "receiver.h"
#include "messages.h"
#include "client.h"

Client::Client(void)
  : m_ui(this),
    m_receiver(this),
    m_state(client_state_t::invalid)
{ }

uint32_t Client::hash(const char *str)
{
  uint32_t base = 0x1925F;
  for(; *str != '\0'; str++)
    base = (base ^ *str) << 4;
  return base;
}

bool Client::parse_params(char *args, const char *fmt, ...)
{
  if (args == NULL) {
    m_ui.write("[exec] Command expected %d parameter(s).\n", strlen(fmt));
    return false;
  }

  char *lastBuffer = args;
  va_list params;

  va_start(params, fmt);
  for (; *fmt != '\0'; fmt++) {
    if (*lastBuffer == '\0')
      break;

    if (*fmt == 't') {
      fmt++;
      char **param = va_arg(params, char**);
      *param = args;
      break;
    }

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
    m_ui.write("[exec] To few command parameters.\n");
    return false;
  }
  return true;
}

void Client::handle_message(struct msg_header *header)
{
  switch(header->type) {
    case msg_type::server_handshake:
      {
        struct msg_server_handshake *handshake = (struct msg_server_handshake*)header;
        m_ui.write("[notice] %s\n", handshake->description);
      }
      break;
    case msg_type::server_message:
      {
        msg_server *message = (msg_server *)header;
        m_ui.write("[server] %s\n", message->content);
      }
      break;
    case msg_type::auth_response:
      {
        msg_auth_response *response = (msg_auth_response *)header;
        switch(response->status) {
          case auth_status::invalid:
            m_ui.write("[auth] Invalid password or user does not exists.\n");
            break;
          case auth_status::logged_in:
            m_state = client_state_t::logged;
            m_ui.write("[auth] Successfully logged in.\n");
            break;
          case auth_status::exists:
            m_ui.write("[auth] This username already exists.\n");
            break;
          case auth_status::created:
            m_ui.write("[auth] Your account has been created.\n");
            break;
          case auth_status::error:
            m_ui.write("[auth] An error occured while authorization. Please try again.\n");
            break;
        }
      }
      break;
    case msg_type::global_message:
      {
        auto *message = (msg_global_message *)header;
        m_ui.write("# %s\n", message->message);
      }
      break;
    case msg_type::private_message:
      {
        auto *message = (msg_private_message *)header;
        m_ui.write("[%s] %s\n", message->address, message->content);
      }
      break;
    default:
      m_ui.write("[?] message received, type: %04x.\n", header->type);
  }
}

bool Client::init(void)
{
  if (!m_ui.init())
    return false;
  if (!(m_socket = ::socket(AF_INET, SOCK_STREAM, 0)))
    return false;

  m_ui.register_command("connect", &Client::cmd_connect);
  m_ui.register_command("register", &Client::cmd_register);
  m_ui.register_command("login", &Client::cmd_login);
  m_ui.register_command("msg", &Client::cmd_message);
  m_ui.register_command("join", &Client::cmd_join);
  m_ui.register_command("leave", &Client::cmd_leave);

  m_state = client_state_t::notconnected;
  m_ui.write("Hello to super epic chat client v1.3.3.7\n");

  return true;
}

void Client::send(void *msg)
{
  msg_header *header = (msg_header *)msg;
  ::send(m_socket, header, sizeof(msg_header) + header->size, 0);
}

void Client::handle_input(const char *str)
{
  if (guard_state("send", client_state_t::logged))
    return;

  msg_global_message message = {{msg_type::global_message}};
  message.header.size = sizeof(msg_global_message) - sizeof(msg_header);
  strncpy(message.message, str, sizeof(message.message));
  send(&message);
}

bool Client::run(void)
{
  m_ui.process();
  return true;
}

bool Client::guard_state(const char *prefix, client_state_t validState)
{
  if (m_state == validState)
    return false;

  switch(m_state) {
    case client_state_t::notconnected:
      m_ui.write("[%s] You are not connected.\n", prefix);
      break;
    case client_state_t::connected:
      m_ui.write("[%s] You are not logged in!\n", prefix);
      break;
    case client_state_t::logged:
      m_ui.write("[%s] You are logged in.\n", prefix);
      break;
    default:
      m_ui.write("[%s] You can not execute this command.\n", prefix);
  }
  return true;
}

void Client::cmd_connect(char *args)
{
  char *ip_addr; int port;
  if (guard_state("connect", client_state_t::notconnected))
      return;
  if (!parse_params(args, "si", &ip_addr, &port))
    return;

  struct sockaddr_in address = {0};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);

  if (!inet_pton(AF_INET, ip_addr, &address.sin_addr)) {
    m_ui.write("connect: invalid or unsupported address.\n");
    return;
  }

  if (connect(m_socket, (struct sockaddr*)&address, sizeof(address))) {
    m_ui.write("connect: can not connect to server at :%d.\n", port);
    return;
  }
  m_receiver.start(m_socket);
  m_state = client_state_t::connected;
}

void Client::cmd_register(char *args)
{
  char *username, *password;
  if (guard_state("register", client_state_t::connected))
    return;
  if (!parse_params(args, "ss", &username, &password))
    return;

  msg_auth_request auth_register = {{msg_type::auth_register}};
  auth_register.header.size = sizeof(msg_auth_request) - sizeof(msg_header);
  strcpy(auth_register.username, username);
  strcpy(auth_register.password, password);

  send(&auth_register);
}

void Client::cmd_login(char *args)
{
  char *username, *passwd;
  if (guard_state("login", client_state_t::connected))
    return;
  if (!parse_params(args, "ss", &username, &passwd))
    return;

  msg_auth_request login = {{msg_type::auth_login}};
  login.header.size = sizeof(msg_auth_request) - sizeof(msg_header);
  strcpy(login.username, username);
  strcpy(login.password, passwd);

  send(&login);
}

void Client::cmd_message(char *args)
{
  char *username, *msg;
  if (guard_state("msg", client_state_t::logged))
    return;
  if (!parse_params(args, "st", &username, &msg))
    return;

  msg_private_message message = {{msg_type::private_message}};
  message.header.size = sizeof(msg_private_message) - sizeof(msg_header);

  strcpy(message.address, username);
  strcpy(message.content, msg);

  send(&message);
}

void Client::cmd_join(char *args)
{
  char *channel;
  if (guard_state("join", client_state_t::logged))
    return;
  if (!parse_params(args, "s", &channel))
    return;

  if (*channel != '#') {
    m_ui.write("[join] Invalid channel name. Channels must be prefixed with '#'.");
    return;
  }

  msg_channel message = {{msg_type::channel_request}};
  message.header.size = sizeof(msg_channel) - sizeof(msg_header);

  strcpy(message.name, channel);
  message.subscribe = true;

  send(&message);
}

void Client::cmd_leave(char *args)
{
  char *channel;
  if (guard_state("join", client_state_t::logged))
    return;
  if (!parse_params(args, "s", &channel))
    return;

  if (*channel != '#') {
    m_ui.write("[join] Invalid channel name. Channels must be prefixed with '#'.");
    return;
  }

  msg_channel message = {{msg_type::channel_request}};
  message.header.size = sizeof(msg_channel) - sizeof(msg_header);

  strcpy(message.name, channel);
  message.subscribe = false;

  send(&message);
}
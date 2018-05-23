#include <unistd.h>
#include <sys/socket.h>

#include "messages.h"
#include "client.h"

Client::Client(uint32_t b_size, int32_t socket)
  : m_valid(true),
    m_logged(false),
    m_buffer_pos(0),
    m_socket(socket)
{
  printf("Hello new client at socket %d.\n", m_socket);
  m_buffer = (uint8_t *)malloc(sizeof(uint8_t) * b_size);
  m_buffer_size = b_size;
}

Client::~Client(void)
{
  printf("Destroying client at socket %d.\n", m_socket);
  if (m_socket)
    close(m_socket);

  if (m_buffer)
    free(m_buffer);
}

Client* Client::init(void)
{
  m_thread = std::thread(&Client::receive_thread, this);
  return this;
}

bool Client::send(void *msg)
{
  msg_header *header = (msg_header *)msg;
  return ::send(m_socket, header, sizeof(msg_header) + header->size, 0) != -1;
}

void Client::set_login(const char *username)
{
  strncpy(m_username, username, sizeof(m_username));
  m_logged = true;
}

const char* Client::name(void)
{
  return m_username;
}

bool Client::is_active(void)
{
  return (m_valid && m_logged);
}

void Client::receive_thread(void)
{
  msg_header *buffer_message = (msg_header *)m_buffer;
  const uint32_t recv_size = (uint32_t)(m_buffer_size / 2);
  uint32_t result = 0;

  printf("Client %d thread started.\n", m_socket);

  while(m_valid) {
    result = recv(m_socket, m_buffer + m_buffer_pos, recv_size, 0);
    if (result == 0) {
      printf("Client socket %d disconnected.\n", m_socket);
      m_valid = false;
      break;
    }
    else if (result == -1) {
      printf("Error occurred while receiving data in client %d.\n", m_socket);
      continue;
    }

    m_buffer_pos += result;
    if (m_buffer_pos < buffer_message->size)
      continue; // We don't have enough data to handle.

    // Trigger the server event function, that the client received a complete message
    event_on_client_message(this, buffer_message);

    // Content that left in the buffer.
    result = m_buffer_pos - (sizeof(struct msg_header) + buffer_message->size);

    // Move the remaining content to the buffer beginning.
    memmove(m_buffer, m_buffer + m_buffer_pos, result);
    m_buffer_pos = result;
  }
  printf("Client %d receive thread stopped.\n", m_socket);

  // Trigger client disconnected server event function
  event_on_client_disconnect(this);
}

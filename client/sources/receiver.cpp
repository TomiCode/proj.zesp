#include <sys/socket.h>
#include <string.h>

#include "client.h"
#include "messages.h"
#include "receiver.h"

Receiver::Receiver(Client *owner, uint32_t size)
  : m_owner(owner),
    m_socket(0),
    m_running(false),
    m_buffer_pos(0),
    m_buffer_size(size)
{
  m_buffer = (char *)malloc(sizeof(char) * size);
}

Receiver::~Receiver()
{
  if (m_running)
    m_running = false;
  if (m_buffer)
    free(m_buffer);
}

void Receiver::receive_thread(void)
{
  msg_header *header = (msg_header *)m_buffer;
  const uint32_t partial_size = (uint32_t)(m_buffer_size / 2);
  uint32_t result = 0;

  while(m_running) {
    result = recv(m_socket, m_buffer + m_buffer_pos, partial_size, 0);
    if (result == 0) {
      m_running = false;
      break;
    }
    else if (result == -1)
      continue;

    m_buffer_pos += result;
    if (header->size > m_buffer_pos)
      continue;

    // This message is complete and the client should take care of
    m_owner->handle_message(header);

    // Move the unused memory to the beginning of the buffer
    result = m_buffer_pos - (sizeof(msg_header) + header->size);
    memmove(m_buffer, m_buffer + m_buffer_pos, result);
    m_buffer_pos = result;
  }
}

void Receiver::start(int socket)
{
  m_socket = socket;
  m_thread = std::thread(&Receiver::receive_thread, this);
  m_running = true;
}

bool Receiver::is_running(void)
{
  return m_running;
}

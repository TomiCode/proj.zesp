#include "receiver.h"
#include "messages.h"
#include "client.h"

Receiver::Receiver(Client *owner, uint32_t size)
  : owner(owner),
    socket(0),
    running(false),
    buffer_pos(0),
    buffer_size(size)
{
  this->buffer = (char*)malloc(sizeof(char) * size);
}

Receiver::~Receiver()
{
  if (this->running)
    this->running = false;
  if (this->buffer)
    free(this->buffer);
}

void Receiver::receive_thread(void)
{
  struct msg_header *header = (struct msg_header*)this->buffer;
  const uint32_t partial_size = (uint32_t)(this->buffer_size / 2);
  uint32_t result = 0;

  while(this->running) {
    result = recv(this->socket, this->buffer + this->buffer_pos, partial_size, 0);
    if (result == 0) {
      this->running = false;
      break;
    }
    else if (result == -1)
      continue;

    this->buffer_pos += result;
    if (header->size > this->buffer_pos)
      continue;

    // This message is complete and the client should take care of
    this->owner->handle_message(header);

    // Move the unused memory to the beginning of the buffer
    result = this->buffer_pos - (sizeof(struct msg_header) + header->size);
    memmove(this->buffer, this->buffer + this->buffer_pos, result);
    this->buffer_pos = result;
  }
}

void Receiver::start(int socket)
{
  this->socket = socket;
  this->local_thread = std::thread(&Receiver::receive_thread, this);
  this->running = true;
}

bool Receiver::is_running(void)
{
  return this->running;
}

#include "receiver.h"
#include "messages.h"
#include "client.h"

Receiver::Receiver(Client *parent, uint32_t size)
  : owner(parent),
    socket(0),
    running(false),
    bufferPos(0),
    bufferSize(size)
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

void Receiver::threadHandle(void)
{
  struct msg_header *header = (struct msg_header*)this->buffer;
  const uint32_t partialSize = (uint32_t)(this->bufferSize / 2);
  uint32_t result = 0;

  while(this->running) {
    result = recv(this->socket, this->buffer + this->bufferPos, partialSize, 0);
    if (result == 0) {
      this->running = false;
      break;
    }
    else if (result == -1)
      continue;

    this->bufferPos += result;
    if (header->size > this->bufferPos)
      continue;

    this->owner->handleMessage(header);

    result = this->bufferPos - (sizeof(struct msg_header) + header->size);
    memmove(this->buffer, this->buffer + this->bufferPos, result);
    this->bufferPos = result;
  }
}

void Receiver::start(int socket)
{
  this->socket = socket;
  this->localThread = std::thread(&Receiver::threadHandle, this);
  this->running = true;
}

bool Receiver::isRunning(void)
{
  return this->running;
}

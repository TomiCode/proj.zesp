#include "client.h"

Client::Client(uint32_t b_size, int32_t socket)
  : valid(true),
    buffer_position(0),
    socket(socket)
    
{
  printf("Hello, this is a new client -> socket %d.\n", socket);
  this->buffer_ptr = (uint8_t*)malloc(sizeof(uint8_t) * b_size);
  this->buffer_size = b_size;
}

Client::~Client(void)
{
  if (this->buffer_ptr)
    free(this->buffer_ptr);

  if (this->socket)
    close(this->socket);
}

Client* Client::begin(void)
{
  this->_local_thread = std::thread(&Client::_recv_thread, this);
  return this;
}

void Client::_recv_thread(void)
{
  struct msg_header *buffer_message = (struct msg_header*)this->buffer_ptr; 
  const uint32_t recv_size = (uint32_t)(this->buffer_size / 2);
  uint32_t result = 0;

  printf("Client %d thread started.\n", this->socket);

  while(this->valid) {
    result = recv(this->socket, this->buffer_ptr + this->buffer_position, recv_size, 0);
    if (result == 0) {
      printf("Client %d probably disconected.", this->socket);
      break;
    }
    else if (result == -1) {
      printf("Error occured at recv in client %d.\n", this->socket);
      continue;
    }

    this->buffer_position += result;
    if (buffer_message->size > this->buffer_position)
      continue; // We don't have enough data to handle.

    // Handle received message.
    client_messages_handler(buffer_message, this);
    
    // Content that left in the buffer.
    result = this->buffer_position - (sizeof(struct msg_header) + buffer_message->size);
    
    // Move the remaining content to the buffer beginning.
    memmove(this->buffer_ptr, this->buffer_ptr + this->buffer_position, result);
    this->buffer_position = result;
  }
}

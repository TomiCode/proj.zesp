#include "client.h"

Client::Client(uint32_t b_size, int32_t socket)
  : valid(true),
    logged(false),
    buffer_pos(0),
    socket(socket)
{
  printf("Hello new client at socket %d.\n", socket);
  this->buffer = (uint8_t*)malloc(sizeof(uint8_t) * b_size);
  this->buffer_size = b_size;
}

Client::~Client(void)
{
  if (this->buffer)
    free(this->buffer);
  if (this->socket)
    close(this->socket);
}

Client* Client::init(void)
{
  this->local_thread = std::thread(&Client::receive_thread, this);
  return this;
}

bool Client::send(void *msg)
{
  struct msg_header *header = (struct msg_header*)msg;
  return ::send(this->socket, header, sizeof(struct msg_header) + header->size, 0) != -1;
}

void Client::set_login(const char *username)
{
  strcpy(this->username, username);
  this->logged = true;
}

const char* Client::name(void)
{
  return this->username;
}

bool Client::is_active(void)
{
  return (this->valid && this->logged);
}

void Client::receive_thread(void)
{
  struct msg_header *buffer_message = (struct msg_header*)this->buffer; 
  const uint32_t recv_size = (uint32_t)(this->buffer_size / 2);
  uint32_t result = 0;

  printf("Client %d thread started.\n", this->socket);

  while(this->valid) {
    result = recv(this->socket, this->buffer + this->buffer_pos, recv_size, 0);
    if (result == 0) {
      printf("Client socket %d disconnected.\n", this->socket);
      this->valid = false;
      break;
    }
    else if (result == -1) {
      printf("Error occurred while receiving data in client %d.\n", this->socket);
      continue;
    }

    this->buffer_pos += result;
    if (buffer_message->size > this->buffer_pos)
      continue; // We don't have enough data to handle.

    // Trigger the server event function, that the client received a complete message
    event_on_client_message(this, buffer_message);
    
    // Content that left in the buffer.
    result = this->buffer_pos - (sizeof(struct msg_header) + buffer_message->size);
    
    // Move the remaining content to the buffer beginning.
    memmove(this->buffer, this->buffer + this->buffer_pos, result);
    this->buffer_pos = result;
  }
  printf("Client %d receive thread stopped.\n", this->socket);

  // Trigger client disconnected server event function
  event_on_client_disconnect(this);
}
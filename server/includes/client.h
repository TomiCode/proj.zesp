#ifndef CLIENT_H
#define CLIENT_H

#include "main.h"
#include "messages.h"

class Client {
private:
  bool valid;
  bool logged;

  uint8_t *buffer_ptr;
  uint32_t buffer_size;
  uint32_t buffer_position;

  int32_t socket;
  char username[64];

  std::thread _local_thread;
  void _recv_thread(void);

public:
  Client(uint32_t b_size, int32_t socket);
  ~Client(void);

  Client* handshake(struct msg_server_handshake *handshake);
  bool send(void *ptr_header);
  void setLogin(const char *username);

  const char *authName(void);
  bool active(void);
};

#endif // CLIENT_H

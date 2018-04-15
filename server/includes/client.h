#ifndef CLIENT_H
#define CLIENT_H

#include "main.h"
#include "messages.h"

class Client {
private:
  bool valid;
  uint8_t *buffer_ptr;
  uint32_t buffer_size;
  uint32_t buffer_position;

  int32_t socket;
  std::thread _local_thread;
protected:
  void _recv_thread(void);
public:
  Client(uint32_t b_size, int32_t socket);
  ~Client(void);

  Client* handshake(struct msg_header *handshake);
  bool send(void *ptr, size_t size);
};

#endif /* CLIENT_H */

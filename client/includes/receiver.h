#ifndef RECEIVER_H
#define RECEIVER_H

#include "main.h"

class Receiver {
public:
  Receiver(Client *owner, uint32_t size = 1024);
  ~Receiver();

  // Start the listening thread
  void start(int socket);

  // Check if the receiver is still running
  bool is_running(void);
private:
  Client *owner;

  int socket;
  bool running;

  char *buffer;
  uint32_t buffer_pos;
  uint32_t buffer_size;

  std::thread local_thread;

  void receive_thread(void);
};

#endif // RECEIVER_H

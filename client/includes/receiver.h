#ifndef RECEIVER_H
#define RECEIVER_H

#include <iostream>
#include <thread>
#include <stdint.h>

class Client;

class Receiver {
public:
  Receiver(Client *owner, uint32_t size = 1024);
  ~Receiver();

  // Start the listening thread
  void start(int socket);

  // Check if the receiver is still running
  bool is_running(void);
private:
  Client *m_owner;

  int m_socket;
  bool m_running;

  char *m_buffer;
  uint32_t m_buffer_pos;
  uint32_t m_buffer_size;

  std::thread m_thread;

  void receive_thread(void);
};

#endif // RECEIVER_H

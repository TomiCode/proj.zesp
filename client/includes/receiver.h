#ifndef RECEIVER_H
#define RECEIVER_H

#include "main.h"

class Receiver {
private:
  Client *owner;

  int socket;
  bool running;

  char *buffer;
  uint32_t bufferPos;
  uint32_t bufferSize;

  std::thread localThread;

  void threadHandle(void);
public:
  Receiver(Client *parent, uint32_t size = 1024);
  ~Receiver();

  void start(int socket);
  bool isRunning(void);
};

#endif // RECEIVER_H

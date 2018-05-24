#ifndef CHANNEL_H
#define CHANNEL_H

#include <iostream>
#include <vector>

#include "client.h"

class Channel {
private:
  char m_name[64];
  std::vector<Client *> m_subscribers;
public:
  Channel(const char *name);

  void list(Client *sender);
  void join(Client *sender);
  void leave(Client *sender);

  void send(void *msg);

  const char* name();
};

#endif // CHANNEL_H
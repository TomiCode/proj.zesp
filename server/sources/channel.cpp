#include <iostream>
#include <algorithm>
#include <string.h>

#include "messages.h"
#include "channel.h"

Channel::Channel(const char *name)
{
  strncpy(m_name, name, sizeof(m_name));
}

void Channel::list(Client *sender)
{
}

void Channel::join(Client *sender)
{
  m_subscribers.push_back(sender);

  msg_private_message message = {{msg_type::private_message}};
  message.header.size = sizeof(msg_private_message) - sizeof(msg_header);

  strcpy(message.address, m_name);
  snprintf(message.content, sizeof(message.content), "User %s joined the channel.", sender->name());

  send(&message);
}

void Channel::leave(Client *sender)
{
  msg_private_message message = {{msg_type::private_message}};
  message.header.size = sizeof(msg_private_message) - sizeof(msg_header);

  strcpy(message.address, m_name);
  snprintf(message.content, sizeof(message.content), "User %s has left the channel.", sender->name());
  send(&message);

  m_subscribers.erase(
    std::remove(m_subscribers.begin(), m_subscribers.end(), sender), m_subscribers.end()
  );
}

void Channel::send(void *msg)
{
  for(auto &client : m_subscribers) {
    client->send(msg);
  }
}

const char* Channel::name()
{
  return m_name;
}
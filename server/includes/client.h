#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <thread>

#include <stdio.h>
#include <inttypes.h>

class Client {
public:
  Client(uint32_t b_size, int32_t socket);
  ~Client(void);

  // Start the clients listening thread
  Client * init(void);

  // Send a message to the client
  bool send(void *msg);

  // Assign to the user a nickname (after a succeeded login)
  void set_login(const char *username);

  // Get the nickname assigned to the user
  const char * name(void);

  // Check if the client is valid and the thread waits for messages
  bool is_active(void);
private:
  bool m_valid;
  bool m_logged;

  uint8_t *m_buffer;
  uint32_t m_buffer_pos;
  uint32_t m_buffer_size;

  int32_t m_socket;
  char m_username[64];

  std::thread m_thread;
  void receive_thread(void);
};

// Server events prototypes
extern void event_on_client_message(Client* sender, struct msg_header* header);
extern void event_on_client_disconnect(Client* sender);

#endif // CLIENT_H

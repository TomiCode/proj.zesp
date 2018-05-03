#ifndef CLIENT_H
#define CLIENT_H

#include "main.h"
#include "messages.h"

class Client {
public:
  Client(uint32_t b_size, int32_t socket);
  ~Client(void);

  // Start the clients listening thread
  Client* init(void);

  // Send a message to the client
  bool send(void* msg);

  // Assign to the user a nickname (after a succeeded login)
  void set_login(const char* username);

  // Get the nickname assigned to the user
  const char* name(void);

  // Check if the client is valid and the thread waits for messages
  bool is_active(void);
private:
  bool valid;
  bool logged;

  uint8_t *buffer;
  uint32_t buffer_pos;
  uint32_t buffer_size;

  int32_t socket;
  char username[64];

  std::thread local_thread;
  void receive_thread(void);
};

#endif // CLIENT_H

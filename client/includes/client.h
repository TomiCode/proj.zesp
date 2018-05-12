#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <stdint.h>

#include "ui.h"
#include "receiver.h"

// Describes the client state for the client instance
enum class client_state_t : uint8_t {
  invalid = 0,
  notconnected = 1,
  connected = 2,
  logged = 3,
  error = 4
};

class Client {
public:
  Client(void);

  // Register all commands and initialize the socket
  bool init(void);

  // Send a message to the server
  void send(void* msg);

  // Main thread for client work (ui tasks related)
  bool run(void);

  // Process input data, that is not a command
  void handle_input(const char *str);

  // Parse parameters from a command request
  bool parse_params(char *args, const char *fmt, ...);

  // Handle received message according to the message type
  void handle_message(struct msg_header *header);

  // Calculate a hash value based on the input string (NUL-terminated)
  static uint32_t hash(const char *str);
private:
  Ui m_ui;
  Receiver m_receiver;

  client_state_t m_state;
  int m_socket;

  // Guard for command execution states
  bool guard_state(const char *perfix, client_state_t validState);

  // Client command methods
  void cmd_connect(char *args);
  void cmd_register(char *args);
  void cmd_login(char *args);
};

#endif // CLIENT_H

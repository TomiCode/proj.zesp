#ifndef MESSAGES_H
#define MESSAGES_H

#include <inttypes.h>

enum class msg_type : uint8_t {
  invalid = 0,
  server_handshake = 1,
  server_message = 2,
  auth_login = 3,
  auth_register = 4,
  auth_response = 5,
  global_message = 6,
  private_message = 7,
  private_response = 8
};

enum class auth_status : uint8_t {
  invalid = 0,
  logged_in = 1,
  exists = 2,
  created = 3,
  error = 4
};

struct msg_header {
  msg_type type;
  uint32_t size;
} __attribute__((packed));

struct msg_server_handshake {
  struct msg_header header;
  char description[64];
} __attribute__((packed));

struct msg_server {
  struct msg_header header;
  char content[128];
} __attribute__((packed));

struct msg_auth_request {
  struct msg_header header;
  char username[64];
  char password[64];
} __attribute__((packed));

struct msg_auth_response {
  struct msg_header header;
  auth_status status;
} __attribute__((packed));

struct msg_global_message {
  struct msg_header header;
  char message[256];
} __attribute__((packed));

struct msg_private_message {
  struct msg_header header;
  char address[64];
  char content[256];
} __attribute__((packed));

#endif /* MESSAGES_H */

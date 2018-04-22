#ifndef MESSAGES_H
#define MESSAGES_H

#include <inttypes.h>

enum class msg_type : uint8_t {
  invalid = 0,
  server_handshake = 1,
  server_message = 2,
  auth_login = 3,
  auth_register = 4,
  auth_response = 5
};

enum class auth_status : uint8_t {
  invalid = 0,
  logged_in = 1,
  no_account = 2
};

struct msg_header {
  msg_type type;
  uint32_t size;
} __attribute__((packed));

struct msg_server_handshake {
  struct msg_header header;
  char description[64];
};

struct msg_server {
  struct msg_header header;
  char content[128];
};

struct msg_login {
  struct msg_header header;
  char username[64];
  char password[64];
} __attribute__((packed));

struct msg_auth_response {
  struct msg_header header;
  auth_status status;
} __attribute__((packed));


#endif /* MESSAGES_H */

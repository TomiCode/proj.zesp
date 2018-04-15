#ifndef MESSAGES_H
#define MESSAGES_H

#include <inttypes.h>

enum class msg_type : uint8_t {
  invalid = 0,
  server_handshake = 1,
  auth_request = 2,
  auth_response = 3
  
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

struct msg_auth_request {
  struct msg_header header;
  char nick[64];
  char password[64];
} __attribute__((packed));

struct msg_auth_response {
  struct msg_header header;
  auth_status status;
} __attribute__((packed));


#endif /* MESSAGES_H */

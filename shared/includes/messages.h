#ifndef MESSAGES_H
#define MESSAGES_H

#include <inttypes.h>

#define MESSAGE_BODY(type, header) ((type*)((&header.size) + sizeof(uint32_t)))

struct msg_header {
  uint8_t type;
  uint32_t size;
} __attribute__((packed));

struct msg_login {
  char login[64];
  char password[64];
} __attribute__((packed));


#endif /* MESSAGES_H */

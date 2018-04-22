#ifndef CLIENT_H
#define CLIENT_H

#include "main.h"

enum class client_state_t : uint8_t {
  invalid = 0,
  notconnected = 1,
  connected = 2,
  disconnected = 3,
  error = 4
};

class Client {
private:
  Ui *ui;
  Receiver *receiver;
  client_state_t state;

  int socket;

  void cmd_connect(char *args);
  void cmd_register(char *args);
  void cmd_login(char *args);
public:
  Client();
  ~Client();

  static uint32_t hash(const char *str);
  static bool parseParams(char *ptr, const char *fmt, ...);

  void handleMessage(struct msg_header *header);

  bool init(void);
  
  bool process_msg(char *str);
  bool run(void);
  void send(struct msg_header *header);


};

#endif // CLIENT_H

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
  Ui *ui_manager;
  client_state_t state;

  int socket;
public:
  Client();
  ~Client();

  bool init(void);
  
  uint32_t hash(const char *str);
  bool process_msg(char *str);

  bool run(void);

  void cmd_connect(char * params);
};

#endif // CLIENT_H

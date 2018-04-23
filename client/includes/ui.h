#ifndef UI_H
#define UI_H

#include "main.h"

typedef struct {
  char *begin;
  char *input;
  char *params;
} input_buffers_t;

enum class ui_mode_t : uint8_t {
  invalid = 0,
  command = 1,
  params = 2,
  message = 3
};

typedef void (Client::*command_fn_t)(char *args);

class Ui {
private:
  // Our global client manager
  Client *parent;

  // Curses windowses
  WINDOW *root_wnd;
  WINDOW *cmd_wnd;
  WINDOW *messages_wnd;

  // Current input mode
  ui_mode_t cmd_mode;

  // Buffers
  input_buffers_t buffers;
  
  // Commands map 
  std::unordered_map<uint32_t, command_fn_t> commands;

  // Process collected data
  void process_buffers(void);
  void reset_buffers(void);
public:
  Ui(Client *owner);
  ~Ui();

  bool init(void);
  void process(void);
  void write(const char *fmt, ...);
  void register_command(const char *, command_fn_t);
};

#endif // UI_H

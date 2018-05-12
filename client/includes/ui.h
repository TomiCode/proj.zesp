#ifndef UI_H
#define UI_H

#include <iostream>
#include <unordered_map>

#include <stdint.h>
#include <curses.h>

class Client;

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
  // Client instance
  Client *m_owner;

  // Curses windowses
  WINDOW *m_root_wnd;
  WINDOW *m_cmd_wnd;
  WINDOW *m_messages_wnd;

  // Current input mode
  ui_mode_t m_ui_mode;

  // Buffers
  input_buffers_t m_buffers;

  // Commands map
  std::unordered_map<uint32_t, command_fn_t> m_commands;

  // Process collected data
  void process_buffers(void);
  void reset_buffers(void);
public:
  Ui(Client *owner);
  ~Ui();

  // Initialize terminal screen and ncurses window instances
  bool init(void);

  // Process character input from a user
  void process(void);

  // Write something into the message window
  void write(const char *fmt, ...);

  // Register a command listener
  void register_command(const char *, command_fn_t);
};

#endif // UI_H

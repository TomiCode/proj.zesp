#include "client.h"
#include "ui.h"

Ui::Ui(Client *owner)
  : m_owner(owner),
    m_root_wnd(NULL),
    m_cmd_wnd(NULL),
    m_messages_wnd(NULL),
    m_ui_mode(ui_mode_t::invalid)
{
  m_buffers.begin = (char*)malloc(sizeof(char) * 512);
  reset_buffers();
}

Ui::~Ui()
{
  if (m_messages_wnd) delwin(m_messages_wnd);
  if (m_cmd_wnd) delwin(m_cmd_wnd);
  if (m_root_wnd) delwin(m_root_wnd);
  endwin();

  if (m_buffers.begin) free(m_buffers.begin);
  m_buffers.begin = NULL;
}

bool Ui::init(void)
{
  setlocale(LC_ALL, "");
  if ((m_root_wnd = initscr()) == NULL) {
    return false;
  }
  noecho();
  keypad(m_root_wnd, true);
  refresh();

  int maxy, maxx;
  getmaxyx(m_root_wnd, maxy, maxx);

  if ((m_messages_wnd = newwin(maxy - 1, maxx, 0, 0)) == NULL) {
    return false;
  }
  scrollok(m_messages_wnd, true);
  wrefresh(m_messages_wnd);

  if ((m_cmd_wnd = newwin(1, maxx, maxy - 1, 0)) == NULL) {
    return false;
  }
  wrefresh(m_cmd_wnd);
  return true;
}

void Ui::process(void)
{
  int _char = wgetch(m_cmd_wnd);
  switch(_char) {
    case '\n':
      werase(m_cmd_wnd);
      wrefresh(m_cmd_wnd);
      *(m_buffers.input) = '\0';
      process_buffers();
      m_ui_mode = ui_mode_t::invalid;
      reset_buffers();
      break;
    case 0x7F: {
        int y, x;
        getyx(m_cmd_wnd, y, x);
        if (x > 0)
          mvwdelch(m_cmd_wnd, y, x - 1);
        if (x <= 1 && m_ui_mode != ui_mode_t::invalid)
          m_ui_mode = ui_mode_t::invalid;
      }
      if (m_buffers.input > m_buffers.begin)
        m_buffers.input--;
      break;
    case ' ':
      if (m_ui_mode == ui_mode_t::command) {
        *(m_buffers.input++) = '\0';
        m_buffers.params = m_buffers.input;
        m_ui_mode = ui_mode_t::params;
      }
      else if (m_ui_mode == ui_mode_t::params || m_ui_mode == ui_mode_t::message)
        *(m_buffers.input++) = _char;
      break;
    case ':':
      if (m_ui_mode == ui_mode_t::invalid) {
        m_ui_mode = ui_mode_t::command;
        break;
      }
    default:
      if (m_ui_mode == ui_mode_t::invalid)
        m_ui_mode = ui_mode_t::message;
      *(m_buffers.input++) = _char;
  }
  if (_char != 0x7F)
    waddch(m_cmd_wnd, _char | (m_ui_mode == ui_mode_t::command ? A_BOLD : 0));
}

void Ui::process_buffers(void)
{
  if (m_ui_mode == ui_mode_t::command || m_ui_mode == ui_mode_t::params) {
    auto command_it = m_commands.find(m_owner->hash(m_buffers.begin));
    if (command_it == m_commands.end())
      write("[exec] Command %s does not exists.\n", m_buffers.begin);
    else
      (m_owner->*command_it->second)(m_buffers.params);
  }
  else // That is not a command..
    m_owner->handle_input(m_buffers.begin);
}

void Ui::reset_buffers(void)
{
  m_buffers.input = m_buffers.begin;
  m_buffers.params = NULL;
}

void Ui::write(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vwprintw(m_messages_wnd, fmt, args);
  wrefresh(m_messages_wnd);
  va_end(args);
  wrefresh(m_cmd_wnd);
}

void Ui::register_command(const char *cmd, command_fn_t fn)
{
  m_commands.insert({Client::hash(cmd), fn});
}

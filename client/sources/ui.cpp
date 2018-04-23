#include "ui.h"
#include "client.h"

Ui::Ui(Client *owner)
  : parent(owner),
    root_wnd(NULL),
    cmd_wnd(NULL),
    messages_wnd(NULL),
    cmd_mode(ui_mode_t::invalid)
{
  this->buffers.begin = (char*)malloc(sizeof(char) * 512);
  this->reset_buffers();
}

Ui::~Ui()
{
  if (this->messages_wnd) delwin(this->messages_wnd);
  if (this->cmd_wnd) delwin(this->cmd_wnd);
  if (this->root_wnd) delwin(this->root_wnd);
  endwin();

  if (this->buffers.begin) free(this->buffers.begin);
  this->buffers.begin = NULL;
}

bool Ui::init(void)
{
  setlocale(LC_ALL, "");
  if ((this->root_wnd = initscr()) == NULL) {
    return false;
  }
  noecho();
  keypad(this->root_wnd, true);
  refresh();

  int maxy, maxx;
  getmaxyx(this->root_wnd, maxy, maxx);

  if ((this->messages_wnd = newwin(maxy - 1, maxx, 0, 0)) == NULL) {
    return false;
  }
  scrollok(this->messages_wnd, true);
  wrefresh(this->messages_wnd);

  if ((this->cmd_wnd = newwin(1, maxx, maxy - 1, 0)) == NULL) {
    return false;
  }
  wrefresh(this->cmd_wnd);
  return true;
}

void Ui::process(void)
{
  int _char = wgetch(this->cmd_wnd);
  switch(_char) {
    case '\n':
      werase(this->cmd_wnd);
      wrefresh(this->cmd_wnd);
      *(this->buffers.input) = '\0';
      this->process_buffers();
      this->cmd_mode = ui_mode_t::invalid;
      this->reset_buffers();
      break;
    case 0x7F: {
        int y, x;
        getyx(this->cmd_wnd, y, x);
        if (x > 0)
          mvwdelch(this->cmd_wnd, y, x - 1);
        if (x <= 1 && this->cmd_mode != ui_mode_t::invalid)
          this->cmd_mode = ui_mode_t::invalid;
      }
      if (this->buffers.input > this->buffers.begin)
        this->buffers.input--;
      break;
    case ' ':
      if (this->cmd_mode == ui_mode_t::command) {
        *(this->buffers.input++) = '\0';
        this->buffers.params = this->buffers.input;
        this->cmd_mode = ui_mode_t::params;
      }
      else if (this->cmd_mode == ui_mode_t::params || this->cmd_mode == ui_mode_t::message)
        *(this->buffers.input++) = _char;
      break;
    case ':':
      if (this->cmd_mode == ui_mode_t::invalid) {
        this->cmd_mode = ui_mode_t::command;
        break;
      }
    default:
      if (this->cmd_mode == ui_mode_t::invalid)
        this->cmd_mode = ui_mode_t::message;
      *(this->buffers.input++) = _char;
  }
  if (_char != 0x7F)
    waddch(this->cmd_wnd, _char | (this->cmd_mode == ui_mode_t::command ? A_BOLD : 0));
}

void Ui::process_buffers(void)
{
  if (this->cmd_mode == ui_mode_t::command || this->cmd_mode == ui_mode_t::params) {
    auto command_it = this->commands.find(this->parent->hash(this->buffers.begin));
    if (command_it == this->commands.end())
      this->write("[exec] Command %s does not exists.\n", this->buffers.begin);
    else
      (this->parent->*command_it->second)(this->buffers.params);
  }
  else
    this->parent->process_msg(this->buffers.begin);
}

void Ui::reset_buffers(void)
{
  this->buffers.input = this->buffers.begin;
  this->buffers.params = NULL;
}

void Ui::write(const char *fmt, ...)
{
  va_list args;                                                              
  va_start(args, fmt); 
  vwprintw(this->messages_wnd, fmt, args);
  wrefresh(this->messages_wnd);
  va_end(args);
  wrefresh(this->cmd_wnd);
}

void Ui::register_command(const char *cmd, command_fn_t fn)
{
  this->commands.insert({Client::hash(cmd), fn});
}


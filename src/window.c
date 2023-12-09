#include "../headers/window.h"
#include <ctype.h>
#include <ncurses.h>
#include <string.h>

WinWrapper new_win_wrapper(int height, int width, int y_start, int x_start,
                           bool has_border) {
  WINDOW *win = newwin(height, width, y_start, x_start);
  WinWrapper win_wrapper = {height, width, y_start, x_start, has_border, win};
  clear_win(&win_wrapper);
  refresh_win(&win_wrapper);

  return win_wrapper;
}

void free_win_wrapper(WinWrapper *win_wrapper) { delwin(win_wrapper->win); }

int x_end(WinWrapper *win_wrapper) {
  return win_wrapper->width + win_wrapper->x_start;
}
int y_end(WinWrapper *win_wrapper) {
  return win_wrapper->height + win_wrapper->y_start;
}

void clear_win(WinWrapper *win_wrapper) {
  wclear(win_wrapper->win);
  if (win_wrapper->has_border)
    win_border(win_wrapper->win);
}

void move_rel(WinWrapper *win_wrapper, int dy, int dx) {
  int x, y;
  getyx(win_wrapper->win, y, x);
  wmove(win_wrapper->win, y + dy, x + dx);
}

void refresh_win(WinWrapper *win_wrapper) { wrefresh(win_wrapper->win); }

void win_printf(WinWrapper *win_wrapper, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vw_printw(win_wrapper->win, fmt, args);
  va_end(args);
}

void mv_printf_yx(WinWrapper *win_wrapper, int y, int x, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  wmove(win_wrapper->win, y, x);
  vw_printw(win_wrapper->win, fmt, args);
  va_end(args);
}

void mv_printf_centered_var(WinWrapper *win_wrapper, int y, const char *fmt,
                            va_list args) {
  char buf[MAX_OUTPUT_LEN];
  int length = vsprintf(buf, fmt, args);

  int margin = (win_wrapper->width - length - 1) / 2;
  mvwaddstr(win_wrapper->win, y, margin + 1, buf);
}

void mv_printf_centered(WinWrapper *win_wrapper, int y, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  mv_printf_centered_var(win_wrapper, y, fmt, args);
  va_end(args);
}

void printf_centered_nl(WinWrapper *win_wrapper, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int y = getcury(win_wrapper->win) + 1;
  mv_printf_centered_var(win_wrapper, y, fmt, args);
  va_end(args);
}

void clear_line(WinWrapper *win_wrapper, int y_to_clear) {
  wmove(win_wrapper->win, y_to_clear, 1);
  wprintw(win_wrapper->win, "%*s", win_wrapper->width - 2, "");
  wmove(win_wrapper->win, y_to_clear, 1);
  // wclrtoeol(win_wrapper->win);
}
void clear_curr_line(WinWrapper *win_wrapper) {
  int y = getcury(win_wrapper->win);
  clear_line(win_wrapper, y);
}

void win_border(WINDOW *win) { box(win, 0, 0); }

char char_input() {
  char inp = getch();
  return tolower(inp);
}

char win_char_input(WinWrapper *win_wrapper) {
  char inp = wgetch(win_wrapper->win);
  return tolower(inp);
}

void str_to_lower(char *str) {
  int len = strlen(str);
  for (int i = 0; i < len; i++) {
    str[i] = tolower(str[i]);
  }
}

bool check_for_quit_input(char *input) {
  char copy[strlen(input) + 1];
  strcpy(copy, input);
  str_to_lower(copy);
  return strcmp(copy, "q") == 0 || strcmp(copy, "quit") == 0 ||
         strcmp(copy, "exit") == 0;
}

void prompt_input(WinWrapper *io_wrapper, const char *prompt, char *res) {
  printf_centered_nl(io_wrapper, prompt);
  wscanw(io_wrapper->win, "%" STR(MAX_INPUT_LEN) "s", res);
  move_rel(io_wrapper, -1, 0);
}

bool int_prompt_input(WinWrapper *io_wrapper, const char *prompt, int *res) {
  char input[MAX_INPUT_LEN];
  prompt_input(io_wrapper, prompt, input);
  if (check_for_quit_input(input)) {
    *res = -1;
    return true;
  }
  sscanf(input, "%d", res);
  return false;
}

bool int_prompt_input_untill(WinWrapper *io_wrapper, const char *prompt,
                             int *res) {
  *res = -1;
  bool quit = int_prompt_input(io_wrapper, prompt, res);
  while (*res <= -1 && !quit) {
    clear_curr_line(io_wrapper);
    quit = int_prompt_input(io_wrapper, prompt, res);
  }
  return quit;
}

void clear_refresh_win(WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  refresh_win(win_wrapper);
}

#include "../headers/window.hpp"
#include <ncurses.h>

WinWrapper new_win_wrapper(int height, int width, int y_start, int x_start,
                           bool has_border) {
  WINDOW *win = newwin(height, width, y_start, x_start);
  return WinWrapper{height, width, y_start, x_start, has_border, win};
}

WinWrapper new_win_wrapper_at_center(int height, int width, bool has_border) {
  int x_max, y_max;
  getmaxyx(stdscr, y_max, x_max);

  int y_start = (y_max / 2) - (height / 2);
  int x_start = (x_max / 2) - (width / 2);

  return new_win_wrapper(height, width, y_start, x_start, has_border);
}

void clear_win(WinWrapper *win_wrapper) {
  wclear(win_wrapper->win);
  if (win_wrapper->has_border)
    win_border(win_wrapper->win);
}

void print_centered_str(WinWrapper *win_wrapper, int y, const char *str) {
  int str_len = sizeof str - 1;
  if (str_len <= 0)
    return;

  int margin = (win_wrapper->width - str_len) / 2;
  mvwaddstr(win_wrapper->win, y, margin + 1, str);
}

void win_border(WINDOW *win) { box(win, 0, 0); }
void refresh_win(WINDOW *win) { wrefresh(win); }

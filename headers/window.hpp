#pragma once
#include <ncurses.h>

struct WinWrapper {
  int width, height;
  int y_start, x_start;
  bool has_border;
  WINDOW *win;
};
WinWrapper new_win_wrapper(int height, int width, int y_start, int x_start,
                           bool has_border);
WinWrapper new_win_wrapper_at_center(int height, int width, bool has_border);

void clear_win(WinWrapper *win_wrapper);
void print_centered_str(WinWrapper *win_wrapper, int y, const char *str);

void win_border(WINDOW *win);
void refresh_win(WINDOW *win);

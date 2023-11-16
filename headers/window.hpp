#pragma once
#include <ncurses.h>
#include <string>

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

void print(WinWrapper *win_wrapper, std::string str);
void print_colored(WinWrapper *win_wrapper, std::string str, int fg_color,
                   int bg_color);
void mv_print(WinWrapper *win_wrapper, int y, int x, std::string str);
void mv_print_colored(WinWrapper *win_wrapper, int y, int x, std::string str,
                      int fg_color, int bg_color);
void mv_print_centered(WinWrapper *win_wrapper, int y, std::string str);

void win_border(WINDOW *win);
void refresh_win(WINDOW *win);

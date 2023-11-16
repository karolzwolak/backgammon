#include "../headers/window.hpp"
#include <curses.h>
#include <ncurses.h>
#include <string>

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

void print(WinWrapper *win_wrapper, std::string str) {
  waddstr(win_wrapper->win, str.c_str());
}
void print_colored(WinWrapper *win_wrapper, std::string str, int fg_color,
                   int bg_color) {
  attron(fg_color);
  attron(bg_color);
  print(win_wrapper, str);
  attroff(fg_color);
  attroff(bg_color);
}
void mv_print_str(WinWrapper *win_wrapper, int y, int x, std::string str) {
  wmove(win_wrapper->win, y, x);
  print(win_wrapper, str);
}

void mv_print_centered(WinWrapper *win_wrapper, int y, std::string str) {
  int str_len = sizeof str - 1;
  if (str_len <= 0)
    return;

  int margin = (win_wrapper->width - str_len) / 2;
  mv_print_str(win_wrapper, y, margin + 1, str);
}
void mv_print_colored(WinWrapper *win_wrapper, int y, int x, std::string str,
                      int fg_color = A_NORMAL, int bg_color = A_NORMAL) {
  wmove(win_wrapper->win, y, x);
  print_colored(win_wrapper, str, fg_color, bg_color);
}

void win_border(WINDOW *win) { box(win, 0, 0); }
void refresh_win(WINDOW *win) { wrefresh(win); }

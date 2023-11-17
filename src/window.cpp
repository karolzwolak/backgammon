#include "../headers/window.hpp"
#include <cctype>
#include <curses.h>
#include <ncurses.h>
#include <string>

WinWrapper new_win_wrapper(int height, int width, int y_start, int x_start,
                           bool has_border) {
  WINDOW *win = newwin(height, width, y_start, x_start);
  return WinWrapper{height, width, y_start, x_start, has_border, win};
}

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
  int margin = (win_wrapper->width - str.length() - 2) / 2;
  mv_print_str(win_wrapper, y, margin + 1, str);
}
void mv_print_colored(WinWrapper *win_wrapper, int y, int x, std::string str,
                      int fg_color = A_NORMAL, int bg_color = A_NORMAL) {
  wmove(win_wrapper->win, y, x);
  print_colored(win_wrapper, str, fg_color, bg_color);
}

void win_border(WINDOW *win) { box(win, 0, 0); }
void refresh_win(WINDOW *win) { wrefresh(win); }

char char_input() {
  char inp = getch();
  return tolower(inp);
}

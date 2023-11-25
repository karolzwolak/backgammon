#include "../headers/window.h"
#include <ctype.h>
#include <curses.h>
#include <string.h>

WinWrapper new_win_wrapper(int height, int width, int y_start, int x_start,
                           bool has_border) {
  WINDOW *win = newwin(height, width, y_start, x_start);
  WinWrapper win_wrapper = {height, width, y_start, x_start, has_border, win};
  clear_win(&win_wrapper);
  refresh_win(&win_wrapper);

  return win_wrapper;
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

void refresh_win(WinWrapper *win_wrapper) { wrefresh(win_wrapper->win); }

void print(WinWrapper *win_wrapper, const char *str) {
  waddstr(win_wrapper->win, str);
}

void mv_print_str(WinWrapper *win_wrapper, int y, int x, const char *str) {
  wmove(win_wrapper->win, y, x);
  print(win_wrapper, str);
}

void mv_print_centered(WinWrapper *win_wrapper, int y, const char *str) {
  int length = strlen(str);
  int margin = (win_wrapper->width - length - 2) / 2;
  mv_print_str(win_wrapper, y, margin + 1, str);
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

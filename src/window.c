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

void print_centered_on_new_line(WinWrapper *win_wrapper, const char *str) {
  int x, y;
  getyx(win_wrapper->win, y, x);
  if (x == 0)
    y++;
  mv_print_centered(win_wrapper, y, str);
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

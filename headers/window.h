#pragma once
#include <ncurses.h>

typedef struct {
  int height, width;
  int y_start, x_start;
  bool has_border;
  WINDOW *win;
} WinWrapper;
WinWrapper new_win_wrapper(int height, int width, int y_start, int x_start,
                           bool has_border);
WinWrapper new_win_wrapper_at_center(int height, int width, bool has_border);

void clear_win(WinWrapper *win_wrapper);
void refresh_win(WinWrapper *win_wrapper);

void print(WinWrapper *win_wrapper, const char *str);
void mv_print_str(WinWrapper *win_wrapper, int y, int x, const char *str);
void mv_print_centered(WinWrapper *win_wrapper, int y, const char *str);

void win_border(WINDOW *win);

char char_input();
char win_char_input(WinWrapper *win_wrapper);

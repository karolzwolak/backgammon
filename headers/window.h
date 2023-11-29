#pragma once
#include <ncurses.h>

#define MAX_INPUT_LEN 20
#define _STR(x) #x
#define STR(X) _STR(X)

typedef struct {
  int height, width;
  int y_start, x_start;
  bool has_border;
  WINDOW *win;
} WinWrapper;

WinWrapper new_win_wrapper(int height, int width, int y_start, int x_start,
                           bool has_border);
WinWrapper new_win_wrapper_at_center(int height, int width, bool has_border);

void free_win_wrapper(WinWrapper *win_wrapper);

void clear_win(WinWrapper *win_wrapper);
void refresh_win(WinWrapper *win_wrapper);
void move_rel(WinWrapper *win_wrapper, int dy, int dx);

void print(WinWrapper *win_wrapper, const char *str);
void mv_print_str(WinWrapper *win_wrapper, int y, int x, const char *str);
void mv_print_centered(WinWrapper *win_wrapper, int y, const char *str);
void print_centered_on_new_line(WinWrapper *win_wrapper, const char *str);

void clear_line(WinWrapper *win_wrapper, int y_to_clear);
void clear_curr_line(WinWrapper *win_wrapper);

void win_border(WINDOW *win);

char char_input();
char win_char_input(WinWrapper *win_wrapper);

void str_to_lower(char *str);
bool check_for_quit_input(char *input);

void prompt_input(WinWrapper *io_wrapper, const char *prompt, char *res);
// false means user wants to quit
bool int_prompt_input(WinWrapper *io_wrapper, const char *prompt, int *res);

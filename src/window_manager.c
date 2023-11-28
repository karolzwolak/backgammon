#include "../headers/window_manager.h"
#include "../headers/game.h"
#include "../headers/window.h"
#include "game.c"
#include "window.c"
#include <ncurses.h>

const int VERTICAL_MARGIN = 2;
const int IN_BETWEEN_LINES_MARGIN = 1;

const int WIN_HORIZONTAL_MARGIN = 2;
const int WIN_VERTICAL_MARGIN = 1;

const int CONTENT_WIN_HEIGHT = BOARD_HEIGHT + 2;
const int CONTENT_WIN_WIDTH = BOARD_WIDTH + 2;

const int SIDE_WIN_HEIGHT = CONTENT_WIN_HEIGHT;
const int SIDE_WIN_WIDTH = 16;

const int ABOUT_WIN_HEIGHT = 3;
const int ABOUT_WIN_WIDTH =
    2 * WIN_HORIZONTAL_MARGIN + SIDE_WIN_WIDTH * 2 + CONTENT_WIN_WIDTH;

const int IO_WIN_HEIGHT = CONTENT_WIN_HEIGHT / 2;
const int IO_WIN_WIDTH = ABOUT_WIN_WIDTH;

const int MIN_HEIGHT = ABOUT_WIN_HEIGHT + CONTENT_WIN_HEIGHT + IO_WIN_HEIGHT +
                       4 * WIN_VERTICAL_MARGIN + 2;
const int MIN_WIDTH = ABOUT_WIN_WIDTH + 2 * WIN_HORIZONTAL_MARGIN + 2;

WinManager new_win_manager() {
  int term_height, term_width;
  getmaxyx(stdscr, term_height, term_width);

  int y_start = (term_height - MIN_HEIGHT) / 2;
  int x_start = (term_width - MIN_WIDTH) / 2;

  WinWrapper main_win =
      new_win_wrapper(MIN_HEIGHT, MIN_WIDTH, y_start, x_start, true);

  y_start += WIN_VERTICAL_MARGIN + 1;
  x_start += WIN_HORIZONTAL_MARGIN + 1;

  WinWrapper about_win = new_win_wrapper(ABOUT_WIN_HEIGHT, ABOUT_WIN_WIDTH,
                                         y_start, x_start, true);
  WinWrapper legend_win =
      new_win_wrapper(SIDE_WIN_HEIGHT, SIDE_WIN_WIDTH,
                      y_end(&about_win) + WIN_VERTICAL_MARGIN, x_start, true);

  WinWrapper content_win =
      new_win_wrapper(CONTENT_WIN_HEIGHT, CONTENT_WIN_WIDTH,
                      y_end(&about_win) + WIN_VERTICAL_MARGIN,
                      x_end(&legend_win) + WIN_HORIZONTAL_MARGIN, true);

  WinWrapper stats_win = new_win_wrapper(
      SIDE_WIN_HEIGHT, SIDE_WIN_WIDTH, y_end(&about_win) + WIN_VERTICAL_MARGIN,
      x_end(&content_win) + WIN_HORIZONTAL_MARGIN, true);

  WinWrapper io_win =
      new_win_wrapper(IO_WIN_HEIGHT, IO_WIN_WIDTH,
                      y_end(&content_win) + WIN_VERTICAL_MARGIN, x_start, true);

  WinManager res = {main_win,  about_win, legend_win,  content_win,
                    stats_win, io_win,    term_height, term_width};
  return res;
}

void add_main_border(WinWrapper *main_win) {
  clear_win(main_win);
  refresh_win(main_win);
}

void print_line(WinWrapper *win_wrapper, int *line, const char *str) {
  mv_print_centered(win_wrapper, *line, str);
  *line += IN_BETWEEN_LINES_MARGIN + 1;
}

void print_menu(WinWrapper *win_wrapper) {
  int line = VERTICAL_MARGIN + 1;
  print_line(win_wrapper, &line, "Play");
  print_line(win_wrapper, &line, "Watch your previous games");
  print_line(win_wrapper, &line, "Hall of fame");
  print_line(win_wrapper, &line, "How to play");
  print_line(win_wrapper, &line, "About");
}

void display_menu_ui(WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  print_menu(win_wrapper);
  refresh_win(win_wrapper);
}

void display_menu(WinManager *win_manager) {
  display_menu_ui(&win_manager->content_win);
}

void disable_cursor() {
  noecho();
  curs_set(0);
}

void enable_cursor() {
  echo();
  curs_set(1);
}

void menu_loop(WinManager *win_manager) {
  disable_cursor();

  while (true) {
    display_menu(win_manager);
    switch (char_input()) {
    case 'q':
      return;
    case 'p':
      // enable_cursor();
      game_loop(win_manager);
      disable_cursor();
      break;
    default:
      break;
    }
  }
}

void show_about_info(WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  mv_print_centered(
      win_wrapper, 1,
      "backgammon game in terminal, made by: Karol Zwolak, id: 197883");
  refresh_win(win_wrapper);
}

void run() {
  WinManager win_manager = new_win_manager();

  show_about_info(&win_manager.about_win);
  menu_loop(&win_manager);

  /* menu_loop(&win_manager); */
}

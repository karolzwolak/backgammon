#include "../headers/window_manager.hpp"
#include "../headers/window.hpp"
#include "board.cpp"
#include "window.cpp"
#include <curses.h>
#include <ncurses.h>

const int VERTICAL_MARGIN = 2;
const int IN_BETWEEN_LINES_MARGIN = 1;

const int WIN_HORIZONTAL_MARGIN = 2;
const int WIN_VERTICAL_MARGIN = 1;

const int BOARD_HEIGHT = 13 + 2;
const int BOARD_WIDTH = 51 + 4;

const int SIDE_WIN_WIDTH = 16;
const int SIDE_WIN_HEIGHT = BOARD_HEIGHT;

const int ABOUT_WIN_HEIGHT = 3;
const int ABOUT_WIN_WIDTH =
    2 * WIN_HORIZONTAL_MARGIN + SIDE_WIN_WIDTH * 2 + BOARD_WIDTH;

const int IO_WIN_HEIGHT = BOARD_HEIGHT / 2;
const int IO_WIN_WIDTH = ABOUT_WIN_WIDTH;

const int MIN_WIDTH = ABOUT_WIN_WIDTH + 2 * WIN_HORIZONTAL_MARGIN + 2;
const int MIN_HEIGHT = ABOUT_WIN_HEIGHT + BOARD_HEIGHT + IO_WIN_HEIGHT +
                       4 * WIN_VERTICAL_MARGIN + 2;

struct WinManager {
  WinWrapper main_win, about_win, legend_win, content_win, stats_win, io_win;
  int term_height, term_width;
};

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

  WinWrapper content_win = new_win_wrapper(
      BOARD_HEIGHT, BOARD_WIDTH, y_end(&about_win) + WIN_VERTICAL_MARGIN,
      x_end(&legend_win) + WIN_HORIZONTAL_MARGIN, true);

  WinWrapper stats_win = new_win_wrapper(
      SIDE_WIN_HEIGHT, SIDE_WIN_WIDTH, y_end(&about_win) + WIN_VERTICAL_MARGIN,
      x_end(&content_win) + WIN_HORIZONTAL_MARGIN, true);

  WinWrapper io_win =
      new_win_wrapper(IO_WIN_HEIGHT, IO_WIN_WIDTH,
                      y_end(&content_win) + WIN_VERTICAL_MARGIN, x_start, true);

  return WinManager{main_win,  about_win, legend_win,  content_win,
                    stats_win, io_win,    term_height, term_width};
}

void add_main_border(WinWrapper *main_win) {
  clear_win(main_win);
  refresh_win(main_win->win);
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

void display_menu(WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  print_menu(win_wrapper);
  refresh_win(win_wrapper->win);
}

void enter_menu() {
  noecho();
  curs_set(0);
}

void leave_menu() {
  echo();
  curs_set(1);
}

void menu_loop(WinManager *win_manager) {
  enter_menu();
  display_menu(&win_manager->content_win);

  while (true) {
    switch (char_input()) {
    case 'q':
      return;
    default:
      break;
    }
  }
}

void test_print(WinWrapper *win_wrapper, const char *str) {
  clear_win(win_wrapper);
  mv_print_centered(win_wrapper, 1, str);
  refresh_win(win_wrapper->win);
}

void debug_menu(WinManager *win_manager) {
  test_print(&win_manager->legend_win, "legend");
  test_print(&win_manager->about_win, "about");
  test_print(&win_manager->stats_win, "stats");
  test_print(&win_manager->io_win, "io");
  display_menu(&win_manager->content_win);
}

void run() {
  WinManager win_manager = new_win_manager();

  add_main_border(&win_manager.main_win);

  debug_menu(&win_manager);

  getch();

  /* menu_loop(&win_manager); */
}

#include "../headers/window_manager.h"
#include "../headers/game.h"
#include "../headers/window.h"
#include "game.c"
#include "window.c"
#include <ncurses.h>
#include <time.h>

WinWrapper main_w(int y, int x) {
  return new_win_wrapper(MIN_HEIGHT, MIN_WIDTH, y, x, true);
}

WinWrapper about_w(int y, int x) {
  return new_win_wrapper(ABOUT_WIN_HEIGHT, ABOUT_WIN_WIDTH, y, x, true);
}

WinWrapper legend_w(int y, int x) {
  return new_win_wrapper(SIDE_WIN_HEIGHT, SIDE_WIN_WIDTH, y, x, true);
}

WinWrapper content_w(int y, int x) {
  return new_win_wrapper(CONTENT_WIN_HEIGHT, CONTENT_WIN_WIDTH, y, x, true);
}

WinWrapper stats_w(int y, int x) {
  return new_win_wrapper(SIDE_WIN_HEIGHT, SIDE_WIN_WIDTH, y, x, true);
}

WinWrapper io_w(int y, int x) {
  return new_win_wrapper(IO_WIN_HEIGHT, IO_WIN_WIDTH, y, x, true);
}

void setup_main_win(WinManager *manager, int *y_start, int *x_start) {
  getmaxyx(stdscr, manager->term_height, manager->term_width);

  *y_start = (manager->term_height - MIN_HEIGHT) / 2;
  *x_start = (manager->term_width - MIN_WIDTH) / 2;

  manager->main_win = main_w(*y_start, *x_start);

  *y_start += WIN_V_MARGIN + 1;
  *x_start += WIN_H_MARGIN + 1;
}

WinManager new_win_manager() {
  WinManager res;
  int x_start, y_start;
  setup_main_win(&res, &y_start, &x_start);

  res.about_win = about_w(y_start, x_start);
  res.legend_win = legend_w(y_end(&res.about_win) + WIN_V_MARGIN, x_start);

  res.content_win = content_w(y_end(&res.about_win) + WIN_V_MARGIN,
                              x_end(&res.legend_win) + WIN_H_MARGIN);

  res.stats_win = stats_w(y_end(&res.about_win) + WIN_V_MARGIN,
                          x_end(&res.content_win) + WIN_H_MARGIN);

  res.io_win = io_w(y_end(&res.content_win) + WIN_V_MARGIN, x_start);

  return res;
}

void free_win_manager(WinManager *manager) {
  free_win_wrapper(&manager->main_win);
  free_win_wrapper(&manager->about_win);
  free_win_wrapper(&manager->legend_win);
  free_win_wrapper(&manager->content_win);
  free_win_wrapper(&manager->stats_win);
  free_win_wrapper(&manager->io_win);
}

void add_main_border(WinWrapper *main_win) {
  clear_win(main_win);
  refresh_win(main_win);
}

void print_line(WinWrapper *win_wrapper, int *line, const char *str) {
  mv_printf_centered(win_wrapper, *line, str);
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
      play_menu_loop(win_manager);
      clear_refresh_win(&win_manager->stats_win);
      clear_refresh_win(&win_manager->io_win);
      break;
    case 'w':
      watch_menu_loop(win_manager);
      clear_refresh_win(&win_manager->stats_win);
      break;
    default:
      break;
    }
  }
}

void show_about_info(WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  mv_printf_centered(
      win_wrapper, 1,
      "backgammon game in terminal, made by: Karol Zwolak, id: 197883");
  refresh_win(win_wrapper);
}

void init_rng() { srand(time(NULL)); }

void run() {
  WinManager win_manager = new_win_manager();
  init_rng();

  show_about_info(&win_manager.about_win);
  menu_loop(&win_manager);

  free_win_manager(&win_manager);
}

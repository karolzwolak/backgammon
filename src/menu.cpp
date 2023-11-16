#include "../headers/window.hpp"
#include <curses.h>
#include <ncurses.h>
#include <string>

const int VERTICAL_MARGIN = 2;
const int IN_BETWEEN_LINES_MARGIN = 1;

void print_line(WinWrapper *win_wrapper, int *line, std::string str) {
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

void menu_loop(WinWrapper *win_wrapper) {
  enter_menu();
  display_menu(win_wrapper);

  while (true) {
    switch (char_input()) {
    case 'q':
      return;
    default:
      break;
    }
  }
}

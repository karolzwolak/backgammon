#include "window_manager.c"
#include <ncurses.h>

void init_ncurses() {
  initscr();
  refresh();
}

void cleanup_ncurses() { endwin(); }

int main() {
  init_ncurses();

  run();

  cleanup_ncurses();

  return 0;
}

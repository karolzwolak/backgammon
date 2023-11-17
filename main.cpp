#include "src/window_manager.cpp"
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

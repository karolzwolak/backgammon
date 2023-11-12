#include "src/board.cpp"
#include "src/window.cpp"
#include <ncurses.h>

const int BOARD_HEIGHT = 13 + 2;
const int BOARD_WIDTH = 51 + 4;

void init_ncurses() {
  initscr();
  refresh();
}

void cleanup_ncurses() { endwin(); }

int main() {
  init_ncurses();

  WinWrapper content_win =
      new_win_wrapper_at_center(BOARD_HEIGHT, BOARD_WIDTH, true);

  Board board = new_board();

  display_board(&board, &content_win);

  getch();

  cleanup_ncurses();

  return 0;
}

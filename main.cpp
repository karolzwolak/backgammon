#include "board.cpp"
#include <ncurses.h>

const int WIN_HEIGHT = 20;
const int WIN_WIDTH = 79;

int main() {
  Board board = new_board(WIN_HEIGHT, WIN_WIDTH);

  print_board(&board);
  getch();

  cleanup_board();

  return 0;
}

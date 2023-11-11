#include "board.cpp"
#include <ncurses.h>

const int WIN_SIZE = 17;
const int WIN_HEIGHT = WIN_SIZE;
const int WIN_WIDTH = WIN_SIZE * 2;

int main() {
  Board board = new_board(WIN_HEIGHT, WIN_WIDTH);

  printw("Hello World!");

  getch();

  cleanup_board();

  return 0;
}

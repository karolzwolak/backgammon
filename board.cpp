#include "window.cpp"
#include <ncurses.h>

struct Board {};

Board new_board() { return Board{}; }

void print_board(Board *board, WinWrapper *win_wrapper) {
  WINDOW *win = win_wrapper->win;

  mvwaddstr(win, 1, 2, "12  11  10  9   8   7   | |   6   5   4   3   2   1");
  mvwaddstr(win, 7, 2, "---------------------- [BAR] ----------------------");
  mvwaddstr(win, 13, 2, "13  14  15  16  17  18  | |  19  20  21  22  23  24");

  for (int i = 0; i < 5; i++) {
    mvwaddstr(win, 12 - i, 6 * 4 + 2, "| |");
    mvwaddstr(win, 2 + i, 6 * 4 + 2, "| |");
  }
}

void display_board(Board *board, WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  print_board(board, win_wrapper);
  refresh_win(win_wrapper->win);
}

/* void print_board(Board *board) { */
/*   for (int i = 0; i < 6; i++) { */
/*     mvwaddstr(board->win, 1 + i, 2, */
/*               "| |   \\ /   | |   \\ /   | |   \\ /   ---   " */
/*               "| |   \\ /   | |   \\ /   | |   \\ /"); */
/*     mvwaddstr(board->win, 1 + 6 + i, 2, */
/*               "                                    ---   " */
/*               "                                 "); */
/*     mvwaddstr(board->win, board->win_height - 2 - i, 2, */
/*               "/ \\   | |   / \\   | |   / \\   | |   ---   " */
/*               "/ \\   | |   / \\   | |   / \\   | |"); */
/*   } */
/**/
/*   refresh_win(board->win); */
/* } */

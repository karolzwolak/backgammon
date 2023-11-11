#include <ncurses.h>

struct Board {
  int win_height;
  int win_width;
  WINDOW *win;
};
void cleanup_board() { endwin(); }

void add_border(WINDOW *win) { box(win, 0, 0); }

void clear_win(WINDOW *win) {
  wclear(win);
  add_border(win);
}

void refresh_win(WINDOW *win) { wrefresh(win); }

WINDOW *create_win(int height, int width) {
  initscr();
  refresh();

  int x_max, y_max;
  getmaxyx(stdscr, y_max, x_max);

  int y_start = (y_max / 2) - (height / 2);
  int x_start = (x_max / 2) - (width / 2);

  return newwin(height, width, y_start, x_start);
}

Board new_board(int height, int width) {
  WINDOW *win = create_win(height, width);

  clear_win(win);
  refresh_win(win);

  return Board{height, width, win};
}

void print_board(Board *board) {
  for (int i = 0; i < 6; i++) {
    mvwaddstr(board->win, 1 + i, 2,
              "| |   \\ /   | |   \\ /   | |   \\ /   ---   "
              "| |   \\ /   | |   \\ /   | |   \\ /");
    mvwaddstr(board->win, 1 + 6 + i, 2,
              "                                    ---   "
              "                                 ");
    mvwaddstr(board->win, board->win_height - 2 - i, 2,
              "/ \\   | |   / \\   | |   / \\   | |   ---   "
              "/ \\   | |   / \\   | |   / \\   | |");
  }

  refresh_win(board->win);
}

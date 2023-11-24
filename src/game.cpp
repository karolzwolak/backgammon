#include "../headers/game.hpp"
#include "../headers/window.hpp"
#include "../headers/window_manager.hpp"
#include <ncurses.h>

const int BOARD_SIZE = 24;

enum PlayerKind { None, White, Red };

struct Player {
  const char *name;
  PlayerKind player_kind;
};

struct BoardCell {
  PlayerKind pawn_player_kind;
  int pawn_count;
};

bool board_cell_is_empty(BoardCell *board_cell) {
  return board_cell->pawn_count > 0;
}

BoardCell empty_board_cell() { return BoardCell{None, 0}; }

BoardCell new_board_cell(PlayerKind player_kind, int pawn_count) {
  return BoardCell{player_kind, pawn_count};
}

struct Board {
  BoardCell board_cells[BOARD_SIZE];
};

Board empty_board() {
  Board board = Board{{}};

  for (int i = 0; i < BOARD_SIZE; i++)
    board.board_cells[i] = empty_board_cell();

  return board;
}

void set_pawns(Board *board, int id, PlayerKind player_kind, int count) {
  board->board_cells[id].pawn_player_kind = player_kind;
  board->board_cells[id].pawn_count = count;
}

Board default_board() {
  Board board = empty_board();
  int default_board_positions[] = {0, 11, 16, 18};
  int default_board_pawn_counts[] = {2, 5, 3, 5};

  for (int i = 0; i < 4; i++) {
    set_pawns(&board, default_board_positions[i], White,
              default_board_pawn_counts[i]);
    set_pawns(&board, BOARD_SIZE - default_board_positions[i], Red,
              default_board_pawn_counts[i]);
  }
  return board;
}

struct GameManager {
  Board board;

  Player white_player;
  Player red_player;
};

Board new_board() { return Board{}; }

void print_board_ui(WinWrapper *win_wrapper) {
  WINDOW *win = win_wrapper->win;

  mvwaddstr(win, 1, 2, "12  11  10  9   8   7   | |   6   5   4   3   2   1");
  mvwaddstr(win, 7, 2, "---------------------- [BAR] ----------------------");
  mvwaddstr(win, 13, 2, "13  14  15  16  17  18  | |  19  20  21  22  23  24");

  for (int i = 0; i < 5; i++) {
    mvwaddstr(win, 12 - i, 6 * 4 + 2, "| |");
    mvwaddstr(win, 2 + i, 6 * 4 + 2, "| |");
  }
}

void print_board_pawns(WinWrapper *win_wrapper, Board *board) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < board->board_cells[i].pawn_count; j++) {
      /* mv_print_str(win_wrapper, j + 1, i + 1, "*"); */
    }
  }
}

void print_board(Board *board, WinWrapper *win_wrapper) {
  print_board_ui(win_wrapper);
  print_board_pawns(win_wrapper, board);
}

void display_board(Board *board, WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  print_board(board, win_wrapper);
  refresh_win(win_wrapper->win);
}
void game_loop(WinManager *win_manager) {
  Board board = default_board();

  display_board(&board, &win_manager->content_win);
}

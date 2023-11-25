#include "../headers/game.hpp"
#include "../headers/window.hpp"
#include "../headers/window_manager.hpp"
#include <ncurses.h>

const int BOARD_SIZE = 24;
const int HALF_BOARD = BOARD_SIZE / 2;
const int QUARTER_BOARD = BOARD_SIZE / 4;

const int CONTENT_HORIZONTAL_MARGIN = 1;
const int CONTENT_VERTICAL_MARGIN = 0;

// count of rows inside board
const int BOARD_ROW_COUNT = 5;
const int BOARD_COL_COUNT = BOARD_SIZE / 2;
const int CONTENT_CELL_SPACING = 3;
const int CONTENT_CELL_WIDTH = CONTENT_CELL_SPACING + 1;

const int BAR_INNER_WIDTH = 1;
const int BAR_VERTICAL_GAP = 1;
const int BAR_HORIZONTAL_GAP = 5 + BAR_INNER_WIDTH;
const int BAR_HORIZONTAL_COMBINED_GAP =
    2 * CONTENT_CELL_WIDTH - BAR_HORIZONTAL_GAP + BAR_INNER_WIDTH;
;

const int CONTENT_WIDTH = BOARD_COL_COUNT * CONTENT_CELL_WIDTH +
                          BAR_HORIZONTAL_COMBINED_GAP +
                          2 * CONTENT_HORIZONTAL_MARGIN;
const int CONTENT_HEIGHT =
    BOARD_ROW_COUNT * 2 + BAR_VERTICAL_GAP + 2 * CONTENT_VERTICAL_MARGIN + 2;

// content start/end is the first or last index that we can draw characters at
const int CONTENT_Y_START = CONTENT_VERTICAL_MARGIN + 1;
const int CONTENT_X_START = CONTENT_HORIZONTAL_MARGIN + 1;
const int CONTENT_X_END = CONTENT_WIDTH - CONTENT_HORIZONTAL_MARGIN;
const int CONTENT_Y_END = CONTENT_HEIGHT - CONTENT_VERTICAL_MARGIN;

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

void print_board_cell_top(WinWrapper *win_wrapper, BoardCell *board_cell,
                          int id) {
  int y = CONTENT_Y_START + 1;
  int x = CONTENT_X_END - id * CONTENT_CELL_WIDTH;

  if (id >= QUARTER_BOARD)
    x -= BAR_HORIZONTAL_GAP;

  for (int i = 0; i < board_cell->pawn_count; i++) {
    mv_print_str(win_wrapper, y, x, "*");
    y += 1;
  }
}

void print_board_cell_bot(WinWrapper *win_wrapper, BoardCell *board_cell,
                          int id) {
  id = id % 12;
  int y = CONTENT_Y_END - 1;
  int x = CONTENT_X_START + id * CONTENT_CELL_WIDTH;

  if (id >= QUARTER_BOARD)
    x += BAR_HORIZONTAL_GAP;

  for (int i = 0; i < board_cell->pawn_count; i++) {
    mv_print_str(win_wrapper, y, x, "*");
    y -= 1;
  }
}

void print_board_cell(WinWrapper *win_wrapper, BoardCell *board_cell, int id) {

  bool on_top = id < HALF_BOARD;
  if (!on_top) {
    print_board_cell_bot(win_wrapper, board_cell, id);
    return;
  }
  print_board_cell_top(win_wrapper, board_cell, id);
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
  /* int default_board_positions[] = {0, 11, 16, 18}; */
  int default_board_positions[] = {0, 5, 6, 7};
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

  mv_print_str(win_wrapper, CONTENT_Y_START, CONTENT_X_START,
               "12  11  10  9   8   7   | |   6   5   4   3   2   1");
  mv_print_str(win_wrapper, CONTENT_Y_START + CONTENT_HEIGHT / 2,
               CONTENT_X_START,
               "---------------------- [BAR] ----------------------");
  mv_print_str(win_wrapper, CONTENT_Y_END, CONTENT_X_START,
               "13  14  15  16  17  18  | |  19  20  21  22  23  24");

  for (int i = 0; i < BOARD_ROW_COUNT; i++) {
    mv_print_centered(win_wrapper, CONTENT_Y_START + i + 1, "| |");
    mv_print_centered(win_wrapper, CONTENT_Y_END - i - 1, "| |");
  }
}

void print_board_pawns(WinWrapper *win_wrapper, Board *board) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    print_board_cell(win_wrapper, &board->board_cells[i], i);
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
  /* mv_print_str(&win_manager->content_win, CONTENT_Y_START, CONTENT_X_START,
   */
  /*              "*"); */
  /* mv_print_str(&win_manager->content_win, CONTENT_Y_END, CONTENT_X_END, "*");
   */
  refresh_win(win_manager->content_win.win);
}

#include "../headers/game.hpp"
#include "../headers/window.hpp"
#include "../headers/window_manager.hpp"
#include <ncurses.h>
#include <stdio.h>

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

const int BAR_VERTICAL_GAP = 1;
const int BAR_HORIZONTAL_GAP = 6;
const int BAR_HORIZONTAL_COMBINED_GAP =
    BAR_HORIZONTAL_GAP - CONTENT_CELL_WIDTH + 1;
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

void print_overflowing_pawns(WinWrapper *win_wrapper, BoardCell *board_cell,
                             int y, int x) {

  char out[3] = "";

  sprintf(out, "%02d", board_cell->pawn_count - BOARD_ROW_COUNT);
  mv_print_str(win_wrapper, y, x, out);
}

void pawn_draw_char(char *out, PlayerKind player_kind) {
  if (player_kind == Red)
    sprintf(out, "R");
  else if (player_kind == White)
    sprintf(out, "W");
}

void print_board_cell(WinWrapper *win_wrapper, BoardCell *board_cell, int id) {

  if (board_cell->pawn_count == 0 || board_cell->pawn_player_kind == None)
    return;

  char out[2] = "";
  pawn_draw_char(out, board_cell->pawn_player_kind);

  bool on_bottom = id >= HALF_BOARD;
  int y, x, move_by;

  if (on_bottom) {
    id = id % 12;
    y = CONTENT_Y_END - 1;
    x = CONTENT_X_START + id * CONTENT_CELL_WIDTH;
    move_by = -1;

    if (id >= QUARTER_BOARD)
      x += BAR_HORIZONTAL_GAP;
  } else {
    y = CONTENT_Y_START + 1;
    x = CONTENT_X_END - id * CONTENT_CELL_WIDTH;
    move_by = 1;

    if (id >= QUARTER_BOARD)
      x -= BAR_HORIZONTAL_GAP;
  }

  int draw_count = board_cell->pawn_count;
  if (draw_count > BOARD_ROW_COUNT)
    draw_count = BOARD_ROW_COUNT - 1;
  for (int i = 0; i < draw_count; i++) {
    mv_print_str(win_wrapper, y, x, out);
    y += move_by;
  }

  if (x > CONTENT_WIDTH / 2)
    x -= 1;
  if (board_cell->pawn_count > BOARD_ROW_COUNT)
    print_overflowing_pawns(win_wrapper, board_cell, y, x);
}

void print_pawns_on_bar(WinWrapper *win_wrapper, BoardCell *board_cell) {
  if (board_cell->pawn_count == 0 || board_cell->pawn_player_kind == None)
    return;

  char out[2] = "";
  pawn_draw_char(out, board_cell->pawn_player_kind);

  int start_y, move_dir;
  if (board_cell->pawn_player_kind == Red) {
    start_y = CONTENT_Y_START + 1;
    move_dir = 1;

  } else {
    start_y = CONTENT_Y_END - 1;
    move_dir = -1;
  }

  for (int i = 0; i < board_cell->pawn_count; i++) {
    int col = i % 3;
    int row = i / 3;

    mv_print_str(win_wrapper, start_y + row * move_dir, CONTENT_WIDTH / 2 + col,
                 out);
  }
}

struct Board {
  BoardCell board_cells[BOARD_SIZE];
  BoardCell white_player_bar;
  BoardCell red_player_bar;
};

Board empty_board() {
  BoardCell white_player_bar = empty_board_cell();
  BoardCell red_player_bar = empty_board_cell();
  white_player_bar.pawn_player_kind = White;
  red_player_bar.pawn_player_kind = Red;

  Board board = Board{{}, white_player_bar, red_player_bar};

  for (int i = 0; i < BOARD_SIZE; i++)
    board.board_cells[i] = empty_board_cell();

  return board;
}

void set_pawns(Board *board, int id, PlayerKind player_kind, int count) {
  board->board_cells[id].pawn_player_kind = player_kind;
  board->board_cells[id].pawn_count = count;
}

void add_pawn_to_bar(Board *board, PlayerKind player_kind) {
  if (player_kind == Red)
    board->red_player_bar.pawn_count++;
  else if (player_kind == White)
    board->white_player_bar.pawn_count++;
}

Board default_board() {
  Board board = empty_board();
  int default_board_positions[] = {0, 11, 16, 18};
  int default_board_pawn_counts[] = {2, 5, 3, 5};

  for (int i = 0; i < 4; i++) {
    set_pawns(&board, default_board_positions[i], White,
              default_board_pawn_counts[i]);
    set_pawns(&board, BOARD_SIZE - default_board_positions[i] - 1, Red,
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
               "12  11  10  09  08  07 |   | 06  05  04  03  02  01");
  mv_print_str(win_wrapper, CONTENT_Y_START + CONTENT_HEIGHT / 2,
               CONTENT_X_START,
               "---------------------- |BAR| ----------------------");
  mv_print_str(win_wrapper, CONTENT_Y_END, CONTENT_X_START,
               "13  14  15  16  17  18 |   | 19  20  21  22  23  24");

  for (int i = 0; i < BOARD_ROW_COUNT; i++) {
    mv_print_centered(win_wrapper, CONTENT_Y_START + i + 1, "|   |");
    mv_print_centered(win_wrapper, CONTENT_Y_END - i - 1, "|   |");
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
  print_pawns_on_bar(win_wrapper, &board->red_player_bar);
  print_pawns_on_bar(win_wrapper, &board->white_player_bar);
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

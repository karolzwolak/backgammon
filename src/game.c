#include "../headers/game.h"
#include "../headers/window.h"
#include "../headers/window_manager.h"
#include <ncurses.h>
#include <string.h>

typedef enum { None, White, Red } PlayerKind;

typedef struct {
  const char *name;
  PlayerKind player_kind;
} Player;

typedef struct {
  PlayerKind pawn_player_kind;
  int pawn_count;
} BoardPoint;

bool board_point_is_empty(BoardPoint *board_point) {
  return board_point->pawn_count > 0;
}

BoardPoint empty_board_point() {
  BoardPoint res = {None, 0};
  return res;
}

BoardPoint new_board_point(PlayerKind player_kind, int pawn_count) {
  BoardPoint res = {player_kind, pawn_count};
  return res;
}

void print_overflowing_pawns(WinWrapper *win_wrapper, BoardPoint *board_point,
                             int y, int x) {

  char out[3] = "";

  sprintf(out, "%02d", board_point->pawn_count - BOARD_ROW_COUNT);
  mv_print_str(win_wrapper, y, x, out);
}

void pawn_draw_char(char *out, PlayerKind player_kind) {
  if (player_kind == Red)
    sprintf(out, "R");
  else if (player_kind == White)
    sprintf(out, "W");
}

void print_board_point(WinWrapper *win_wrapper, BoardPoint *board_point,
                       int id) {

  if (board_point->pawn_count == 0 || board_point->pawn_player_kind == None)
    return;

  char out[2] = "";
  pawn_draw_char(out, board_point->pawn_player_kind);

  bool on_bottom = id >= HALF_BOARD;
  int y, x, move_by;

  if (on_bottom) {
    id = id % 12;
    y = CONTENT_Y_END - 1;
    x = CONTENT_X_START + id * BOARD_POINT_WIDTH;
    move_by = -1;

    if (id >= QUARTER_BOARD)
      x += BAR_HORIZONTAL_GAP;
  } else {
    y = CONTENT_Y_START + 1;
    x = CONTENT_X_END - id * BOARD_POINT_WIDTH;
    move_by = 1;

    if (id >= QUARTER_BOARD)
      x -= BAR_HORIZONTAL_GAP;
  }

  int draw_count = board_point->pawn_count;
  if (draw_count > BOARD_ROW_COUNT)
    draw_count = BOARD_ROW_COUNT - 1;
  for (int i = 0; i < draw_count; i++) {
    mv_print_str(win_wrapper, y, x, out);
    y += move_by;
  }

  if (x > BOARD_WIDTH / 2)
    x -= 1;
  if (board_point->pawn_count > BOARD_ROW_COUNT)
    print_overflowing_pawns(win_wrapper, board_point, y, x);
}

void print_pawns_on_bar(WinWrapper *win_wrapper, BoardPoint *board_point) {
  if (board_point->pawn_count == 0 || board_point->pawn_player_kind == None)
    return;

  char out[2] = "";
  pawn_draw_char(out, board_point->pawn_player_kind);

  int start_y, move_dir;
  if (board_point->pawn_player_kind == Red) {
    start_y = CONTENT_Y_START + 1;
    move_dir = 1;

  } else {
    start_y = CONTENT_Y_END - 1;
    move_dir = -1;
  }

  for (int i = 0; i < board_point->pawn_count; i++) {
    int col = i % 3;
    int row = i / 3;

    mv_print_str(win_wrapper, start_y + row * move_dir, BOARD_WIDTH / 2 + col,
                 out);
  }
}

typedef struct {
  BoardPoint board_points[BOARD_SIZE];
  BoardPoint white_player_bar;
  BoardPoint red_player_bar;
} Board;

Board empty_board() {
  BoardPoint white_player_bar = empty_board_point();
  BoardPoint red_player_bar = empty_board_point();
  white_player_bar.pawn_player_kind = White;
  red_player_bar.pawn_player_kind = Red;

  Board board = {{}, white_player_bar, red_player_bar};

  for (int i = 0; i < BOARD_SIZE; i++)
    board.board_points[i] = empty_board_point();

  return board;
}

void set_pawns(Board *board, int id, PlayerKind player_kind, int count) {
  board->board_points[id].pawn_player_kind = player_kind;
  board->board_points[id].pawn_count = count;
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

typedef struct {
  Board board;

  Player white_player;
  Player red_player;
} GameManager;

void print_board_ui(WinWrapper *win_wrapper) {
  WINDOW *win = win_wrapper->win;

  mv_print_str(win_wrapper, CONTENT_Y_START, CONTENT_X_START,
               "12  11  10  09  08  07 |   | 06  05  04  03  02  01");
  mv_print_str(win_wrapper, CONTENT_Y_START + BOARD_HEIGHT / 2, CONTENT_X_START,
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
    print_board_point(win_wrapper, &board->board_points[i], i);
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
  refresh_win(win_wrapper);
}

void game_loop(WinManager *win_manager) {
  Board board = default_board();

  while (true) {
    display_board(&board, &win_manager->content_win);
    wmove(win_manager->io_win.win, 2, 1);
    refresh_win(&win_manager->io_win);
    switch (win_char_input(&win_manager->io_win)) {
    case 'q':
      return;
    }
  }
}

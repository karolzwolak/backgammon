#include "../headers/game.h"
#include "../headers/window.h"
#include "../headers/window_manager.h"
#include <ncurses.h>

typedef enum { None, White, Red } CheckerKind;

typedef struct {
  const char *name;
  CheckerKind checker_kind;
} Player;

typedef struct {
  CheckerKind checker_kind;
  int checker_count;
} BoardPoint;

bool board_point_is_empty(BoardPoint *board_point) {
  return board_point->checker_count > 0;
}

BoardPoint empty_board_point() {
  BoardPoint res = {None, 0};
  return res;
}

BoardPoint new_board_point(CheckerKind checker_kind, int checker_count) {
  BoardPoint res = {checker_kind, checker_count};
  return res;
}

void print_overflowing_checkers(WinWrapper *win_wrapper,
                                BoardPoint *board_point, int y, int x) {

  char out[3] = "";

  sprintf(out, "%02d", board_point->checker_count - BOARD_ROW_COUNT);
  mv_print_str(win_wrapper, y, x, out);
}

void checker_draw_char(char *out, CheckerKind checker_kind) {
  if (checker_kind == Red)
    sprintf(out, "R");
  else if (checker_kind == White)
    sprintf(out, "W");
}

void print_board_point(WinWrapper *win_wrapper, BoardPoint *board_point,
                       int id) {

  if (board_point->checker_count == 0 || board_point->checker_kind == None)
    return;

  char out[2] = "";
  checker_draw_char(out, board_point->checker_kind);

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

  int draw_count = board_point->checker_count;
  if (draw_count > BOARD_ROW_COUNT)
    draw_count = BOARD_ROW_COUNT - 1;
  for (int i = 0; i < draw_count; i++) {
    mv_print_str(win_wrapper, y, x, out);
    y += move_by;
  }

  if (x > BOARD_WIDTH / 2)
    x -= 1;
  if (board_point->checker_count > BOARD_ROW_COUNT)
    print_overflowing_checkers(win_wrapper, board_point, y, x);
}

void print_checkers_on_bar(WinWrapper *win_wrapper, BoardPoint *board_point) {
  if (board_point->checker_count == 0 || board_point->checker_kind == None)
    return;

  char out[2] = "";
  checker_draw_char(out, board_point->checker_kind);

  int start_y, move_dir;
  if (board_point->checker_kind == Red) {
    start_y = CONTENT_Y_START + 1;
    move_dir = 1;

  } else {
    start_y = CONTENT_Y_END - 1;
    move_dir = -1;
  }

  for (int i = 0; i < board_point->checker_count; i++) {
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
  white_player_bar.checker_kind = White;
  red_player_bar.checker_kind = Red;

  Board board = {{}, white_player_bar, red_player_bar};

  for (int i = 0; i < BOARD_SIZE; i++)
    board.board_points[i] = empty_board_point();

  return board;
}

void set_checkers(Board *board, int id, CheckerKind checker_kind, int count) {
  board->board_points[id].checker_kind = checker_kind;
  board->board_points[id].checker_count = count;
}

void add_checker_to_bar(Board *board, CheckerKind checker_kind) {
  if (checker_kind == Red)
    board->red_player_bar.checker_count++;
  else if (checker_kind == White)
    board->white_player_bar.checker_count++;
}

Board default_board() {
  Board board = empty_board();
  int default_board_positions[] = {0, 11, 16, 18};
  int default_board_checker_counts[] = {2, 5, 3, 5};

  for (int i = 0; i < 4; i++) {
    set_checkers(&board, default_board_positions[i], White,
                 default_board_checker_counts[i]);
    set_checkers(&board, BOARD_SIZE - default_board_positions[i] - 1, Red,
                 default_board_checker_counts[i]);
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

void print_board_checkers(WinWrapper *win_wrapper, Board *board) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    print_board_point(win_wrapper, &board->board_points[i], i);
  }
}

void print_board(Board *board, WinWrapper *win_wrapper) {
  print_board_ui(win_wrapper);
  print_board_checkers(win_wrapper, board);
  print_checkers_on_bar(win_wrapper, &board->red_player_bar);
  print_checkers_on_bar(win_wrapper, &board->white_player_bar);
}

void display_board(Board *board, WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  print_board(board, win_wrapper);
  refresh_win(win_wrapper);
}

int input_int(WinWrapper *io_wrapper, const char *prompt) {
  print_centered_on_new_line(io_wrapper, prompt);
  int res = -1;
  wscanw(io_wrapper->win, "%d", &res);
  move_rel(io_wrapper, -1, 0);
  return res;
}

void clear_io_win(WinWrapper *io_wrapper) {
  clear_win(io_wrapper);
  refresh_win(io_wrapper);
}

void game_loop(WinManager *win_manager) {
  Board board = default_board();

  disable_cursor();

  clear_io_win(&win_manager->io_win);

  while (true) {
    display_board(&board, &win_manager->content_win);
    switch (win_char_input(&win_manager->io_win)) {
    case 'i':
      enable_cursor();
      int res = input_int(&win_manager->io_win, "move from:");
      while (res == -1) {
        clear_curr_line(&win_manager->io_win);
        res = input_int(&win_manager->io_win, "move from:");
      }
      clear_io_win(&win_manager->io_win);
      disable_cursor();
      break;
    case 'q':
      return;
    }
  }
}

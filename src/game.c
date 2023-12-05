#include "../headers/game.h"
#include "../headers/window.h"
#include "../headers/window_manager.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

#define WHITE_HOME_START 5
#define RED_HOME_START BOARD_SIZE - 5

#define WHITE_OUT_START -1
#define RED_OUT_START BOARD_SIZE

#define CHECKER_COUNT 15

typedef enum { None, White, Red } CheckerKind;

CheckerKind opposite_checker(CheckerKind checker_kind) {
  if (checker_kind == None)
    return None;
  if (checker_kind == White)
    return Red;
  return White;
}

typedef struct {
  int v1, v2;
  bool used1, used2;
  int doublet_times_used;
} DiceRoll;

int roll_dice() { return rand() % 6 + 1; }

DiceRoll new_dice_roll() {
  return (DiceRoll){roll_dice(), roll_dice(), false, false, 0};
}

bool can_use_roll_val(DiceRoll *dice_roll, int val) {
  if (dice_roll->v1 == dice_roll->v2) {
    return val == dice_roll->v1 && dice_roll->doublet_times_used < 4;
  }
  if (val == dice_roll->v1) {
    return dice_roll->used1;
  }
  if (val == dice_roll->v2) {
    return dice_roll->used2;
  }
  return false;
}

void use_roll_val(DiceRoll *dice_roll, int val) {
  if (dice_roll->v1 == dice_roll->v2 && val == dice_roll->v1) {
    dice_roll->doublet_times_used++;
  }
  if (val == dice_roll->v1) {
    dice_roll->used1 = true;
  }
  if (val == dice_roll->v2) {
    dice_roll->used2 = true;
  }
}

bool dice_roll_used(DiceRoll *dice_roll) {
  return (dice_roll->v1 == dice_roll->v2 &&
          dice_roll->doublet_times_used >= 4) ||
         (dice_roll->used1 && dice_roll->used2);
}

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
  BoardPoint white_bar;
  BoardPoint red_bar;

  int white_out_count;
  int red_out_count;
} Board;

Board empty_board() {
  BoardPoint white_bar = empty_board_point();
  BoardPoint red_bar = empty_board_point();

  white_bar.checker_kind = White;
  red_bar.checker_kind = Red;

  Board board = {{}, white_bar, red_bar, 0, 0};

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
    board->red_bar.checker_count++;
  else if (checker_kind == White)
    board->white_bar.checker_count++;
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

bool can_player_bear_off(Board *board, CheckerKind checker_kind) {
  int sum = 0;
  if (checker_kind == White) {
    sum += board->white_out_count;
    for (int i = WHITE_HOME_START; i > WHITE_OUT_START; i--) {
      if (board->board_points[i].checker_kind != checker_kind)
        continue;
      sum += board->board_points[i].checker_count;
    }
  } else {
    sum += board->red_out_count;
    for (int i = RED_HOME_START; i < RED_OUT_START; i++) {
      if (board->board_points[i].checker_kind != checker_kind)
        continue;
      sum += board->board_points[i].checker_count;
    }
  }

  return sum == CHECKER_COUNT;
}

bool can_player_move_to_point(Board *board, CheckerKind checker_kind, int pos) {

  return board->board_points[pos].checker_kind == checker_kind ||
         board->board_points[pos].checker_count <= 1;
}

// just check if moving checker is legal, doesnt check for checkers on bar,
// and whether player has dice enough to play that move
bool is_move_legal(Board *board, CheckerKind checker_kind, int from, int dest) {
  if (from == dest || checker_kind == None ||
      board->board_points[from].checker_count == 0 ||
      board->board_points[from].checker_kind != checker_kind)
    return false;

  if (dest <= WHITE_OUT_START || dest >= RED_OUT_START) {
    if ((dest <= WHITE_OUT_START && checker_kind != White) ||
        (dest >= RED_OUT_START && checker_kind != Red))
      return false;
    return can_player_bear_off(board, checker_kind);
  }
  return can_player_move_to_point(board, checker_kind, dest);
}

int enter_pos(CheckerKind checker_kind, int move_by) {
  if (checker_kind == White) {
    return RED_OUT_START - move_by;
  } else if (checker_kind == Red) {
    return WHITE_OUT_START + move_by;
  }
  return -1;
}
bool is_enter_legal(Board *board, CheckerKind checker_kind, int move_by) {
  if (checker_kind == None || move_by > QUARTER_BOARD || move_by <= 0)
    return false;

  int pos = enter_pos(checker_kind, move_by);
  return can_player_move_to_point(board, checker_kind, pos);
}

void decrement_point(Board *board, int pos) {
  if (board->board_points[pos].checker_count == 0)
    return;
  board->board_points[pos].checker_count--;
  if (board->board_points[pos].checker_count == 0) {
    board->board_points[pos].checker_kind = None;
  }
}

void increment_point(Board *board, int pos, CheckerKind checker_kind) {
  board->board_points[pos].checker_count++;
  board->board_points[pos].checker_kind = checker_kind;
}

void add_to_bar(Board *board, CheckerKind checker_kind) {
  if (checker_kind == Red)
    board->red_bar.checker_count++;
  else if (checker_kind == White)
    board->white_bar.checker_count++;
}

// returns whether move was legal
bool player_move(Board *board, CheckerKind checker_kind, int from,
                 int move_by) {
  int dest = from;
  if (checker_kind == White) {
    dest -= move_by;
  } else if (checker_kind == Red) {
    dest += move_by;
  }

  if (!is_move_legal(board, checker_kind, from, dest))
    return false;

  decrement_point(board, from);

  if (dest <= WHITE_OUT_START || dest >= RED_OUT_START) {
    if (checker_kind == White)
      board->white_out_count++;
    else
      board->red_out_count++;
    return true;
  }

  if (board->board_points[dest].checker_kind != checker_kind &&
      board->board_points[dest].checker_count > 0) {
    add_to_bar(board, board->board_points[dest].checker_kind);
    decrement_point(board, dest);
  }
  increment_point(board, dest, checker_kind);

  return true;
}

bool player_enter(Board *board, CheckerKind checker_kind, int move_by) {
  if (!is_enter_legal(board, checker_kind, move_by))
    return false;

  if (checker_kind == White) {
    board->white_bar.checker_count--;
  } else {
    board->red_bar.checker_count--;
  }

  int pos = enter_pos(checker_kind, move_by);
  increment_point(board, pos, checker_kind);

  return true;
}

int legal_enters_count(Board *board, CheckerKind checker_kind,
                       DiceRoll *dice_roll) {
  if (checker_kind == None)
    return 0;

  int checker_count;
  if (checker_kind == White)
    checker_count = board->white_bar.checker_count;
  else
    checker_count = board->red_bar.checker_count;
  if (checker_count == 0)
    return 0;

  int count = 0;
  if (dice_roll->v1 == dice_roll->v2) {
    if (is_enter_legal(board, checker_kind, dice_roll->v1))
      count += 4;

  } else {
    if (is_enter_legal(board, checker_kind, dice_roll->v1))
      count++;
    if (is_enter_legal(board, checker_kind, dice_roll->v2))
      count++;
  }
  if (count > checker_count)
    count = checker_count;
  return count;
}

bool any_move_legal(Board *board, CheckerKind checker_kind,
                    DiceRoll *dice_roll) {
  if (checker_kind == None || dice_roll_used(dice_roll))
    return false;

  bool v1_used = dice_roll->used1;
  bool v2_used = dice_roll->used2;
  if (dice_roll->v1 == dice_roll->v2) {
    v1_used = dice_roll->doublet_times_used >= 4;
    v2_used = true;
  }
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (board->board_points[i].checker_kind != checker_kind)
      continue;
    if (!v1_used && is_move_legal(board, checker_kind, i, dice_roll->v1))
      return true;
    if (!v2_used && is_move_legal(board, checker_kind, i, dice_roll->v2))
      return true;
  }
  return false;
}

typedef struct {
  Board board;

  Player white_player;
  Player red_player;
  CheckerKind curr_player;

} GameManager;

GameManager new_game_manager(const char *white_name, const char *red_name) {
  Player white = {white_name, White};
  Player red = {red_name, Red};
  GameManager game_manager = {default_board(), white, red, None};
  return game_manager;
}

void print_board_ui(WinWrapper *win_wrapper) {
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
  print_checkers_on_bar(win_wrapper, &board->red_bar);
  print_checkers_on_bar(win_wrapper, &board->white_bar);
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

void clear_refresh_win(WinWrapper *win_wrapper) {
  clear_win(win_wrapper);
  refresh_win(win_wrapper);
}

void init_game(WinManager *win_manager, GameManager *game_manager) {
  char white_name[MAX_INPUT_LEN];
  char red_name[MAX_INPUT_LEN];

  prompt_input(&win_manager->io_win, "enter white name: ", white_name);
  prompt_input(&win_manager->io_win, "enter red name: ", red_name);
  clear_refresh_win(&win_manager->io_win);
  *game_manager = new_game_manager(white_name, red_name);
}

bool move_input(WinManager *win_manager, int *from, int *by) {
  bool quit =
      int_prompt_input_untill(&win_manager->io_win, "Move from: ", from);
  if (quit)
    return true;
  (*from)--;
  quit = int_prompt_input_untill(&win_manager->io_win, "Move by: ", by);
  return quit;
}

bool make_move(WinManager *win_manager, GameManager *game_manager) {
  int by = -1, from = -1;
  bool quit = move_input(win_manager, &from, &by);
  if (quit)
    return true;

  // TODO: make the actual move
  bool legal =
      player_move(&game_manager->board, game_manager->curr_player, from, by);
  if (legal)
    wprintw(win_manager->io_win.win, " made move %d by %d", from, by);
  else
    wprintw(win_manager->io_win.win, " illegal move %d by %d", from, by);

  win_char_input(&win_manager->io_win);
  clear_refresh_win(&win_manager->io_win);

  return false;
}

void game_loop(WinManager *win_manager) {
  disable_cursor();

  clear_refresh_win(&win_manager->io_win);

  GameManager game_manager = new_game_manager("white", "red");
  game_manager.curr_player = White;
  int field = 11;

  while (true) {
    display_board(&game_manager.board, &win_manager->content_win);
    switch (win_char_input(&win_manager->io_win)) {
    case 'i':
      game_manager.curr_player =
          game_manager.curr_player == White ? Red : White;
      break;
    case 'q':
      return;

    default:
      enable_cursor();
      make_move(win_manager, &game_manager);
      disable_cursor();
      break;
    }
  }
}

#include "../headers/game.h"
#include "../headers/window.h"
#include "../headers/window_manager.h"
#include <ncurses.h>
#include <stdlib.h>

#define MAX_DOUBLET_USES 4

#define WHITE_HOME_START 5
#define RED_HOME_START BOARD_SIZE - 5

#define WHITE_OUT_START -1
#define RED_OUT_START BOARD_SIZE

#define CHECKER_COUNT 15

#define STATS_LINES_COUNT 3
#define STATS_GAP 3
#define STATS_TOP_BOT_MARGIN                                                   \
  (SIDE_WIN_HEIGHT - STATS_GAP - 2 * STATS_LINES_COUNT) / 2
#define STATS_WHITE_Y_START STATS_TOP_BOT_MARGIN
#define STATS_RED_Y_START                                                      \
  SIDE_WIN_HEIGHT - STATS_TOP_BOT_MARGIN - STATS_LINES_COUNT

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
    return val == dice_roll->v1 &&
           dice_roll->doublet_times_used < MAX_DOUBLET_USES;
  }
  if (val == dice_roll->v1) {
    return !dice_roll->used1;
  }
  if (val == dice_roll->v2) {
    return !dice_roll->used2;
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
          dice_roll->doublet_times_used >= MAX_DOUBLET_USES) ||
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
  mv_printf_yx(win_wrapper, y, x, "%02d",
               board_point->checker_count - BOARD_ROW_COUNT);
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
    mv_printf_yx(win_wrapper, y, x, out);
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

    mv_printf_yx(win_wrapper, start_y + row * move_dir, BOARD_WIDTH / 2 + col,
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

  for (int i = 0; i < MAX_DOUBLET_USES; i++) {
    set_checkers(&board, default_board_positions[i], White,
                 default_board_checker_counts[i]);
    set_checkers(&board, BOARD_SIZE - default_board_positions[i] - 1, Red,
                 default_board_checker_counts[i]);
  }
  return board;
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

typedef struct {
  Board board;

  Player white_player;
  Player red_player;

  CheckerKind curr_player;
  DiceRoll dice_roll;

} GameManager;

GameManager new_game_manager(const char *white_name, const char *red_name) {
  Player white = {white_name, White};
  Player red = {red_name, Red};

  CheckerKind curr_player = White;
  DiceRoll dice_roll = new_dice_roll();
  while (dice_roll.v1 == dice_roll.v2) {
    dice_roll = new_dice_roll();
  }
  if (dice_roll.v1 < dice_roll.v2) {
    curr_player = Red;
  }

  GameManager game_manager = {default_board(), white, red, curr_player,
                              dice_roll};
  return game_manager;
}

void swap_players(GameManager *game_manager) {
  game_manager->dice_roll = new_dice_roll();
  if (game_manager->curr_player == White) {
    game_manager->curr_player = Red;
  } else {
    game_manager->curr_player = White;
  }
}

bool can_player_bear_off(GameManager *game_manager) {
  int sum = 0;
  Board *board = &game_manager->board;
  if (game_manager->curr_player == White) {
    sum += board->white_out_count;
    for (int i = WHITE_HOME_START; i > WHITE_OUT_START; i--) {
      if (board->board_points[i].checker_kind != game_manager->curr_player)
        continue;
      sum += board->board_points[i].checker_count;
    }
  } else {
    sum += board->red_out_count;
    for (int i = RED_HOME_START; i < RED_OUT_START; i++) {
      if (board->board_points[i].checker_kind != game_manager->curr_player)
        continue;
      sum += board->board_points[i].checker_count;
    }
  }

  return sum == CHECKER_COUNT;
}

bool can_player_move_to_point(GameManager *game_manager, int pos) {
  return game_manager->board.board_points[pos].checker_kind ==
             game_manager->curr_player ||
         game_manager->board.board_points[pos].checker_count <= 1;
}

// just check if moving checker is legal, doesnt check for checkers on bar,
// and whether player has dice enough to play that move
bool is_move_legal(GameManager *game_manager, int from, int dest) {
  Board *board = &game_manager->board;
  if (from == dest || game_manager->curr_player == None ||
      board->board_points[from].checker_count == 0 ||
      board->board_points[from].checker_kind != game_manager->curr_player)
    return false;

  if (dest <= WHITE_OUT_START || dest >= RED_OUT_START) {
    if ((dest <= WHITE_OUT_START && game_manager->curr_player != White) ||
        (dest >= RED_OUT_START && game_manager->curr_player != Red))
      return false;
    return can_player_bear_off(game_manager);
  }
  return can_player_move_to_point(game_manager, dest);
}

int enter_pos(CheckerKind checker_kind, int move_by) {
  if (checker_kind == White) {
    return RED_OUT_START - move_by;
  } else if (checker_kind == Red) {
    return WHITE_OUT_START + move_by;
  }
  return -1;
}

bool is_enter_legal(GameManager *game_manager, int move_by) {
  if (game_manager->curr_player == None || move_by > QUARTER_BOARD ||
      move_by <= 0)
    return false;

  int pos = enter_pos(game_manager->curr_player, move_by);
  return can_player_move_to_point(game_manager, pos);
}

// returns whether move was legal
bool player_move(GameManager *game_manager, int from, int move_by) {
  Board *board = &game_manager->board;
  int dest = from;

  if (game_manager->curr_player == White) {
    dest -= move_by;
  } else if (game_manager->curr_player == Red) {
    dest += move_by;
  }

  if (!is_move_legal(game_manager, from, dest))
    return false;

  decrement_point(board, from);

  if (dest <= WHITE_OUT_START || dest >= RED_OUT_START) {
    if (game_manager->curr_player == White)
      board->white_out_count++;
    else
      board->red_out_count++;
    return true;
  }

  if (board->board_points[dest].checker_kind != game_manager->curr_player &&
      board->board_points[dest].checker_count > 0) {
    add_to_bar(board, board->board_points[dest].checker_kind);
    decrement_point(board, dest);
  }
  increment_point(board, dest, game_manager->curr_player);

  return true;
}

bool player_enter(GameManager *game_manager, int move_by) {
  if (!is_enter_legal(game_manager, move_by))
    return false;
  Board *board = &game_manager->board;

  if (game_manager->curr_player == White) {
    board->white_bar.checker_count--;
  } else {
    board->red_bar.checker_count--;
  }

  int pos = enter_pos(game_manager->curr_player, move_by);
  increment_point(board, pos, game_manager->curr_player);

  return true;
}

int legal_enters_count(GameManager *game_manager) {
  if (game_manager->curr_player == None)
    return 0;

  int checker_count;
  if (game_manager->curr_player == White)
    checker_count = game_manager->board.white_bar.checker_count;
  else
    checker_count = game_manager->board.red_bar.checker_count;
  if (checker_count == 0)
    return 0;

  int count = 0;
  int v1 = game_manager->dice_roll.v1;
  int v2 = game_manager->dice_roll.v2;
  if (v1 == v2) {
    if (is_enter_legal(game_manager, v1))
      count += MAX_DOUBLET_USES;
  } else {
    if (is_enter_legal(game_manager, v1))
      count++;
    if (is_enter_legal(game_manager, v2))
      count++;
  }
  if (count > checker_count)
    count = checker_count;
  return count;
}

bool any_move_legal(GameManager *game_manager) {
  if (game_manager->curr_player == None ||
      dice_roll_used(&game_manager->dice_roll))
    return false;

  int v1 = game_manager->dice_roll.v1;
  int v2 = game_manager->dice_roll.v2;
  bool v1_used = game_manager->dice_roll.used1;
  bool v2_used = game_manager->dice_roll.used2;

  if (v1 == v2) {
    v1_used = game_manager->dice_roll.doublet_times_used >= MAX_DOUBLET_USES;
    v2_used = true;
  }
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (game_manager->board.board_points[i].checker_kind !=
        game_manager->curr_player)
      continue;
    if (!v1_used && is_move_legal(game_manager, i, v1))
      return true;
    if (!v2_used && is_move_legal(game_manager, i, v2))
      return true;
  }
  return false;
}

void print_board_ui(WinWrapper *win_wrapper) {
  mv_printf_yx(win_wrapper, CONTENT_Y_START, CONTENT_X_START,
               "12  11  10  09  08  07 |   | 06  05  04  03  02  01");
  mv_printf_yx(win_wrapper, CONTENT_Y_START + BOARD_HEIGHT / 2, CONTENT_X_START,
               "---------------------- |BAR| ----------------------");
  mv_printf_yx(win_wrapper, CONTENT_Y_END, CONTENT_X_START,
               "13  14  15  16  17  18 |   | 19  20  21  22  23  24");

  for (int i = 0; i < BOARD_ROW_COUNT; i++) {
    mv_printf_centered(win_wrapper, CONTENT_Y_START + i + 1, "|   |");
    mv_printf_centered(win_wrapper, CONTENT_Y_END - i - 1, "|   |");
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

void display_board(WinWrapper *win_wrapper, Board *board) {
  clear_win(win_wrapper);
  print_board(board, win_wrapper);
  refresh_win(win_wrapper);
}

void print_stats(WinWrapper *win_wrapper, GameManager *game_manager) {
  mv_printf_centered(win_wrapper, STATS_WHITE_Y_START, "White");
  printf_centered_on_new_line(win_wrapper, "out: %02d",
                              game_manager->board.white_out_count);

  mv_printf_centered(win_wrapper, STATS_RED_Y_START, "Red");
  printf_centered_on_new_line(win_wrapper, "out: %02d",
                              game_manager->board.red_out_count);
  int y = 2;
  if (game_manager->curr_player == White)
    y += STATS_WHITE_Y_START;
  else
    y += STATS_RED_Y_START;
  int v1 = game_manager->dice_roll.v1;
  int v2 = game_manager->dice_roll.v2;

  mv_printf_centered(win_wrapper, y, "roll: ");
  if (v1 == v2) {
    for (int i = game_manager->dice_roll.doublet_times_used;
         i < MAX_DOUBLET_USES; i++)
      win_printf(win_wrapper, " %d", v1);
    for (int i = 0; i < game_manager->dice_roll.doublet_times_used; i++)
      win_printf(win_wrapper, " _");
  } else {
    if (game_manager->dice_roll.used1) {
      win_printf(win_wrapper, " %d _", v2);
      return;
    }
    win_printf(win_wrapper, " %d", v1);
    if (!game_manager->dice_roll.used2)
      win_printf(win_wrapper, " %d", v2);
    else
      win_printf(win_wrapper, " _");
  }
}

void display_stats(WinWrapper *win_wrapper, GameManager *game_manager) {
  clear_win(win_wrapper);
  print_stats(win_wrapper, game_manager);
  refresh_win(win_wrapper);
}

void display_game(WinManager *win_manager, GameManager *game_manager) {
  display_board(&win_manager->content_win, &game_manager->board);
  display_stats(&win_manager->stats_win, game_manager);
}

int input_int(WinWrapper *io_wrapper, const char *prompt) {
  printf_centered_on_new_line(io_wrapper, "%s", prompt);
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

bool make_move_loop(WinManager *win_manager, GameManager *game_manager) {
  bool legal = false;
  int by, from;
  while (!legal) {
    by = -1;
    from = -1;
    bool quit = move_input(win_manager, &from, &by);
    if (quit)
      return true;

    bool can_use = can_use_roll_val(&game_manager->dice_roll, by);
    bool was_moved = player_move(game_manager, from, by);
    legal = can_use && was_moved;
    // legal = can_use_roll_val(&game_manager->dice_roll, by) &&
    //         player_move(game_manager, from, by);

    clear_refresh_win(&win_manager->io_win);
    win_printf(&win_manager->io_win, "cant from %d by %d %d %d", from, by,
               can_use, was_moved);
  }
  use_roll_val(&game_manager->dice_roll, by);

  return false;
}

bool make_enter_move_loop(WinManager *win_manager, GameManager *game_manager) {
  bool legal = false;
  int val;
  while (!legal) {
    val = -1;
    bool quit = int_prompt_input_untill(&win_manager->io_win,
                                        "Enter using value: ", &val);
    if (quit)
      return true;

    legal = can_use_roll_val(&game_manager->dice_roll, val) &&
            player_enter(game_manager, val);

    clear_refresh_win(&win_manager->io_win);
  }
  use_roll_val(&game_manager->dice_roll, val);

  return false;
}

bool play_turn(WinManager *win_manager, GameManager *game_manager) {
  int enter_moves_count = legal_enters_count(game_manager);
  for (int i = 0; i < enter_moves_count; i++) {
    if (make_enter_move_loop(win_manager, game_manager))
      return true;
    display_game(win_manager, game_manager);
  }

  while (!dice_roll_used(&game_manager->dice_roll) &&
         any_move_legal(game_manager)) {

    if (make_move_loop(win_manager, game_manager))
      return true;

    win_printf(&win_manager->io_win, "moved");
    display_game(win_manager, game_manager);
  }

  clear_refresh_win(&win_manager->io_win);
  swap_players(game_manager);

  return false;
}

void game_loop(WinManager *win_manager) {
  disable_cursor();

  clear_refresh_win(&win_manager->io_win);

  GameManager game_manager = new_game_manager("white", "red");

  enable_cursor();
  while (true) {
    display_game(win_manager, &game_manager);
    if (play_turn(win_manager, &game_manager)) {
      clear_refresh_win(&win_manager->io_win);
      disable_cursor();
      return;
    }
    // switch (win_char_input(&win_manager->io_win)) {
    // case 'i':
    //   game_manager.curr_player =
    //       game_manager.curr_player == White ? Red : White;
    //   break;
    // case 'q':
    //   return;
    //
    // default:
    //   enable_cursor();
    //   make_move_loop(win_manager, &game_manager);
    //   disable_cursor();
    //   break;
    // }
  }
}

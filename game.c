#include "game.h"
#include "window.h"
#include "window_manager.h"

#include "vec.c"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILE_HEADER "BOARD"
#define TURN_LOG_HEADER "HISTORY"

#define MAX_DOUBLET_USES 4

#define WHITE_HOME_START BOARD_SIZE - 5
#define RED_HOME_START 5
#define home_start(player) player == White ? WHITE_HOME_START : RED_HOME_START

#define WHITE_OUT_START BOARD_SIZE
#define RED_OUT_START -1
#define out_start(player) player == White ? WHITE_OUT_START : RED_OUT_START

#define WHITE_BAR_POS -8
#define RED_BAR_POS -7
#define PLAYER_BAR_POS(player) player == White ? WHITE_BAR_POS : RED_BAR_POS
#define ENEMY_BAR_POS(player) player == White ? RED_BAR_POS : WHITE_BAR_POS

#define WHITE_DIR 1
#define RED_DIR -1
#define CHECKER_DIR(checker_kind) checker_kind == White ? WHITE_DIR : RED_DIR

#define CHECKER_COUNT 15
#define BAR_WIDTH 3

#define MAX_FILENAME_LEN 100

#define STATS_LINES_COUNT 3
#define STATS_GAP 3
#define STATS_TOP_BOT_MARGIN                                                   \
  (SIDE_WIN_HEIGHT - STATS_GAP - 2 * STATS_LINES_COUNT) / 2

#define STATS_WHITE_Y SIDE_WIN_HEIGHT - STATS_TOP_BOT_MARGIN - STATS_LINES_COUNT
#define STATS_RED_Y STATS_TOP_BOT_MARGIN

#define STATS_ROLL_X 4
#define STATS_ROLL_DOUBLET_X 1

#define WHITE_CHECKER_CHAR 'W'
#define RED_CHECKER_CHAR 'R'

#define checker_char(checker_kind)                                             \
  checker_kind == White ? WHITE_CHECKER_CHAR : RED_CHECKER_CHAR

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

DiceRoll new_dice_roll(int v1, int v2) {
  return (DiceRoll){v1, v2, false, false, 0};
}
DiceRoll new_random_roll() { return new_dice_roll(roll_dice(), roll_dice()); }

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
    return;
  }
  if (val == dice_roll->v1) {
    dice_roll->used1 = true;
  }
  if (val == dice_roll->v2) {
    dice_roll->used2 = true;
  }
}

void reverse_use_roll_val(DiceRoll *dice_roll, int val) {
  if (dice_roll->v1 == dice_roll->v2 && val == dice_roll->v1) {
    dice_roll->doublet_times_used--;
    return;
  }
  if (val == dice_roll->v1) {
    dice_roll->used1 = false;
  }
  if (val == dice_roll->v2) {
    dice_roll->used2 = false;
  }
}

bool dice_roll_used(DiceRoll *dice_roll) {
  return (dice_roll->v1 == dice_roll->v2 &&
          dice_roll->doublet_times_used >= MAX_DOUBLET_USES) ||
         (dice_roll->used1 && dice_roll->used2);
}

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

void get_yx_for_print_point(int id, int *y, int *x_out, int *move_by_out) {
  bool on_bottom = id >= HALF_BOARD;

  if (on_bottom) {
    id = id % 12;
    *y = CONTENT_Y_END - 1;
    *x_out = CONTENT_X_START + id * BOARD_POINT_WIDTH;
    *move_by_out = -1;

    if (id >= QUARTER_BOARD)
      *x_out += BAR_HORIZONTAL_GAP;
  } else {
    *y = CONTENT_Y_START + 1;
    *x_out = CONTENT_X_END - id * BOARD_POINT_WIDTH;
    *move_by_out = 1;

    if (id >= QUARTER_BOARD)
      *x_out -= BAR_HORIZONTAL_GAP;
  }
}

void print_point_checkers(WinWrapper *win_wrapper, BoardPoint *board_point,
                          int *y, int x, int move_by) {

  char out = checker_char(board_point->checker_kind);
  int draw_count = board_point->checker_count;
  if (draw_count > BOARD_ROW_COUNT)
    draw_count = BOARD_ROW_COUNT - 1;

  for (int i = 0; i < draw_count; i++) {
    mv_printf_yx(win_wrapper, *y, x, "%c", out);
    *y += move_by;
  }
}

void print_board_point(WinWrapper *win_wrapper, BoardPoint *board_point,
                       int id) {

  if (board_point->checker_count == 0 || board_point->checker_kind == None)
    return;

  int x, y, move_by;
  get_yx_for_print_point(id, &y, &x, &move_by);

  print_point_checkers(win_wrapper, board_point, &y, x, move_by);

  if (x > BOARD_WIDTH / 2)
    x -= 1;
  if (board_point->checker_count > BOARD_ROW_COUNT)
    print_overflowing_checkers(win_wrapper, board_point, y, x);
}

void print_checkers_on_bar(WinWrapper *win_wrapper, BoardPoint *board_point) {
  if (board_point->checker_count == 0 || board_point->checker_kind == None)
    return;

  char out = checker_char(board_point->checker_kind);

  int start_y, move_dir = CHECKER_DIR(board_point->checker_kind);

  if (board_point->checker_kind == White) {
    start_y = CONTENT_Y_START + 1;
  } else {
    start_y = CONTENT_Y_END - 1;
  }

  for (int i = 0; i < board_point->checker_count; i++) {
    int col = i % BAR_WIDTH;
    int row = i / BAR_WIDTH;

    mv_printf_yx(win_wrapper, start_y + row * move_dir, BOARD_WIDTH / 2 + col,
                 "%c", out);
  }
}

typedef struct {
  int from, by, hit_enemy;
} MoveEntry;

void serialize_move_entry(MoveEntry *move_entry, FILE *fp) {
  fprintf(fp, "move f:%d b:%d h:%d\n", move_entry->from, move_entry->by,
          move_entry->hit_enemy);
}

bool deserialize_move_entry(MoveEntry *move_entry, FILE *fp) {
  int scanned = fscanf(fp, "move f:%d b:%d h:%d\n", &move_entry->from,
                       &move_entry->by, &move_entry->hit_enemy);
  return scanned == 3;
}

typedef struct {
  int dice1, dice2;
  MoveEntry moves[MAX_DOUBLET_USES];
  int move_count;
} TurnEntry;

void serialize_turn_entry(TurnEntry *turn_entry, FILE *fp) {
  fprintf(fp, "turn dice:%d dice:%d move_count:%d\n", turn_entry->dice1,
          turn_entry->dice2, turn_entry->move_count);
  for (int i = 0; i < turn_entry->move_count; i++) {
    serialize_move_entry(&turn_entry->moves[i], fp);
  }
}

bool deserialize_turn_entry(TurnEntry *turn_entry, FILE *fp) {
  int scanned =
      fscanf(fp, "turn dice:%d dice:%d move_count:%d\n", &turn_entry->dice1,
             &turn_entry->dice2, &turn_entry->move_count);
  if (scanned < 3 || turn_entry->move_count > MAX_DOUBLET_USES)
    return false;
  for (int i = 0; i < turn_entry->move_count; i++) {
    if (!deserialize_move_entry(&turn_entry->moves[i], fp))
      return false;
  }
  return true;
}

void turn_entry_new(TurnEntry *turn_entry, DiceRoll *dice_roll) {
  *turn_entry = (TurnEntry){dice_roll->v1, dice_roll->v2, {}, 0};
}

void turn_entry_add_move(TurnEntry *turn_entry, int from, int by,
                         bool hit_enemy) {
  turn_entry->moves[turn_entry->move_count] = (MoveEntry){from, by, hit_enemy};
  turn_entry->move_count++;
}

typedef struct {
  Vec vec;
  int trav_turn_id, trav_move_id;
} TurnLog;

void new_turn_log(TurnLog *turn_log_out, int cap) {
  Vec vec;
  if (cap <= 0) {
    vec_new(&vec, sizeof(TurnEntry));
  } else {
    vec_with_cap(&vec, sizeof(TurnEntry), cap);
  }
  if (vec.data == NULL) {
    exit(NO_HEAP_MEM_EXIT);
  }
  *turn_log_out = (TurnLog){vec, 0, -1};
}

void push_to_turn_log(TurnLog *turn_log, TurnEntry *turn_entry) {
  if (turn_log->vec.len + 1 > turn_log->vec.cap) {
    if (vec_extend(&turn_log->vec) == 1) {
      exit(NO_HEAP_MEM_EXIT);
      return;
    }
  }

  TurnEntry *data = turn_log->vec.data;
  data[turn_log->vec.len] = *turn_entry;
  turn_log->vec.len++;
}

TurnEntry *turn_at(TurnLog *turn_log, int id) {
  if (id < 0 || id >= turn_log->vec.len)
    return NULL;
  TurnEntry *data = turn_log->vec.data;
  return &data[id];
}

TurnEntry *turn_log_last_turn(TurnLog *turn_log) {
  int id = turn_log->vec.len - 1;
  return turn_at(turn_log, id);
}

bool trav_on_end(TurnLog *turn_log) {
  return turn_log->trav_turn_id + 1 >= turn_log->vec.len &&
         turn_log->trav_move_id + 1 >=
             turn_at(turn_log, turn_log->trav_turn_id)->move_count;
}

bool trav_on_start(TurnLog *turn_log) {
  return turn_log->trav_turn_id == 0 && turn_log->trav_move_id <= -1;
}

bool trav_on_first(TurnLog *turn_log) {
  return turn_log->trav_turn_id == 0 && turn_log->trav_move_id == 0;
}

bool trav_on_new_turn(TurnLog *turn_log) {
  return turn_log->trav_move_id == -1;
}

bool trav_next_move(TurnLog *turn_log) {
  if (trav_on_end(turn_log))
    return false;

  turn_log->trav_move_id++;

  TurnEntry *curr_turn = turn_at(turn_log, turn_log->trav_turn_id);
  bool new_turn = turn_log->trav_move_id >= curr_turn->move_count;

  if (new_turn) {
    turn_log->trav_turn_id++;
    turn_log->trav_move_id = -1;
  }
  return new_turn;
}

bool trav_prev_move(TurnLog *turn_log) {
  if (trav_on_start(turn_log))
    return false;

  turn_log->trav_move_id--;
  bool new_turn = turn_log->trav_move_id == -2 && !trav_on_start(turn_log);

  if (new_turn) {
    turn_log->trav_turn_id--;
    TurnEntry *turn_entry = turn_at(turn_log, turn_log->trav_turn_id);
    turn_log->trav_move_id = turn_entry->move_count - 1;
  }

  return new_turn;
}

MoveEntry *trav_curr_move(TurnLog *turn_log) {
  TurnEntry *turn_entry = turn_at(turn_log, turn_log->trav_turn_id);
  return &turn_entry->moves[turn_log->trav_move_id];
}

void trav_goto_end(TurnLog *turn_log) {
  if (turn_log->vec.len > 0)
    turn_log->trav_turn_id = turn_log->vec.len - 1;
  turn_log->trav_move_id =
      turn_at(turn_log, turn_log->trav_turn_id)->move_count;
  if (turn_log->trav_move_id >= 0)
    turn_log->trav_move_id--;
}

void trav_goto_start(TurnLog *turn_log) {
  turn_log->trav_turn_id = 0;
  turn_log->trav_move_id = -1;
}

void serialize_turn_log(TurnLog *turn_log, FILE *fp) {
  fprintf(fp, "\n%s len:%d\n", TURN_LOG_HEADER, turn_log->vec.len);
  for (int i = 0; i < turn_log->vec.len; i++) {
    serialize_turn_entry(turn_at(turn_log, i), fp);
  }
}

bool deserialize_turn_log(TurnLog *turn_log, FILE *fp) {
  int len;
  char header[MAX_INPUT_LEN];
  int scanned = fscanf(fp, "\n%s len:%d\n", header, &len);
  if (scanned < 2 || strcmp(header, TURN_LOG_HEADER) != 0)
    return false;
  new_turn_log(turn_log, len);
  turn_log->vec.len = len;

  for (int i = 0; i < len; i++) {
    if (!deserialize_turn_entry(turn_at(turn_log, i), fp))
      return false;
  }
  return true;
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
  int len =
      sizeof(default_board_positions) / sizeof(default_board_positions[0]);

  for (int i = 0; i < len; i++) {
    set_checkers(&board, default_board_positions[i], White,
                 default_board_checker_counts[i]);
    set_checkers(&board, BOARD_SIZE - default_board_positions[i] - 1, Red,
                 default_board_checker_counts[i]);
  }
  return board;
}

void add_to_point(Board *board, int pos, CheckerKind checker_kind,
                  int d_count) {
  BoardPoint *point = &board->board_points[pos];
  point->checker_count += d_count;

  if (point->checker_count == 0)
    point->checker_kind = None;
  else
    point->checker_kind = checker_kind;
}

void add_to_bar(Board *board, CheckerKind checker_kind, int d_count) {
  if (checker_kind == Red)
    board->red_bar.checker_count += d_count;
  else if (checker_kind == White)
    board->white_bar.checker_count += d_count;
}

void add_to_out(Board *board, CheckerKind checker_kind, int d_count) {
  if (checker_kind == Red)
    board->red_out_count += d_count;
  else if (checker_kind == White)
    board->white_out_count += d_count;
}

typedef struct {
  Board board;

  CheckerKind curr_player;
  DiceRoll dice_roll;

  TurnLog turn_log;
} GameManager;

GameManager new_game_manager() {
  CheckerKind curr_player = White;
  DiceRoll dice_roll;
  do {
    dice_roll = new_random_roll();
  } while (dice_roll.v1 == dice_roll.v2);

  if (dice_roll.v1 < dice_roll.v2) {
    curr_player = Red;
  }
  TurnLog turn_log;
  new_turn_log(&turn_log, 0);

  GameManager game_manager = {default_board(), curr_player, dice_roll,
                              turn_log};
  return game_manager;
}

void free_game_manager(GameManager *game_manager) {
  vec_free(&game_manager->turn_log.vec);
}

void game_add_move_entry(GameManager *game_manager, int from, int by,
                         bool hit_enemy) {
  TurnEntry *turn_entry = turn_log_last_turn(&game_manager->turn_log);
  if (turn_entry == NULL)
    exit(NO_HEAP_MEM_EXIT);

  turn_entry_add_move(turn_entry, from, by, hit_enemy);
}

bool serialize_board_points(Board *board, FILE *fp) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (board->board_points[i].checker_kind == None ||
        board->board_points[i].checker_count == 0)
      continue;
    if (!fprintf(fp, "point #%d %c %d\n", i,
                 checker_char(board->board_points[i].checker_kind),
                 board->board_points[i].checker_count))
      return false;
  }
  return true;
}

bool serialize_board(Board *board, FILE *fp) {
  if (!serialize_board_points(board, fp))
    return false;

  fprintf(fp, "\n");
  fprintf(fp, "bar %c %d\n", checker_char(board->white_bar.checker_kind),
          board->white_bar.checker_count);
  fprintf(fp, "bar %c %d\n", checker_char(board->red_bar.checker_kind),
          board->red_bar.checker_count);

  fprintf(fp, "\n");
  fprintf(fp, "out %c %d\n", checker_char(White), board->white_out_count);
  fprintf(fp, "out %c %d\n", checker_char(Red), board->red_out_count);
  fprintf(fp, "\n");
  return true;
}

void serialize_player_roll(GameManager *game_manager, FILE *fp) {
  DiceRoll *dice_roll = &game_manager->dice_roll;
  fprintf(fp, "player %c\n", checker_char(game_manager->curr_player));
  fprintf(fp, "roll v:%d v:%d u:%d u:%d d_u:%d\n", dice_roll->v1, dice_roll->v2,
          dice_roll->used1, dice_roll->used2, dice_roll->doublet_times_used);
}

bool serialize_game(GameManager *game_manager, char *filename) {
  FILE *fp = fopen(filename, "w");

  if (fp == NULL)
    return false;

  Board *board = &game_manager->board;
  fprintf(fp, "%s\n", FILE_HEADER);

  if (!serialize_board(board, fp))
    return false;

  serialize_player_roll(game_manager, fp);

  serialize_turn_log(&game_manager->turn_log, fp);

  fclose(fp);
  return true;
}

CheckerKind checker_kind_from_char(char c) {
  switch (c) {
  case WHITE_CHECKER_CHAR:
    return White;
  case RED_CHECKER_CHAR:
    return Red;
  default:
    return None;
  }
}

bool scan_board_points(Board *board, FILE *fp, int *white_count,
                       int *red_count) {
  int checker_count, id;
  char checker_char;
  CheckerKind checker_kind;

  while (1) {
    int scanned =
        fscanf(fp, "point #%d %c %d\n", &id, &checker_char, &checker_count);
    if (scanned < 3)
      break;

    if (checker_char == WHITE_CHECKER_CHAR) {
      checker_kind = White;
      *white_count += checker_count;
    } else if (checker_char == RED_CHECKER_CHAR) {
      checker_kind = Red;
      *red_count += checker_count;
    } else {
      return false;
    }

    board->board_points[id].checker_count = checker_count;
    board->board_points[id].checker_kind = checker_kind;
  }
  return true;
}

bool scan_board_bar(Board *board, FILE *fp, int *white_count, int *red_count) {
  int checker_count;
  char checker_char;

  if (fscanf(fp, "bar %c %d\n", &checker_char, &checker_count) < 2)
    return false;
  if (checker_char != WHITE_CHECKER_CHAR)
    return false;

  board->white_bar.checker_count = checker_count;
  *white_count += checker_count;

  if (fscanf(fp, "bar %c %d\n", &checker_char, &checker_count) < 2)
    return false;
  if (checker_char != RED_CHECKER_CHAR)
    return false;

  board->red_bar.checker_count = checker_count;
  *red_count += checker_count;

  return true;
}

bool scan_board_out(Board *board, FILE *fp, int *white_count, int *red_count) {
  int checker_count;
  char checker_char;

  if (fscanf(fp, "out %c %d\n", &checker_char, &checker_count) < 2)
    return false;
  if (checker_char != WHITE_CHECKER_CHAR)
    return false;
  board->white_out_count = checker_count;
  *white_count += checker_count;

  if (fscanf(fp, "out %c %d\n", &checker_char, &checker_count) < 2)
    return false;
  if (checker_char != RED_CHECKER_CHAR)
    return false;
  board->red_out_count = checker_count;
  *red_count += checker_count;

  return true;
}

bool scan_player_roll(GameManager *game_manager, FILE *fp) {
  char checker_char;
  if (fscanf(fp, "player %c\n", &checker_char) < 1)
    return false;

  game_manager->curr_player = checker_kind_from_char(checker_char);
  if (game_manager->curr_player == None)
    return false;

  int v1, v2, used1, used2, doublet_times_used;
  int scanned = fscanf(fp, "roll v:%d v:%d u:%d u:%d d_u:%d\n", &v1, &v2,
                       &used1, &used2, &doublet_times_used);
  if (scanned < 5)
    return false;

  game_manager->dice_roll =
      (DiceRoll){v1, v2, used1, used2, doublet_times_used};

  return true;
}

bool scan_game_board(GameManager *game_manager, FILE *fp) {
  Board board = empty_board();

  int white_count = 0, red_count = 0;
  char header[MAX_INPUT_LEN];
  if (fscanf(fp, "%s\n", header) == 0)
    return false;
  if (strcmp(header, FILE_HEADER) != 0)
    return false;

  if (!scan_board_points(&board, fp, &white_count, &red_count))
    return false;
  if (!scan_board_bar(&board, fp, &white_count, &red_count))
    return false;
  if (!scan_board_out(&board, fp, &white_count, &red_count))
    return false;
  if (!scan_player_roll(game_manager, fp))
    return false;
  game_manager->board = board;

  return white_count + red_count == 2 * CHECKER_COUNT;
}

bool deserialize_game(WinManager *win_manager, GameManager *out_game,
                      char *filename) {

  FILE *fp = fopen(filename, "r");
  refresh_win(&win_manager->io_win);
  if (fp == NULL) {
    printf_centered_nl(&win_manager->io_win, "Cannot access file '%s'",
                       filename);
    return false;
  }

  int success = scan_game_board(out_game, fp) &&
                deserialize_turn_log(&out_game->turn_log, fp);
  // new_turn_log(&out_game->turn_log, 0);

  fclose(fp);

  if (!success) {
    printf_centered_nl(&win_manager->io_win, "Wrong data in file '%s'",
                       filename);
  }

  return success;
}

void swap_players(GameManager *game_manager) {
  game_manager->dice_roll = new_random_roll();
  if (game_manager->curr_player == White) {
    game_manager->curr_player = Red;
  } else {
    game_manager->curr_player = White;
  }
}

bool is_pos_out(int pos) {
  return (pos > RED_BAR_POS && pos <= RED_OUT_START) || pos >= WHITE_OUT_START;
}

bool is_pos_on_bar(int pos) {
  return pos == WHITE_BAR_POS || pos == RED_BAR_POS;
}

CheckerKind checker_kind_at(Board *board, int pos) {
  if (pos == WHITE_BAR_POS || pos >= WHITE_OUT_START)
    return White;
  if (pos == RED_BAR_POS || pos <= RED_OUT_START)
    return Red;
  return board->board_points[pos].checker_kind;
}

int checker_move_by(CheckerKind checker_kind, int from, int by) {
  return from + by * (CHECKER_DIR(checker_kind));
}

int enter_dest(CheckerKind checker_kind, int move_by) {
  return checker_move_by(checker_kind,
                         out_start(opposite_checker(checker_kind)), move_by);
}

int move_dest_checker_kind(CheckerKind checker_kind, int from, int move_by) {
  if (from == WHITE_BAR_POS) {
    return enter_dest(White, move_by);
  } else if (from == RED_BAR_POS) {
    return enter_dest(Red, move_by);
  }
  return checker_move_by(checker_kind, from, move_by);
}

int move_dest(GameManager *game_manager, int from, int move_by) {
  return move_dest_checker_kind(checker_kind_at(&game_manager->board, from),
                                from, move_by);
}

int move_dest_rev(GameManager *game_manager, int from, int move_by) {
  return move_dest_checker_kind(game_manager->curr_player, from, move_by);
}

// returns true if there was definitive answer
bool check_fhit_pos(Board *board, CheckerKind curr_player, DiceRoll *dice_roll,
                    int pos) {

  CheckerKind enemy = opposite_checker(curr_player);
  if (checker_kind_at(board, pos) != enemy ||
      board->board_points[pos].checker_count > 1)
    return false;
  int from1 = pos - dice_roll->v1 * (CHECKER_DIR(curr_player));
  int from2 = pos - dice_roll->v2 * (CHECKER_DIR(curr_player));
  bool can_hit_from1 = can_use_roll_val(dice_roll, dice_roll->v1) &&
                       checker_kind_at(board, from1) == curr_player;
  bool can_hit_from2 = can_use_roll_val(dice_roll, dice_roll->v2) &&
                       checker_kind_at(board, from2) == curr_player;

  return can_hit_from1 || can_hit_from2;
}

int fhit_pos_red(GameManager *game_manager) {
  Board *board = &game_manager->board;
  DiceRoll *dice_roll = &game_manager->dice_roll;
  for (int i = out_start(White) - 1; i >= 0; i--) {
    if (!check_fhit_pos(board, Red, dice_roll, i))
      continue;
    return i;
  }
  return -1;
}

int fhit_pos_white(GameManager *game_manager) {
  Board *board = &game_manager->board;
  DiceRoll *dice_roll = &game_manager->dice_roll;
  for (int i = out_start(Red) + 1; i < BOARD_SIZE; i++) {
    if (!check_fhit_pos(board, White, dice_roll, i))
      continue;
    return i;
  }
  return -1;
}

int fhit_pos(GameManager *game_manager) {
  if (game_manager->curr_player == Red)
    return fhit_pos_red(game_manager);
  return fhit_pos_white(game_manager);
}

bool check_fhit_pos_enter(GameManager *game_manager, int d_val) {
  int pos = enter_dest(game_manager->curr_player, d_val);

  return checker_kind_at(&game_manager->board, pos) ==
             opposite_checker(game_manager->curr_player) &&
         game_manager->board.board_points[pos].checker_count == 1;
}

int fhit_pos_enter(GameManager *game_manager) {
  DiceRoll *dice_roll = &game_manager->dice_roll;
  int v1 = dice_roll->v1;
  int v2 = dice_roll->v2;

  bool can_hit_dice1 =
      can_use_roll_val(dice_roll, v1) && check_fhit_pos_enter(game_manager, v1);
  bool can_hit_dice2 =
      can_use_roll_val(dice_roll, v2) && check_fhit_pos_enter(game_manager, v2);

  if (can_hit_dice1)
    return enter_dest(game_manager->curr_player, v1);
  if (can_hit_dice2)
    return enter_dest(game_manager->curr_player, v2);
  return -1;
}

bool check_fhit(GameManager *game_manager, int dest) {
  int pos = fhit_pos(game_manager);
  return pos != -1 && pos == dest;
}

bool can_player_bear_off(GameManager *game_manager) {
  int sum = 0;
  Board *board = &game_manager->board;
  if (game_manager->curr_player == White) {
    sum += board->white_out_count;
    for (int i = WHITE_HOME_START; i < WHITE_OUT_START; i++) {
      if (board->board_points[i].checker_kind != game_manager->curr_player)
        continue;
      sum += board->board_points[i].checker_count;
    }
  } else {
    sum += board->red_out_count;
    for (int i = RED_HOME_START; i > RED_OUT_START; i--) {
      if (board->board_points[i].checker_kind != game_manager->curr_player)
        continue;
      sum += board->board_points[i].checker_count;
    }
  }

  return sum == CHECKER_COUNT;
}

// dont check if player can bear off at all
bool check_f_bear_off(GameManager *game_manager, int dest) {
  return ((dest >= WHITE_OUT_START && game_manager->curr_player == White) ||
          (dest <= RED_OUT_START && game_manager->curr_player == Red));
}

bool can_player_move_to_point(GameManager *game_manager, int dest) {
  return game_manager->board.board_points[dest].checker_kind ==
             game_manager->curr_player ||
         game_manager->board.board_points[dest].checker_count <= 1;
}

// check if move from point is legal
bool is_move_legal_basic(GameManager *game_manager, int from, int move_by) {
  Board *board = &game_manager->board;
  int dest = move_dest(game_manager, from, move_by);

  if (from < 0 || !can_use_roll_val(&game_manager->dice_roll, move_by) ||
      checker_kind_at(board, from) != game_manager->curr_player)
    return false;

  if (is_pos_out(dest)) {
    if ((dest >= WHITE_OUT_START && game_manager->curr_player != White) ||
        (dest <= RED_OUT_START && game_manager->curr_player != Red))
      return false;
    return can_player_bear_off(game_manager);
  }
  return can_player_move_to_point(game_manager, dest);
}

bool check_passes_forced(WinManager *win_manager, GameManager *game_manager,
                         int dest) {
  int fpos = fhit_pos(game_manager);
  bool hits_fhit = fpos == dest && dest != -1;
  bool passes_fhit = fpos == -1 || hits_fhit;

  bool bears_off = check_f_bear_off(game_manager, dest);
  bool passes_f_out = !can_player_bear_off(game_manager) || bears_off;

  if ((passes_fhit && passes_f_out) || hits_fhit || bears_off)
    return true;

  if (!passes_fhit)
    printf_centered_nl(&win_manager->io_win, "you have to hit #%d pos",
                       fpos + 1);
  if (!passes_f_out)
    printf_centered_nl(&win_manager->io_win, "you have to bear off");
  return false;
}

bool is_move_legal_forced(WinManager *win_manager, GameManager *game_manager,
                          int from, int move_by) {
  clear_win(&win_manager->io_win);

  int dest = move_dest(game_manager, from, move_by);
  if (!is_move_legal_basic(game_manager, from, move_by)) {
    printf_centered_nl(&win_manager->io_win, "illegal move");
    return false;
  }

  return check_passes_forced(win_manager, game_manager, dest);
}

bool is_enter_legal_basic(GameManager *game_manager, int move_by) {
  if (game_manager->curr_player == None || move_by <= 0)
    return false;

  int pos = enter_dest(game_manager->curr_player, move_by);
  return can_player_move_to_point(game_manager, pos);
}

bool is_enter_legal_forced(WinManager *win_manager, GameManager *game_manager,
                           int move_by) {
  clear_win(&win_manager->io_win);

  if (!is_enter_legal_basic(game_manager, move_by)) {
    printf_centered_nl(&win_manager->io_win, "illegal move");
    return false;
  }
  int dest = enter_dest(game_manager->curr_player, move_by);

  int fpos = fhit_pos_enter(game_manager);
  bool passes_fhit = fpos == -1 || fpos == dest;
  if (passes_fhit)
    return true;

  printf_centered_nl(&win_manager->io_win, "you have to hit #%d pos", fpos + 1);
  return false;
}

void move_checker(GameManager *game_manager, int from, int dest, bool reverse) {
  Board *board = &game_manager->board;

  if (reverse) {
    int temp = from;
    from = dest;
    dest = temp;
  }
  CheckerKind checker_kind = checker_kind_at(board, from);

  if (is_pos_on_bar(from)) {
    add_to_bar(board, checker_kind, -1);
  } else if (is_pos_out(from)) {
    add_to_out(board, checker_kind, -1);
  } else {
    add_to_point(board, from, checker_kind, -1);
  }

  if (is_pos_on_bar(dest)) {
    add_to_bar(board, checker_kind, 1);
  } else if (is_pos_out(dest)) {
    add_to_out(board, checker_kind, 1);
  } else {
    add_to_point(board, dest, checker_kind, 1);
  }
}

bool move_checker_check_hit(GameManager *game_manager, int from, int move_by) {
  Board *board = &game_manager->board;
  int dest = move_dest(game_manager, from, move_by);

  bool hit = false;
  if (!is_pos_out(dest) &&
      board->board_points[dest].checker_kind != game_manager->curr_player &&
      board->board_points[dest].checker_count > 0) {
    move_checker(game_manager, dest, ENEMY_BAR_POS(game_manager->curr_player),
                 false);
    hit = true;
  }
  move_checker(game_manager, from, dest, false);

  return hit;
}

void apply_move_entry(MoveEntry *move_entry, GameManager *game_manager,
                      bool reverse) {
  int from = move_entry->from;
  int by = move_entry->by;

  int dest = move_dest(game_manager, from, by);

  if (reverse) {
    dest = move_dest_rev(game_manager, from, by);
    move_checker(game_manager, from, dest, reverse);
    reverse_use_roll_val(&game_manager->dice_roll, by);
  }

  if (move_entry->hit_enemy) {
    move_checker(game_manager, dest, ENEMY_BAR_POS(game_manager->curr_player),
                 reverse);
  }

  if (!reverse) {
    move_checker(game_manager, from, dest, reverse);
    use_roll_val(&game_manager->dice_roll, by);
  }
}

void synch_prev_turn_dice(TurnEntry *turn_entry, GameManager *game_manager) {
  for (int i = 0; i < turn_entry->move_count; i++) {
    use_roll_val(&game_manager->dice_roll, turn_entry->moves[i].by);
  }
}

void apply_turn_entry(TurnEntry *turn_entry, GameManager *game_manager) {
  game_manager->curr_player = opposite_checker(game_manager->curr_player);
  game_manager->dice_roll = new_dice_roll(turn_entry->dice1, turn_entry->dice2);
}

void trav_apply_reverse_move(GameManager *game_manager, TurnLog *turn_log) {
  if (trav_on_start(&game_manager->turn_log))
    return;
  if (!trav_on_new_turn(turn_log)) {
    apply_move_entry(trav_curr_move(turn_log), game_manager, true);
    trav_prev_move(turn_log);
    return;
  }
  trav_prev_move(turn_log);
  TurnEntry *prev_turn = turn_at(turn_log, turn_log->trav_turn_id);
  apply_turn_entry(prev_turn, game_manager);
  synch_prev_turn_dice(prev_turn, game_manager);
}

void trav_apply_next_move(GameManager *game_manager, TurnLog *turn_log) {
  if (trav_on_end(&game_manager->turn_log))
    return;
  bool new_turn = trav_next_move(turn_log);
  if (new_turn) {
    apply_turn_entry(turn_at(turn_log, turn_log->trav_turn_id), game_manager);
    return;
  }
  apply_move_entry(trav_curr_move(turn_log), game_manager, false);
}

void trav_apply_move(GameManager *game_manager, bool reverse) {
  TurnLog *turn_log = &game_manager->turn_log;
  if (reverse)
    trav_apply_reverse_move(game_manager, turn_log);
  else
    trav_apply_next_move(game_manager, turn_log);
}

void trav_apply_to_start(GameManager *game_manager) {
  trav_goto_start(&game_manager->turn_log);

  game_manager->board = default_board();
  apply_turn_entry(turn_at(&game_manager->turn_log, 0), game_manager);

  if (game_manager->dice_roll.v1 < game_manager->dice_roll.v2) {
    game_manager->curr_player = Red;
  } else {
    game_manager->curr_player = White;
  }
}

void trav_apply_to_end(GameManager *game_manager, GameManager *end_game) {
  *game_manager = *end_game;
  trav_goto_end(&game_manager->turn_log);
}

void trav_delete_next_moves(TurnLog *turn_log) {
  turn_log->vec.len = turn_log->trav_turn_id + 1;
  TurnEntry *curr_turn = turn_at(turn_log, turn_log->trav_turn_id);
  curr_turn->move_count = turn_log->trav_move_id + 1;
}

// returns whether move was legal
bool player_move(WinManager *win_manager, GameManager *game_manager, int from,
                 int move_by) {
  if (!is_move_legal_forced(win_manager, game_manager, from, move_by))
    return false;

  int hit_enemy = move_checker_check_hit(game_manager, from, move_by);
  game_add_move_entry(game_manager, from, move_by, hit_enemy);

  return true;
}

bool player_enter(WinManager *win_manager, GameManager *game_manager,
                  int move_by) {
  if (!is_enter_legal_forced(win_manager, game_manager, move_by))
    return false;

  int from = PLAYER_BAR_POS(game_manager->curr_player);
  int hit_enemy = move_checker_check_hit(game_manager, from, move_by);
  game_add_move_entry(game_manager, from, move_by, hit_enemy);

  return true;
}

int bar_count(Board *board, CheckerKind checker_kind) {
  if (checker_kind == White)
    return board->white_bar.checker_count;
  if (checker_kind == Red)
    return board->red_bar.checker_count;
  return -1;
}

int legal_enters_count(GameManager *game_manager) {
  if (game_manager->curr_player == None)
    return 0;

  int checker_count =
      bar_count(&game_manager->board, game_manager->curr_player);

  int count = 0;
  int v1 = game_manager->dice_roll.v1;
  int v2 = game_manager->dice_roll.v2;
  if (v1 == v2) {
    if (is_enter_legal_basic(game_manager, v1))
      count = MAX_DOUBLET_USES;
  } else {
    if (is_enter_legal_basic(game_manager, v1))
      count++;
    if (is_enter_legal_basic(game_manager, v2))
      count++;
  }
  if (count > checker_count)
    count = checker_count;
  return count;
}

bool any_move_legal(GameManager *game_manager) {
  DiceRoll *dice_roll = &game_manager->dice_roll;
  CheckerKind curr_player = game_manager->curr_player;
  if (curr_player == None || dice_roll_used(dice_roll) ||
      bar_count(&game_manager->board, curr_player) > 0)
    return false;

  int v1 = dice_roll->v1;
  int v2 = dice_roll->v2;

  for (int i = 0; i < BOARD_SIZE; i++) {
    if (checker_kind_at(&game_manager->board, i) != curr_player)
      continue;
    if (is_move_legal_basic(game_manager, i, v1))
      return true;
    if (v1 != v2 && is_move_legal_basic(game_manager, i, v2))
      return true;
  }
  return false;
}

void print_board_ui(WinWrapper *win_wrapper) {
  mv_printf_yx(win_wrapper, CONTENT_Y_START, CONTENT_X_START,
               "12  11  10  09  08  07 |   | 06  05  04  03  02  01");
  mv_printf_yx(win_wrapper, CONTENT_Y_START + BOARD_HEIGHT / 2, CONTENT_X_START,
               "---------------------- |BAR| ----------------- HOME");
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

void print_dice_val(WinWrapper *win_wrapper, int v, bool used) {
  if (!used)
    win_printf(win_wrapper, " %d", v);
  else
    win_printf(win_wrapper, " _");
}

void print_stats_roll(WinWrapper *win_wrapper, GameManager *game_manager, int y,
                      int x) {
  int v1 = game_manager->dice_roll.v1;
  int v2 = game_manager->dice_roll.v2;
  int d_times = game_manager->dice_roll.doublet_times_used;

  mv_printf_yx(win_wrapper, y, x, " roll", v1);
  if (v1 == v2) {
    for (int i = 0; i < MAX_DOUBLET_USES; i++)
      print_dice_val(win_wrapper, v1, i + d_times >= MAX_DOUBLET_USES);
  } else {
    print_dice_val(win_wrapper, v1, game_manager->dice_roll.used1);
    print_dice_val(win_wrapper, v2, game_manager->dice_roll.used2);
  }
}

void print_stats_header(WinWrapper *win_wrapper, GameManager *game_manager) {
  mv_printf_centered(win_wrapper, STATS_WHITE_Y, "-> White");
  printf_centered_nl(win_wrapper, "out: %02d",
                     game_manager->board.white_out_count);

  mv_printf_centered(win_wrapper, STATS_RED_Y, "-> Red");
  printf_centered_nl(win_wrapper, "out: %02d",
                     game_manager->board.red_out_count);
}

void print_stats(WinWrapper *win_wrapper, GameManager *game_manager) {
  print_stats_header(win_wrapper, game_manager);

  int y = STATS_LINES_COUNT - 1;
  if (game_manager->curr_player == White)
    y += STATS_WHITE_Y;
  else
    y += STATS_RED_Y;

  int x = STATS_ROLL_X;

  if (game_manager->dice_roll.v1 == game_manager->dice_roll.v2) {
    x = STATS_ROLL_DOUBLET_X;
  }
  print_stats_roll(win_wrapper, game_manager, y, x);
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
  printf_centered_nl(io_wrapper, "%s", prompt);
  int res = -1;
  wscanw(io_wrapper->win, "%d", &res);
  move_rel(io_wrapper, -1, 0);
  return res;
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

    legal = player_move(win_manager, game_manager, from, by);

    refresh_win(&win_manager->io_win);
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
            player_enter(win_manager, game_manager, val);

    refresh_win(&win_manager->io_win);
  }
  use_roll_val(&game_manager->dice_roll, val);

  return false;
}

void log_new_turn(GameManager *game_manager) {
  TurnEntry turn_entry;
  turn_entry_new(&turn_entry, &game_manager->dice_roll);
  push_to_turn_log(&game_manager->turn_log, &turn_entry);
}

bool play_turn(WinManager *win_manager, GameManager *game_manager,
               bool resume) {
  if (!resume) {
    log_new_turn(game_manager);
  }

  int count = legal_enters_count(game_manager);
  for (int i = 0; i < count; i++) {
    if (make_enter_move_loop(win_manager, game_manager))
      return true;

    display_game(win_manager, game_manager);
  }

  while (any_move_legal(game_manager)) {
    if (make_move_loop(win_manager, game_manager))
      return true;

    display_game(win_manager, game_manager);
  }

  clear_refresh_win(&win_manager->io_win);
  swap_players(game_manager);

  return false;
}

CheckerKind check_game_over(GameManager *game_manager) {
  if (game_manager->board.white_out_count >= CHECKER_COUNT)
    return White;
  if (game_manager->board.red_out_count >= CHECKER_COUNT)
    return Red;
  return None;
}

bool check_handle_win(WinManager *win_manager, GameManager *game_manager) {
  CheckerKind won = check_game_over(game_manager);
  if (won == None)
    return false;

  clear_refresh_win(&win_manager->content_win);
  mv_printf_centered(&win_manager->content_win, CONTENT_Y_END / 2,
                     "Game Over!");

  if (won == White)
    printf_centered_nl(&win_manager->content_win, "White Wins!");
  else
    printf_centered_nl(&win_manager->content_win, "Red Wins!");

  win_char_input(&win_manager->io_win);
  return true;
}

void save_game(WinManager *win_manager, GameManager *game_manager) {
  char filename[MAX_INPUT_LEN] = "";
  prompt_input(&win_manager->io_win, "Save to file: ", filename);
  if (filename[0] == '\0')
    return;
  if (!serialize_game(game_manager, filename)) {
    printf_centered_nl(&win_manager->io_win, "Failed to access '%s'", filename);
  }
}

void game_loop(WinManager *win_manager, GameManager *game_manager,
               bool resume) {
  enable_cursor();
  clear_refresh_win(&win_manager->io_win);

  bool save = false;
  while (true) {
    display_game(win_manager, game_manager);
    if (play_turn(win_manager, game_manager, resume)) {
      save = true;
      break;
    }
    if (check_handle_win(win_manager, game_manager))
      break;
    resume = false;
  }

  clear_refresh_win(&win_manager->io_win);
  clear_refresh_win(&win_manager->stats_win);
  clear_refresh_win(&win_manager->content_win);

  if (save)
    save_game(win_manager, game_manager);

  disable_cursor();

  free_game_manager(game_manager);
}

void print_play_menu(WinWrapper *win_wrapper) {
  mv_printf_centered(win_wrapper, CONTENT_Y_END / 2, "New game");
  mv_printf_centered(win_wrapper, CONTENT_Y_END / 2 + 2, "Load game");
}

void display_play_menu(WinManager *win_manager) {
  clear_win(&win_manager->content_win);
  print_play_menu(&win_manager->content_win);
  refresh_win(&win_manager->content_win);
}

bool load_game(WinManager *win_manager, GameManager *game_manager) {

  enable_cursor();
  clear_win(&win_manager->io_win);

  char filename[MAX_FILENAME_LEN];
  prompt_input(&win_manager->io_win, "Load from file: ", filename);

  disable_cursor();
  if (!deserialize_game(win_manager, game_manager, filename)) {
    refresh_win(&win_manager->io_win);
    return false;
  }
  return true;
}

bool play_load_game(WinManager *win_manager) {
  GameManager game_manager;

  if (!load_game(win_manager, &game_manager))
    return false;
  game_loop(win_manager, &game_manager, true);

  return true;
}

bool play_new_game(WinManager *win_manager) {
  enable_cursor();

  GameManager game_manager;
  game_manager = new_game_manager();
  game_loop(win_manager, &game_manager, false);

  return true;
}

void resume_game_from_watch(WinManager *win_manager,
                            GameManager *game_manager) {
  clear_refresh_win(&win_manager->legend_win);
  trav_delete_next_moves(&game_manager->turn_log);
  game_loop(win_manager, game_manager, true);
}

void print_legend(WinWrapper *legend_win, const char *str) {
  move_rel(legend_win, 1, 0);
  printf_centered_nl(legend_win, str);
}

void display_quit_legend(WinWrapper *legend_win) {
  clear_win(legend_win);
  print_legend(legend_win, "q quit");
  refresh_win(legend_win);
}

void display_watch_legend(WinWrapper *legend_win) {
  display_quit_legend(legend_win);
  print_legend(legend_win, "p prev move");
  print_legend(legend_win, "n next move");
  print_legend(legend_win, "r resume");
  print_legend(legend_win, "a goto start");
  print_legend(legend_win, "z goto end");
  refresh_win(legend_win);
}

bool init_watch_menu(WinManager *win_manager, GameManager *game_manager,
                     GameManager *end_game) {
  clear_refresh_win(&win_manager->io_win);
  if (!load_game(win_manager, game_manager)) {
    return false;
  }

  display_watch_legend(&win_manager->legend_win);
  *end_game = *game_manager;
  trav_apply_to_start(game_manager);
  return true;
}

void watch_menu_loop(WinManager *win_manager) {
  GameManager game_manager, end_game;
  if (!init_watch_menu(win_manager, &game_manager, &end_game))
    return;
  clear_refresh_win(&win_manager->io_win);

  while (true) {
    display_game(win_manager, &game_manager);
    switch (char_input()) {
    case 'j':
    case 'p':
      trav_apply_move(&game_manager, true);
      break;
    case 'k':
    case 'n':
      trav_apply_move(&game_manager, false);
      break;
    case 'a':
      trav_apply_to_start(&game_manager);
      break;
    case 'z':
      trav_apply_to_end(&game_manager, &end_game);
      break;
    case 'r':
      resume_game_from_watch(win_manager, &game_manager);
      return;

    case 'q':
      return;
    }
  }
}

void play_menu_loop(WinManager *win_manager) {

  display_quit_legend(&win_manager->legend_win);
  clear_refresh_win(&win_manager->io_win);
  while (true) {
    display_play_menu(win_manager);
    switch (char_input()) {
    case 'l':
      if (play_load_game(win_manager))
        return;
      break;
    case 'n':
      play_new_game(win_manager);
      return;
    case 'q':
      return;
    }
  }
}

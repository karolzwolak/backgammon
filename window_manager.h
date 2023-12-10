#pragma once
#include "window.h"

const int VERTICAL_MARGIN = 2;
const int IN_BETWEEN_LINES_MARGIN = 1;

const int WIN_H_MARGIN = 2;
const int WIN_V_MARGIN = 1;

// count of rows inside board
const int BOARD_ROW_COUNT = 5;
const int BOARD_COL_COUNT = 12;
const int BOARD_POINT_SPACING = 3;
const int BOARD_POINT_WIDTH = BOARD_POINT_SPACING + 1;

const int CONTENT_VERTICAL_MARGIN = 0;
const int CONTENT_HORIZONTAL_MARGIN = 1;

const int BAR_VERTICAL_GAP = 1;
const int BAR_HORIZONTAL_GAP = 6;
const int BAR_HORIZONTAL_COMBINED_GAP =
    BAR_HORIZONTAL_GAP - BOARD_POINT_WIDTH + 1;

const int BOARD_HEIGHT =
    BOARD_ROW_COUNT * 2 + BAR_VERTICAL_GAP + 2 * CONTENT_VERTICAL_MARGIN + 2;
const int BOARD_WIDTH = BOARD_COL_COUNT * BOARD_POINT_WIDTH +
                        BAR_HORIZONTAL_COMBINED_GAP +
                        2 * CONTENT_HORIZONTAL_MARGIN;

const int CONTENT_WIN_HEIGHT = BOARD_HEIGHT + 2;
const int CONTENT_WIN_WIDTH = BOARD_WIDTH + 2;

const int SIDE_WIN_HEIGHT = CONTENT_WIN_HEIGHT;
const int SIDE_WIN_WIDTH = 16;

const int ABOUT_WIN_HEIGHT = 3;
const int ABOUT_WIN_WIDTH =
    2 * WIN_H_MARGIN + SIDE_WIN_WIDTH * 2 + CONTENT_WIN_WIDTH;

const int IO_WIN_HEIGHT = CONTENT_WIN_HEIGHT / 2;
const int IO_WIN_WIDTH = ABOUT_WIN_WIDTH;

const int MIN_HEIGHT = ABOUT_WIN_HEIGHT + CONTENT_WIN_HEIGHT + IO_WIN_HEIGHT +
                       4 * WIN_V_MARGIN + 2;
const int MIN_WIDTH = ABOUT_WIN_WIDTH + 2 * WIN_H_MARGIN + 2;

typedef struct {
  WinWrapper main_win, about_win, legend_win, content_win, stats_win, io_win;
  int term_height, term_width;
} WinManager;

void free_win_manager(WinManager *manager);

void run();

void disable_cursor();
void enable_cursor();

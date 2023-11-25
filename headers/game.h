#pragma once

#include "window_manager.h"

#define BOARD_SIZE 24
const int HALF_BOARD = BOARD_SIZE / 2;
const int QUARTER_BOARD = BOARD_SIZE / 4;

const int CONTENT_VERTICAL_MARGIN = 0;
const int CONTENT_HORIZONTAL_MARGIN = 1;

// count of rows inside board
const int BOARD_ROW_COUNT = 5;
const int BOARD_COL_COUNT = BOARD_SIZE / 2;
const int BOARD_POINT_SPACING = 3;
const int BOARD_POINT_WIDTH = BOARD_POINT_SPACING + 1;

const int BAR_VERTICAL_GAP = 1;
const int BAR_HORIZONTAL_GAP = 6;
const int BAR_HORIZONTAL_COMBINED_GAP =
    BAR_HORIZONTAL_GAP - BOARD_POINT_WIDTH + 1;

const int BOARD_HEIGHT =
    BOARD_ROW_COUNT * 2 + BAR_VERTICAL_GAP + 2 * CONTENT_VERTICAL_MARGIN + 2;
const int BOARD_WIDTH = BOARD_COL_COUNT * BOARD_POINT_WIDTH +
                        BAR_HORIZONTAL_COMBINED_GAP +
                        2 * CONTENT_HORIZONTAL_MARGIN;

// content start/end is the first or last index that we can draw characters at
const int CONTENT_Y_START = CONTENT_VERTICAL_MARGIN + 1;
const int CONTENT_Y_END = BOARD_HEIGHT - CONTENT_VERTICAL_MARGIN;
const int CONTENT_X_START = CONTENT_HORIZONTAL_MARGIN + 1;
const int CONTENT_X_END = BOARD_WIDTH - CONTENT_HORIZONTAL_MARGIN;

void game_loop(WinManager *win_manager);

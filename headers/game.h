#pragma once

#include "window_manager.h"

#define BOARD_SIZE 24
const int HALF_BOARD = BOARD_SIZE / 2;
const int QUARTER_BOARD = BOARD_SIZE / 4;

// content start/end is the first or last index that we can draw characters at
const int CONTENT_Y_START = CONTENT_VERTICAL_MARGIN + 1;
const int CONTENT_Y_END = BOARD_HEIGHT - CONTENT_VERTICAL_MARGIN;
const int CONTENT_X_START = CONTENT_HORIZONTAL_MARGIN + 1;
const int CONTENT_X_END = BOARD_WIDTH - CONTENT_HORIZONTAL_MARGIN;

void play_menu_loop(WinManager *win_manager);

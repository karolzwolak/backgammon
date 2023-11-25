#pragma once
#include "window.h"

typedef struct {
  WinWrapper main_win, about_win, legend_win, content_win, stats_win, io_win;
  int term_height, term_width;
} WinManager;

void run();

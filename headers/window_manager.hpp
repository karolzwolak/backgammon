#pragma once
#include "window.hpp"

struct WinManager {
  WinWrapper main_win, about_win, legend_win, content_win, stats_win, io_win;
  int term_height, term_width;
};

void run();

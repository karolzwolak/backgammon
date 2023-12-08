#pragma once

#include <stdlib.h>

#define DEFAULT_CAP 8
#define GROWTH_FACTOR 2
#define NO_HEAP_MEM_EXIT 2

typedef struct {
  void *data;
  size_t elem_size;
  int len, cap;
} Vec;

void vec_with_cap(Vec *vec_out, size_t elem_size, int cap);
void vec_new(Vec *vec_out, size_t elem_size);
int vec_extend(Vec *vec);
void vec_free(Vec *vec);

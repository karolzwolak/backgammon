#pragma once

#include <stdlib.h>

#define DEFAULT_CAP 10

typedef struct {
  void *data;
  size_t elem_size;
  int len, cap;
} Vec;

void vec_new(Vec *vec_out, size_t elem_size);
int vec_extend(Vec *vec);
void vec_free(Vec *vec);

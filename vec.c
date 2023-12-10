#include "vec.h"
#include <stdlib.h>

void vec_with_cap(Vec *vec_out, size_t elem_size, int cap) {
  void *data = malloc(cap * elem_size);
  if (data == NULL) {
    vec_out->data = NULL;
    return;
  }
  vec_out->elem_size = elem_size;
  vec_out->data = data;
  vec_out->cap = cap;
  vec_out->len = 0;
}

void vec_new(Vec *vec_out, size_t elem_size) {
  vec_with_cap(vec_out, elem_size, DEFAULT_CAP);
}

int vec_extend(Vec *vec) {
  if (vec == NULL)
    return 1;

  vec->cap *= GROWTH_FACTOR;
  vec->data = realloc(vec->data, vec->cap * vec->elem_size);

  if (vec->data == NULL)
    return 1;

  return 0;
}

void vec_free(Vec *vec) {
  free(vec->data);
  vec->data = NULL;
  vec->cap = 0;
  vec->len = 0;
}

#include "../headers/vec.h"
#include <stdlib.h>
#include <string.h>

void vec_new(Vec *vec_out, size_t elem_size) {
  void *data = malloc(DEFAULT_CAP * elem_size);
  if (data == NULL) {
    vec_out->data = NULL;
    return;
  }
  vec_out->elem_size = elem_size;
  vec_out->data = data;
  vec_out->cap = DEFAULT_CAP;
  vec_out->len = 0;
}

int vec_extend(Vec *vec) {
  if (vec == NULL)
    return 1;
  void *new_data = malloc(vec->cap * 2 * vec->elem_size);
  if (new_data == NULL)
    return 1;

  memcpy(new_data, vec->data, vec->len * vec->elem_size);
  free(vec->data);
  vec->data = new_data;
  vec->cap *= 2;
  return 0;
}

void vec_free(Vec *vec) {
  free(vec->data);
  vec->data = NULL;
  vec->cap = 0;
  vec->len = 0;
}

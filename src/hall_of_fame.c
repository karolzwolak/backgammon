#include "../headers/hall_of_fame.h"
#include "../headers/window.h"
#include "stdio.h"
#include "vec.c"
#include <string.h>

#define FAME_FILE ".hall_of_fame.txt"
#define FAME_HEADER "HALL-OF-FAME"

void push_player(HallOfFame *h, PlayerEntry *player) {
  if (h->vec.len + 1 > h->vec.cap) {
    if (vec_extend(&h->vec) == 1) {
      exit(NO_HEAP_MEM_EXIT);
    }
  }

  PlayerEntry *data = h->vec.data;
  data[h->vec.len++] = *player;
}

void add_player(HallOfFame *h, PlayerEntry *player) {
  for (int i = 0; i < h->vec.len; i++) {
    PlayerEntry *data = h->vec.data;
    PlayerEntry *entry = &data[i];
    if (strcmp(player->name, entry->name) == 0) {
      entry->points += player->points;
      return;
    }
  }
  push_player(h, player);
}

void free_entries(HallOfFame *h) {
  PlayerEntry *data = h->vec.data;
  for (int i = 0; i < h->vec.len; i++) {
    free(data[i].name);
  }
  vec_free(&h->vec);
}

int compare_players(const PlayerEntry *a, const PlayerEntry *b) {
  if (a->points > b->points)
    return 1;
  if (a->points < b->points)
    return -1;
  return 0;
}

void sort_players(HallOfFame *h) {
  qsort(h->vec.data, sizeof(PlayerEntry), h->vec.len,
        (int (*)(const void *, const void *))compare_players);
}

bool deserialize_player(FILE *fp, PlayerEntry *player) {
  char *name = malloc(MAX_INPUT_LEN * sizeof(char));
  int points;
  if (name == NULL) {
    exit(NO_HEAP_MEM_EXIT);
  }
  if (fscanf(fp, "%s %d\n", name, &points) != 2)
    return false;

  *player = (PlayerEntry){points, name};
  return true;
}

bool deserialize_entries(HallOfFame *h, FILE *fp) {
  vec_new(&h->vec, sizeof(PlayerEntry));

  if (h->vec.data == NULL) {
    exit(NO_HEAP_MEM_EXIT);
  }

  PlayerEntry player;
  char header[MAX_INPUT_LEN] = "";

  if (fscanf(fp, "%s\n", header) != 1 || strcmp(header, FAME_HEADER) != 0)
    return false;

  while (deserialize_player(fp, &player)) {
    push_player(h, &player);
  }

  return true;
}

void serialize_player(PlayerEntry *player, FILE *fp) {
  fprintf(fp, "%s %d\n", player->name, player->points);
}

bool serialize_entries(HallOfFame *h) {
  FILE *fp = fopen(FAME_FILE, "w");

  if (fp == NULL)
    return false;

  fprintf(fp, "%s\n", FAME_HEADER);

  PlayerEntry *data = h->vec.data;
  for (int i = 0; i < h->vec.len; i++) {
    serialize_player(&data[i], fp);
  }

  fclose(fp);
  return true;
}

bool save_entries(HallOfFame *h) {
  bool success = serialize_entries(h);
  free_entries(h);
  return success;
}

// returns if file was created
bool open_or_create_file(FILE **f_out) {
  *f_out = fopen(FAME_FILE, "r+");

  if (*f_out == NULL) {
    *f_out = fopen(FAME_FILE, "w+");
    if (*f_out != NULL)
      fprintf(*f_out, "%s\n", FAME_HEADER);
    return true;
  }
  return false;
}

bool load_entries(HallOfFame *h) {
  FILE *fp = NULL;

  if (open_or_create_file(&fp))
    return false;

  bool success = deserialize_entries(h, fp);
  fclose(fp);
  return success;
}

bool add_one_player(int points, char *name) {
  HallOfFame h;
  FILE *fp = NULL;

  if (open_or_create_file(&fp))
    return false;

  if (deserialize_entries(&h, fp)) {
    PlayerEntry player = (PlayerEntry){points, name};
    add_player(&h, &player);
    sort_players(&h);
  }

  return save_entries(&h);
}

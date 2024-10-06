#include "colors.h"

#include <time.h>

/* Bitwise */

bool colors_is_equal(const colors_t colors1, const colors_t colors2) {
  return colors1 == colors2;
}

bool colors_is_in(const colors_t colors, const size_t color_id) {
  return !((colors & colors_set(color_id)) == 0ULL);
}

bool colors_is_singleton(const colors_t colors) {
  return (colors != 0) && ((colors & (colors - 1)) == 0);
}

bool colors_is_subset(const colors_t colors1, const colors_t colors2) {
  return (colors1 & colors2) == colors1;
}

colors_t colors_add(const colors_t colors, const size_t color_id) {
  return colors | colors_set(color_id);
}

colors_t colors_and(const colors_t colors1, const colors_t colors2) {
  return colors1 & colors2;
}

colors_t colors_discard(const colors_t colors, const size_t color_id) {
  return colors & ~(colors_set(color_id));
}

colors_t colors_empty(void) {
  return 0ULL;
}

colors_t colors_full(const size_t size) {
  if (size >= MAX_COLORS) {
    return ~0ULL;
  }
  return (1ULL << size) - 1ULL;
}

colors_t colors_leftmost(const colors_t colors) {
  colors_t tmp = colors;
  tmp |= (tmp >> 1);
  tmp |= (tmp >> 2);
  tmp |= (tmp >> 4);
  tmp |= (tmp >> 8);
  tmp |= (tmp >> 16);
  tmp |= (tmp >> 32);
  return (tmp - (tmp >> 1));
}

colors_t colors_negate(const colors_t colors) {
  return ~colors;
}

colors_t colors_or(const colors_t colors1, const colors_t colors2) {
  return colors1 | colors2;
}

/* Here we suppose that the seed has already been initialized in the main */
colors_t colors_random(const colors_t colors) {
  if (colors == 0) {
    return 0;
  }

  colors_t copy = colors;
  size_t count = colors_count(colors);
  int index = rand() % count;

  while (index > 0) {
    colors_t singleton = colors_rightmost(copy);
    copy = colors_subtract(copy, singleton);
    index--;
  }

  return colors_rightmost(copy);
}

colors_t colors_rightmost(const colors_t colors) {
  return colors & -colors;
}

colors_t colors_set(const size_t color_id) {
  if (color_id > MAX_COLORS) {
    return 0ULL;
  }
  return 1ULL << color_id;
}

colors_t colors_subtract(const colors_t colors1, const colors_t colors2) {
  return colors1 & ~colors2;
}

colors_t colors_xor(const colors_t colors1, const colors_t colors2) {
  return colors1 ^ colors2;
}

size_t colors_count(const colors_t colors) {
  colors_t B5 = (1ULL << 32) - 1ULL;
  colors_t B4 = B5 ^ (B5 << 16);
  colors_t B3 = B4 ^ (B4 << 8);
  colors_t B2 = B3 ^ (B3 << 4);
  colors_t B1 = B2 ^ (B2 << 2);
  colors_t B0 = B1 ^ (B1 << 1);

  size_t count = colors;
  count = ((count >> 1) & B0) + (count & B0);
  count = ((count >> 2) & B1) + (count & B1);
  count = ((count >> 4) + count) & B2;
  count = ((count >> 8) + count) & B3;
  count = ((count >> 16) + count) & B4;
  count = ((count >> 32) + count) & B5;

  return count;
}

/* Heuristics and  subgrid functions  */

bool cross_hatching_heuristic(colors_t *subgrid[], size_t size) {
  colors_t singleton = colors_empty();
  bool changed = false;

  for (size_t index = 0; index < size; index++) {
    if (colors_is_singleton(*subgrid[index])) {
      singleton = colors_or(singleton, *subgrid[index]);
    }
  }

  if (singleton != 0) {
    for (size_t index = 0; index < size; index++) {
      if (!colors_is_singleton(*subgrid[index]) &&
          colors_and(singleton, *subgrid[index]) != 0) {

        *subgrid[index] = colors_subtract(*subgrid[index], singleton);
        changed = true;
      }
    }
  }

  return changed;
}

bool lone_number_heuristic(colors_t *subgrid[], const size_t size) {
  bool result = false;
  size_t final_index = 0;
  int count;

  for (size_t id_one = 0; id_one < size; id_one++) {
    count = 0;
    for (size_t id_two = 0; id_two < size; id_two++) {
      if (colors_is_in(*subgrid[id_two], id_one)) {
        final_index = id_two;
        count++;
      }
    }

    if (count == 1 && !colors_is_singleton(*subgrid[final_index])) {
      *subgrid[final_index] = colors_set(id_one);
      result = true;
    }
  }
  return result;
}

bool naked_subset_heuristic(colors_t *subgrid[], const size_t size) {
  bool result = false;

  for (size_t id_one = 0; id_one < size - 1; id_one++) {
    int count = 1;
    int color_nb = 0;

    if (!colors_is_singleton(*subgrid[id_one]) &&
        *subgrid[id_one] != colors_full(size)) {
      color_nb = colors_count(*subgrid[id_one]);

      for (size_t id_two = id_one + 1; id_two < size; id_two++) {
        if (id_one != id_two &&
            colors_is_equal(*subgrid[id_one], *subgrid[id_two])) {
          count++;
        }
      }
    }

    if (color_nb == count) {

      for (size_t id_three = 0; id_three < size; id_three++) {
        if (id_three != id_one &&
            !colors_is_equal(*subgrid[id_one], *subgrid[id_three]) &&
            colors_and(*subgrid[id_one], *subgrid[id_three]) != 0) {
          *subgrid[id_three] =
              colors_subtract(*subgrid[id_three], *subgrid[id_one]);
          result = true;
        }
      }
    }
  }

  return result;
}

bool hidden_subset_heuristic(colors_t *subgrid[], const size_t size) {
  bool result = false;
  size_t count;
  size_t color_nb;
  for (size_t id_one = 0; id_one < size; id_one++) {
    count = 1;
    color_nb = 0;
    if (!colors_is_singleton(*subgrid[id_one]) &&
        *subgrid[id_one] != colors_full(size)) {
      color_nb = colors_count(*subgrid[id_one]);
      for (size_t id_two = 0; id_two < size; id_two++) {
        if (id_one != id_two &&
            *subgrid[id_two] !=
                colors_subtract(*subgrid[id_one], *subgrid[id_two])) {

          count++;
        }
      }

      if (color_nb == count) {
        for (size_t id_three = 0; id_three < size; id_three++) {

          if (id_three != id_one &&
              *subgrid[id_three] !=
                  colors_subtract(*subgrid[id_one], *subgrid[id_three])) {
            *subgrid[id_three] =
                colors_and(*subgrid[id_three], *subgrid[id_one]);
            result = true;
          }
        }
      }
    }
  }
  return result;
}

bool subgrid_consistency(colors_t subgrid[], const size_t size) {
  if (subgrid == NULL) {
    return false;
  }

  colors_t color_count = 0ULL;
  colors_t singleton_set = 0ULL;
  for (size_t index = 0; index < size; index++) {

    if (subgrid[index] == 0) {
      return false;
    }

    if (colors_is_singleton(subgrid[index])) {
      if (colors_is_subset(subgrid[index], singleton_set)) {
        return false;
      }
      singleton_set = colors_or(singleton_set, subgrid[index]);
    }
    color_count = colors_or(color_count, subgrid[index]);
  }

  return color_count == colors_full(size);
}

bool subgrid_heuristics(colors_t *subgrid[], const size_t size) {
  if (subgrid == NULL) {
    return false;
  }
  bool changes = false;

  while (1) {
    if (cross_hatching_heuristic(subgrid, size)) {
      changes = true;
      continue;
    }
    if (lone_number_heuristic(subgrid, size)) {
      changes = true;
      continue;
    }
    break;
  }

  while (naked_subset_heuristic(subgrid, size)) {
    changes = true;
  }
  while (hidden_subset_heuristic(subgrid, size)) {
    changes = true;
  }
  return changes;
}

#include "grid.h"

#include <math.h>

/* Internal structure (hidden from outside for a sudoku grid) */
struct _grid_t {
  size_t size;
  colors_t **cells;
};

/* Grid functions */

char *grid_get_cell(const grid_t *grid, const size_t row, const size_t column) {
  if (grid == NULL || row >= grid->size || column >= grid->size) {
    return NULL;
  }

  colors_t color = grid->cells[row][column];
  char *string = calloc(colors_count(color) + 1, sizeof(char));
  if (string == NULL) {
    return NULL;
  }

  int string_index = 0;
  for (size_t index = 0; index < grid->size; index++) {
    if (colors_is_in(color, index)) {
      string[string_index] = color_table[index];
      string_index++;
    }
  }
  return string;
}

bool grid_check_char(const grid_t *grid, const char c) {
  if (grid == NULL) {
    return false;
  }

  if (c == EMPTY_CELL) {
    return true;
  }

  for (unsigned long index = 0; index < grid->size; index++) {
    if (c == color_table[index]) {
      return true;
    }
  }
  return false;
}

bool grid_check_size(const size_t size) {
  return (size == 1 || size == 4 || size == 9 || size == 16 || size == 25 ||
          size == 36 || size == 49 || size == 64);
}

bool grid_is_consistent(grid_t *grid) {
  if (grid == NULL) {
    return false;
  }

  bool result = true;
  size_t size = grid_get_size(grid);
  colors_t subgrid[size];

  /* Rows */
  for (size_t row = 0; row < size; row++) {
    for (size_t col = 0; col < size; col++) {
      subgrid[col] = grid->cells[row][col];
    }
    result &= subgrid_consistency(subgrid, size);
  }

  /* Columns */
  for (size_t col = 0; col < size; col++) {
    for (size_t row = 0; row < size; row++) {
      subgrid[row] = grid->cells[row][col];
    }
    result &= subgrid_consistency(subgrid, size);
  }

  /* Blocks */
  size_t block_size = sqrt(size);

  for (size_t block_number = 0; block_number < size; block_number++) {

    size_t index = 0;
    size_t row_offset = (block_number / block_size) * block_size;
    size_t col_offset = (block_number % block_size) * block_size;

    for (size_t row = 0; row < block_size; row++) {
      for (size_t col = 0; col < block_size; col++) {
        subgrid[index] = grid->cells[row + row_offset][col + col_offset];
        index++;
      }
    }
    result &= subgrid_consistency(subgrid, size);
  }
  return result;
}

bool grid_is_solved(grid_t *grid) {
  if (grid == NULL) {
    return false;
  }

  for (size_t row = 0; row < grid->size; row++) {
    for (size_t col = 0; col < grid->size; col++) {
      if (!colors_is_singleton(grid->cells[row][col])) {
        return false;
      }
    }
  }

  return true;
}

bool subgrid_apply(grid_t *grid,
                   bool (*func)(colors_t *subgrid[], const size_t size)) {

  if (grid == NULL) {
    return false;
  }

  bool result = false;
  size_t size = grid_get_size(grid);
  colors_t *subgrid[size];

  /* Rows */
  for (size_t row = 0; row < size; row++) {
    for (size_t col = 0; col < size; col++) {
      subgrid[col] = &(grid->cells[row][col]);
    }

    result |= func(subgrid, size);
  }

  /* Columns */
  for (size_t col = 0; col < size; col++) {
    for (size_t row = 0; row < size; row++) {
      subgrid[row] = &(grid->cells[row][col]);
    }

    result |= func(subgrid, size);
  }

  /* Blocks */
  size_t block_size = sqrt(size);
  for (size_t block_number = 0; block_number < size; block_number++) {

    size_t index = 0;
    size_t row_offset = (block_number / block_size) * block_size;
    size_t col_offset = (block_number % block_size) * block_size;

    for (size_t row = 0; row < block_size; row++) {
      for (size_t col = 0; col < block_size; col++) {
        subgrid[index] = &(grid->cells[row + row_offset][col + col_offset]);
        index++;
      }
    }
    result |= func(subgrid, size);
  }

  return result;
}

grid_t *grid_alloc(size_t size) {
  if (!grid_check_size(size)) {
    return NULL;
  }

  grid_t *grid = malloc(sizeof(grid_t));
  if (grid == NULL) {
    return NULL;
  }

  grid->cells = malloc(size * sizeof(colors_t *));
  if (grid->cells == NULL) {
    free(grid);
    return NULL;
  }

  for (size_t index = 0; index < size; index++) {
    grid->cells[index] = malloc(size * sizeof(colors_t));
    if (grid->cells[index] == NULL) {
      for (size_t de_alloc_ind = 0; de_alloc_ind < index; de_alloc_ind++) {
        free(grid->cells[de_alloc_ind]);
      }
      free(grid->cells);
      free(grid);
      return NULL;
    }
  }
  grid->size = size;
  return grid;
}

grid_t *grid_copy(const grid_t *grid) {
  if (grid == NULL) {
    return NULL;
  }

  size_t size = grid->size;
  grid_t *copy = grid_alloc(size);
  if (copy == NULL) {
    return NULL;
  }
  for (size_t row = 0; row < size; row++) {
    for (size_t col = 0; col < size; col++) {
      copy->cells[row][col] = grid->cells[row][col];
    }
  }

  copy->size = size;
  return copy;
}

size_t grid_get_size(const grid_t *grid) {
  if (grid == NULL) {
    return 0;
  }
  return grid->size;
}

void grid_free(grid_t *grid) {
  if (grid == NULL) {
    return;
  }
  if (grid != NULL) {
    for (size_t index = 0; index < grid->size; index++) {
      free(grid->cells[index]);
    }
    free(grid->cells);
    free(grid);
  }
}

void grid_print(const grid_t *grid, FILE *fd) {
  if (grid == NULL || fd == NULL) {
    return;
  }

  for (size_t row = 0; row < grid->size; row++) {
    for (size_t col = 0; col < grid->size; col++) {
      if (colors_is_singleton(grid->cells[row][col])) {
        char *string = grid_get_cell(grid, row, col);
        fprintf(fd, "%s ", string);
        free(string);
      } else {
        fprintf(fd, "_ ");
      }
    }
    fputc('\n', fd);
  }
  fputc('\n', fd);
}

void grid_set_cell(grid_t *grid, const size_t row, const size_t column,
                   const char color) {
  if (grid == NULL || row >= grid->size || column >= grid->size) {
    return;
  }

  for (size_t index = 0; index <= grid->size; index++) {
    if (color_table[index] == color) {
      grid->cells[row][column] = 1ULL << index;
      return;
    }
  }
  grid->cells[row][column] = colors_full(grid->size);
}

status_t grid_heuristics(grid_t *grid) {
  if (grid == NULL) {
    return grid_inconsistent;
  }

  while (subgrid_apply(grid, subgrid_heuristics))
    ;

  if (!grid_is_consistent(grid)) {
    return grid_inconsistent;
  }
  if (grid_is_solved(grid)) {
    return grid_solved;
  }
  return grid_unsolved;
}

/* Choice functions */

bool grid_choice_is_empty(const choice_t choice) {
  return choice.color == colors_empty();
}

void grid_choice_apply(grid_t *grid, const choice_t choice) {
  if (grid == NULL || grid_choice_is_empty(choice)) {
    return;
  }

  grid->cells[choice.row][choice.col] = choice.color;
}

void grid_choice_discard(grid_t *grid, const choice_t choice) {
  if (grid == NULL || choice.row >= grid->size || choice.col >= grid->size) {
    return;
  }

  grid->cells[choice.row][choice.col] =
      colors_discard(grid->cells[choice.row][choice.col], log2(choice.color));
}

void grid_choice_print(const choice_t choice, FILE *fd) {
  if (fd == NULL) {
    return;
  }

  fprintf(fd, "Next choice: row [%ld], col [%ld], choice = %c\n",
          choice.row + 1, choice.col + 1, color_table[(int)log2(choice.color)]);
}

choice_t grid_choice(grid_t *grid) {
  choice_t choice;

  if (grid == NULL) {
    choice.row = 0;
    choice.col = 0;
    choice.color = colors_empty();
    return choice;
  }

  colors_t color_ref = colors_full(grid->size);
  size_t row_ref = 0;
  size_t column_ref = 0;

  for (size_t row = 0; row < grid->size; row++) {
    for (size_t col = 0; col < grid->size; col++) {
      if (!colors_is_singleton(grid->cells[row][col]) &&
          colors_count(grid->cells[row][col]) <= colors_count(color_ref)) {
        color_ref = grid->cells[row][col];
        row_ref = row;
        column_ref = col;
      }
    }
  }

  choice.row = row_ref;
  choice.col = column_ref;
  choice.color = colors_rightmost(color_ref);
  return (choice);
}

/* For gererating grid */

colors_t get_grid_color(grid_t *grid, size_t row, size_t col) {
  return grid->cells[row][col];
}

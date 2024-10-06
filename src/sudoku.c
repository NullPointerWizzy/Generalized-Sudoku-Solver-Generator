#include "sudoku.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <getopt.h>
#include <math.h>
#include <time.h>

#include "grid.h"

static bool verbose = false;
static bool unique = false;
static int solutions = 0;
static int grid_size = DEFAULT_GRID_SIZE;

/* Function used to initialise a seed once */

static void seed_init(unsigned int seed) {
  static bool seed_initialized = false;
  if (!seed_initialized) {
    srand(seed);
    seed_initialized = true;
  }
}

/* Backtrack */

static grid_t *backtrack(grid_t *grid, const mode_tt mode, FILE *fd) {
  if (grid == NULL) {
    return NULL;
  }
  while (grid_is_consistent(grid)) {
    while (subgrid_apply(grid, subgrid_heuristics))
      ;
    if (grid_is_solved(grid)) {
      if (grid_is_consistent(grid)) {
        solutions++;
        if (mode == mode_all) {
          if (!unique) {
            fprintf(fd, "Solution #%d:\n", solutions);
            grid_print(grid, fd);
          }
          grid_free(grid);
          return NULL;
        }
        return grid;
      }
    }

    choice_t choice = grid_choice(grid);
    if (choice.color == 0) {
      grid_free(grid);
      return NULL;
    }
    if (verbose) {
      grid_choice_print(choice, fd);
    }
    grid_t *copy = grid_copy(grid);
    if (copy == NULL) {
      grid_free(grid);
      return NULL;
    }
    grid_choice_apply(copy, choice);
    copy = backtrack(copy, mode, fd);
    if (copy == NULL) {
      grid_choice_discard(grid, choice);
    } else {
      grid_free(grid);
      return copy;
    }
  }

  grid_free(grid);
  return NULL;
}

static void cleaning(FILE *filename, grid_t *grid) {
  if (filename != NULL) {
    fclose(filename);
  }

  if (grid != NULL) {
    grid_free(grid);
  }
}

/* Generator */

static grid_t *grid_generator(size_t size, FILE *fd) {
  grid_t *grid = grid_alloc(size);
  if (grid == NULL) {
    return NULL;
  }
  for (size_t row = 0; row < size; row++) {
    for (size_t col = 0; col < size; col++) {
      grid_set_cell(grid, row, col, colors_full(size));
    }
  }

  colors_t color_choice = colors_full(size);
  colors_t color;
  colors_t color_after_row = colors_full(size);
  colors_t color_after_block = colors_full(size);

  size_t sqrt_s = (size_t)sqrt(size);

  /* First row */
  for (size_t col = 0; col < size; col++) {
    color = colors_random(color_choice);
    grid_set_cell(grid, 0, col, color_table[(int)log2(color)]);
    color_choice = colors_discard(color_choice, (int)log2(color));
    if (col == 0) {
      color_after_block = colors_discard(color_after_block, (int)log2(color));
    }
    if (col < sqrt_s) {
      color_after_row = colors_discard(color_after_row, (int)log2(color));
    }
  }

  /* First block */
  for (size_t row = 1; row < sqrt_s; row++) {
    for (size_t col = 0; col < sqrt_s; col++) {
      color = colors_random(color_after_row);
      grid_set_cell(grid, row, col, color_table[(int)log2(color)]);
      color_after_row = colors_discard(color_after_row, (int)log2(color));
      if (col == 0) {
        color_after_block = colors_discard(color_after_block, (int)log2(color));
      }
    }
  }

  /* First column */
  for (size_t row = sqrt_s; row < size; row++) {
    color = colors_random(color_after_block);
    grid_set_cell(grid, row, 0, color_table[(int)log2(color)]);
    color_after_block = colors_discard(color_after_block, (int)log2(color));
  }
  grid_t *after_backtrack = backtrack(grid, mode_first, fd);
  size_t cells_filled = size * size;
  size_t cells_filled_wanted = size * size * FILLING_RATE;

  while (cells_filled > cells_filled_wanted) {
    size_t row = rand() % (size);
    size_t col = rand() % (size);
    grid_t *copy = grid_copy(after_backtrack);
    if (!unique) {
      if (colors_is_singleton(get_grid_color(after_backtrack, row, col))) {
        grid_set_cell(after_backtrack, row, col, colors_full(size));
        cells_filled--;
      }
    } else {
      copy = backtrack(after_backtrack, mode_all, fd);
      if (solutions == 1) {
        return copy;
      }
      return grid_generator(size, fd);
    }
    grid_free(copy);
  }

  return after_backtrack;
}

/* File Parser */

static grid_t *file_parser(char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    warnx("warning: couldn't open file!");
    return NULL;
  }

  /* Getting first row */
  char first_row[MAX_GRID_SIZE];
  char current_char = fgetc(file);
  int size_counter = 0;

  while (current_char != '\n' && current_char != EOF) {
    if (current_char == '#') {
      while (current_char != '\n' && current_char != EOF) {
        current_char = fgetc(file);
      }
      current_char = fgetc(file);

      if (current_char == EOF) {
        warnx("warning: '%s': empty grid!", filename);
        cleaning(file, NULL);
        return NULL;
      }
      while (current_char == '\n') {
        current_char = fgetc(file);
      }
    }

    if (current_char != ' ' && current_char != '\t') {
      first_row[size_counter] = current_char;
      size_counter++;
    }
    current_char = fgetc(file);
  }

  size_t grid_size = size_counter;

  if (grid_size == 0) {
    warnx("warning: '%s': empty grid!", filename);
    cleaning(file, NULL);
    return NULL;
  }

  if (!grid_check_size(grid_size)) {
    warnx("warning: '%s': invalid grid size!", filename);
    cleaning(file, NULL);
    return NULL;
  }

  /* Filling the grid first row */
  grid_t *grid = grid_alloc(grid_size);
  if (grid == NULL) {
    warnx("error: could not allocate the grid!");
    cleaning(file, NULL);
    return NULL;
  }

  for (size_t index = 0; index < grid_size; index++) {
    if (!grid_check_char(grid, first_row[index])) {
      warnx("warning: '%s': wrong character '%c' at row '%d'!", filename,
            first_row[index], 1);
      cleaning(file, grid);
      return NULL;
    }

    grid_set_cell(grid, 0, index, first_row[index]);
  }

  /* Reading file and filling grid */
  size_t row = 1;
  size_t col = 0;

  while ((current_char = fgetc(file)) != EOF) {

    switch (current_char) {
    case '#':
      while (current_char != '\n' && current_char != EOF) {
        current_char = fgetc(file);
      }
      break;

    case '\n':
      if (col != grid_size && col != 0) {
        warnx("warning: '%s': row '%ld' is malformed! (wrong number of "
              "columns)",
              filename, row + 1);
        cleaning(file, grid);
        return NULL;
      }
      if (!(col == 0)) {
        row++;
      }
      col = 0;
      break;

    case ' ':
    case '\t':
      break;

    default:
      if (!grid_check_char(grid, current_char)) {
        warnx("warning: '%s': wrong character '%c' at row '%ld'!", filename,
              current_char, row + 1);
        cleaning(file, grid);
        return NULL;
      }

      if (col == grid_size) {
        warnx("warning: '%s': row '%ld' is malformed! (wrong number of "
              "columns)",
              filename, row + 1);
        cleaning(file, grid);
        return NULL;
      }

      if (row >= grid_size) {
        warnx("warning: '%s': grid has too many rows", filename);
        cleaning(file, grid);
        return NULL;
      }

      grid_set_cell(grid, row, col, current_char);
      col++;
      break;
    }
  }

  if (row < grid_size - 1 || (col == 0 && row == grid_size - 1)) {
    warnx("warning: '%s': grid has 1 missing row(s)", filename);
    cleaning(file, grid);
    return NULL;
  }

  if (col != grid_size && col != 0) {
    warnx("warning: '%s': row '%ld' is malformed! (wrong number of "
          "columns)",
          filename, row + 1);
    cleaning(file, grid);
    return NULL;
  }

  fclose(file);
  return grid;
}

int main(int argc, char *argv[]) {
  bool all = false, error_handler = false, generator = false, unique = false;
  bool consistency = true;
  bool solver = true;
  char *filename = NULL;
  FILE *output = stdout;
  int optc;
  mode_tt mode = mode_first;
  seed_init(time(NULL));

  const struct option l_opts[] = {{"help", no_argument, NULL, 'h'},
                                  {"generate", optional_argument, NULL, 'g'},
                                  {"all", no_argument, NULL, 'a'},
                                  {"output", required_argument, NULL, 'o'},
                                  {"unique", no_argument, NULL, 'u'},
                                  {"verbose", no_argument, NULL, 'v'},
                                  {"version", no_argument, NULL, 'V'},
                                  {"help", no_argument, NULL, 'h'},
                                  {NULL, 0, NULL, 0}};

  while ((optc = getopt_long(argc, argv, "ag::o:uvVh", l_opts, NULL)) != -1) {
    switch (optc) {
    case 'a': /* search for all possible solutions */
      all = true;
      mode = mode_all;
      break;

    case 'g': /* generate a grid of size NxN (default: DEFAULT_GRID_SIZE) */
      solver = false;
      generator = true;
      if (optarg == NULL) /* checks for grid size input */
      {
        fprintf(output, "Setting grid size to default: %d\n\n",
                DEFAULT_GRID_SIZE);
      }
      if (optarg != NULL) {
        grid_size = atoi(optarg);
        if (!grid_check_size(grid_size)) {
          errx(EXIT_FAILURE, "error: invalid grid size given!");
        }
      }
      break;

    case 'o': /* write output to file */
      filename = optarg;
      if (filename == NULL) {
        errx(EXIT_FAILURE, "error: ");
      }
      break;

    case 'u': /* generates a grid with a unique solution */
      unique = true;
      break;

    case 'v': /* verbose output */
      verbose = true;
      break;

    case 'V': /* displays version and exit */
      printf("\nsudoku %d.%d.%d\n Solve/generate sudoku grids of "
             "size: 1, 4, 9, 16, 25, 36, 49, 64\n\n",
             VERSION, SUBVERSION, REVISION);
      exit(EXIT_SUCCESS);

    case 'h': /* displays sudoku usage help for */
      printf("\nUsage: sudoku [-a|-o FILE|-v|-V|-h] FILE...\n"
             "       sudoku -g[SIZE] [-u|-o FILE|-v|-V|-h]\n"
             "Solve or generate Sudoku grids of size: "
             "1, 4, 9, 16, 25, 36, 49, 64 \n\n"
             "-a,--all              search for all possible "
             "solutions\n"
             "-g[N],--generate[=N]  generate a grid of size NxN "
             "(default: 9)\n"
             "-o FILE,--output FILE write output to FILE\n"
             "-u,--unique           generate a grid with unique "
             "solution\n"
             "-v,--verbose          verbose output\n"
             "-V,--version          display version and exit\n"
             "-h,--help             display this help and exit\n\n");

      exit(EXIT_SUCCESS);

    default:
      errx(EXIT_FAILURE, "error: invalid option given '%s' !",
           argv[optind - 1]);
    }
  }

  if (filename != NULL) {
    output = fopen(filename, "a");
    if (output == NULL) {
      err(EXIT_FAILURE, "error: ");
    }
  }

  /* Checking various cases (inputs/modes) */
  if (solver) {
    if (unique) {
      error_handler = true;
      warnx("error: option 'unique' conflicts with solver mode, "
            "disabling it!");
    }

    if (optind >= argc) {
      fclose(output);
      errx(EXIT_FAILURE, "error: no input grid given!");
    }

    for (int i = optind; i < argc; i++) {
      fprintf(output, "====================%s====================\n\n",
              argv[i]);
      grid_t *grid_test = file_parser(argv[i]);
      if (grid_test == NULL) {
        error_handler = true;
      } else {
        fprintf(output, "Initial grid:\n");
        grid_print(grid_test, output);
        if (!grid_is_consistent(grid_test)) {
          grid_free(grid_test);
          errx(EXIT_FAILURE, "error: Grid is inconsistent!");
        }
        grid_test = backtrack(grid_test, mode, output);
        if (mode == mode_all) {

          if (solutions == 0) {
            grid_free(grid_test);
            errx(EXIT_FAILURE, "error: Grid is inconsistent!");
          }
          fprintf(output, "There are '%d' solutions\n\n", solutions);
          solutions = 0;
        }

        if (mode == mode_first) {
          grid_test = backtrack(grid_test, mode, output);
          if (grid_test != NULL) {
            fprintf(output, "Solved grid:\n");
            grid_print(grid_test, output);
          } else {
            grid_free(grid_test);
            errx(EXIT_FAILURE, "error: Grid is inconsistent!");
          }
        }
        grid_free(grid_test);
      }
    }
  }

  if (generator) {
    /* Check for conflict of modes */
    if (all) {
      warnx("warning: option 'all' conflict with generator mode!");
      error_handler = true;
    }
    fprintf(output,
            "~~~~~~~~~~~~~ Generator ~~~~~~~~~~~~~\n\n"
            "Generating a grid of size: %d\n",
            grid_size);
    grid_t *grid = grid_generator(grid_size, output);
    grid_print(grid, output);
    fprintf(output, "Filling rate: %0.1f percent.\n\n",
            FILLING_RATE * PERCENT_CONV);
    grid_free(grid);
  }

  if (output != stdout) {
    fclose(output);
  }

  if (error_handler || !consistency) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

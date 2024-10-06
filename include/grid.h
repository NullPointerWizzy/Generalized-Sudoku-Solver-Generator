#ifndef GRID_H
#define GRID_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "colors.h"

#define EMPTY_CELL '_'
#define MAX_GRID_SIZE 64

static const char color_table[] = "123456789"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "@"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "&*";

/* Sudoku grid (forward declaration to hide the implementation)*/

typedef enum { grid_solved, grid_unsolved, grid_inconsistent } status_t;

typedef struct {
  size_t row;
  size_t col;
  colors_t color;
} choice_t;

typedef struct _grid_t grid_t;

/* Functions prototypes */

/**
@brief: gets the grid cell at given row and column
@param: const grid_t *grid, const size_t row, const size_t column
@return: char *
**/
char *grid_get_cell(const grid_t *grid, const size_t row, const size_t column);

/**
@brief: checks if the given character is valid
@param: const grid_t *grid, const char c
@return: bool
**/
bool grid_check_char(const grid_t *grid, const char c);

/**
@brief: checks if grid size is valid
@param: const size_t size
@return: bool
**/
bool grid_check_size(const size_t size);

/**
@brief: checks if grid is consistent
@param:  grid_t *grid
@return: bool
**/
bool grid_is_consistent(grid_t *grid);

/**
@brief: checks if grid is valid
@param: grid_t *grid
@return: bool
**/
bool grid_is_solved(grid_t *grid);

/**
@brief: applies input function to a subgrid of a grid
@param: grid_t *grid,
                   bool (*func)(colors_t *subgrid[], const size_t size)
@return: bool
**/
bool subgrid_apply(grid_t *grid,
                   bool (*func)(colors_t *subgrid[], const size_t size));

/**
@brief: allocates memory for the grid
@param: size_t size
@return: grid_t *grid
**/
grid_t *grid_alloc(size_t size);

/**
@brief: copies given grid into a new grid
@param: const grid_t *grid
@return: grid_t *grid
**/
grid_t *grid_copy(const grid_t *grid);

/**
@brief: gets grid size
@param: const grid_t *grid
@return: size_t
**/
size_t grid_get_size(const grid_t *grid);

/**
@brief: frees allocated memory of the grid
@param: grid_t *grid
@return: void
**/
void grid_free(grid_t *grid);

/**
@brief: displays the grid into the file
@param: const grid_t *grid, FILE *fd
@return: void
**/
void grid_print(const grid_t *grid, FILE *fd);

/**
@brief: sets the grid cell to given color
@param: grid_t *grid, const size_t row, const size_t column,
                   const char color
@return: void
**/
void grid_set_cell(grid_t *grid, const size_t row, const size_t column,
                   const char color);

/**
@brief: returns the grid status
@param: grid_t *grid
@return: status_t
**/
status_t grid_heuristics(grid_t *grid);

/* Choice functions */

/**
@brief: chooses the smallest set of colors of the grid and selects the rightmost
            color
@param: grid_t *grid
@return: choice_t
**/
choice_t grid_choice(grid_t *grid);

/**
@brief: checks if a choice is empty
@param: const choice_t choice
@return: bool
**/
bool grid_choice_is_empty(const choice_t choice);

/**
@brief: applies a choice to a cell in the grid
@param:grid_t *grid, const choice_t choice
@return: void
**/
void grid_choice_apply(grid_t *grid, const choice_t choice);

/**
@brief: discards a choice in a cell in the grid
@param: grid_t *grid, const choice_t choice
@return: void
**/
void grid_choice_discard(grid_t *grid, const choice_t choice);

/**
@brief: writes the choice on the file descriptor for debugging purposes
@param: const choice_t choice, FILE *fd
@return: void
**/
void grid_choice_print(const choice_t choice, FILE *fd);

/* Helping function used for generating */

/**
@brief: returns a cell color
@param: grid_t *grid, size_t row, size_t col
@return: colors_t
**/
colors_t get_grid_color(grid_t *grid, size_t row, size_t col);

#endif /* GRID_H */
#ifndef COLORS_H
#define COLORS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_COLORS 64

typedef uint64_t colors_t;

/* Functions prototypes */

/**
@brief: checks if two colors are equal
@param: const colors_t colors1, const colors_t colors2
@return: bool
**/
bool colors_is_equal(const colors_t colors1, const colors_t colors2);

/**
@brief: checks if the bit at colors index in colors is set
@param: const colors_t colors, const size_t colors_id
@return: bool
**/
bool colors_is_in(const colors_t colors, const size_t colors_id);

/**
@brief: checks if there's only one bit set
@param: const colors_t colors
@return: bool
**/
bool colors_is_singleton(const colors_t colors);

/**
@brief: checks if colors1 is a subset of colors2
@param: const colors_t colors1, const colors_t colors2
@return: bool
**/
bool colors_is_subset(const colors_t colors1, const colors_t colors2);

/**
@brief: sets the bit at position colors_id
@param: const colors_t colors, const size_t colors_id
@return: colors_t
**/
colors_t colors_add(const colors_t colors, const size_t colors_id);

/**
@brief: sets bits if they are both set
@param: const colors_t colors1, const colors_t colors2
@return: colors_t
**/
colors_t colors_and(const colors_t colors1, const colors_t colors2);

/**
@brief: discard a color from an existing colors_t
@param: const colors_t colors, const size_t colors_id
@return: colors_t
**/
colors_t colors_discard(const colors_t colors, const size_t colors_id);

/**
@brief: return 0
@param: void
@return: 0ULL
**/
colors_t colors_empty(void);

/**
@brief: sets all bits
@param:  size_t size
@return: colors_t
**/
colors_t colors_full(const size_t size);

/**
@brief: returns the leftmost bit
@param:  const colors_t colors
@return: colors_t
**/
colors_t colors_leftmost(const colors_t colors);

/**
@brief: negates all bits
@param: const colors_t colors
@return: colors_t
**/
colors_t colors_negate(const colors_t colors);

/**
@brief: return bits if they are either set in colors1 or colors2
@param: const colors_t colors1, const colors_t colors2
@return: colors_t
**/
colors_t colors_or(const colors_t colors1, const colors_t colors2);

/* Here we suppose that the seed has already been initialized in the main */
/**
@brief: return a 'randomized' color
@param: const colors_t colors
@return: colors_t
**/
colors_t colors_random(const colors_t colors);

/**
@brief: returns rightmost bit
@param: const colors_t colors
@return: colors_t
**/
colors_t colors_rightmost(const colors_t colors);

/**
@brief:  sets a color at position color_id
@param: const size_t color_id
@return: colors_t
**/
colors_t colors_set(const size_t color_id);

/**
@brief: subtraction between two colors_t
@param: const colors_t colors1, const colors_t colors2
@return: colors_t
**/
colors_t colors_subtract(const colors_t colors1, const colors_t colors2);

/**
@brief: exclusive union of two colors
@param: const colors_t colors1, const colors_t colors
@return: colors_t
**/
colors_t colors_xor(const colors_t colors1, const colors_t colors2);

/**
@brief: returns the number of colors enclosed in the set
@param: const colors_t colors
@return: size_t
**/
size_t colors_count(const colors_t colors);

/* Heuristics and subgrid functions */

/**
@brief: applies cross hatching heuristic to given sudoku subgrid
@param: colors_t *subgrid[], const size_t size
@return: bool
**/
bool cross_hatching_heuristic(colors_t *subgrid[], size_t size);

/**
@brief: applies lone number heuristic to a given sudoku subgrid
@param: colors_t *subgrid[], const size_t size
@return: bool
**/
bool lone_number_heuristic(colors_t *subgrid[], size_t size);

/**
@brief: applies naked heuristic to a given sudoku subgrid
@param: colors_t *subgrid[], const size_t size
@return: bool
**/
bool naked_subset_heuristic(colors_t *subgrid[], size_t size);

/**
@brief: applies hidden subset heuristic to given sudoku subgrid
@param: colors_t *subgrid[], const size_t size
@return: bool
**/
bool hidden_subset_heuristic(colors_t *subgrid[], size_t size);

/**
@brief: checks if the subgrids are consistent
@param: colors_t subgrid[], const size_t size
@return: bool
**/
bool subgrid_consistency(colors_t subgrid[], const size_t size);

/**
@brief: applies heuristics to given sudoku subgrid
@param: colors_t *subgrid[], const size_t size
@return: bool
**/
bool subgrid_heuristics(colors_t *subgrid[], size_t size);

#endif /* COLORS_H */
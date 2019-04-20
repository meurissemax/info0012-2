/*
 * File: array.h
 * Authors: Maxime Meurisse & Valentin Vermeylen
 *
 * This library allows the manipulation of arrays, and more specifically
 * the manipulation of two-dimensional arrays implemented in one-dimensional
 * arrays.
 */

#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
 * This function dynamically allocates an array of long.
 *
 * Parameter(s)
 * ------------
 * size: the size of the array
 *
 * Return
 * ------
 * An pointer to an array of size *size*.
 */
long* array_create(size_t size);

/*
 * This function is used to define a value in a 2D array implemented in
 * a 1D array.
 *
 * Parameter(s)
 * ------------
 * array: the array to modify
 * width: the width of the theoretical 2D array
 * line: the theoretical line of the element to set
 * column: the theoretical column of the element to set
 * v: the value of the element
 */
void array_set(long* array, size_t width, size_t line, size_t column, long v);

/*
 * This function is used to get a value in a 2D array implemented in
 * a 1D array.
 *
 * Parameter(s)
 * ------------
 * array: the array
 * width: the width of the theoretical 2D array
 * line: the theoretical line of the element to get
 * column: the theoretical column of the element to get
 */
long array_get(long* array, size_t width, size_t line, size_t column);

/*
 * This function frees the space allocated to the array.
 *
 * Parameter(s)
 * ------------
 * array: the array to free
 */
void array_free(long* array);

/*
 * This function is used to display the elements of the array in
 * the console.
 *
 * Parameter(s)
 * ------------
 * array: the array to print
 * size: the size of the array
 */
void array_print(long* array, size_t size);

/*
 * This function returns the size of array 1D corresponds to the
 * desired 2D array.
 *
 * Parameter(s)
 * ------------
 * line: the number of lines in the 2D array
 * column: the number of columns in the 2D array
 *
 * Return
 * ------
 * The size of the corresponding 1D array.
 */
size_t get_size(size_t line, size_t column);

/*
 * This function returns the position of an element according to its
 * theoretical position in the 2D array.
 *
 * Parameter(s)
 * ------------
 * width: the width of the theoretical 2D array
 * line: the theoretical line of the element
 * column: the theoretical column of the element
 *
 * Return
 * ------
 * The position of the element in the 1D array.
 */
size_t get_index(size_t width, size_t line, size_t column);

#endif

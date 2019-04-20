/*
 * File: array.c
 * Authors: Maxime Meurisse & Valentin Vermeylen
 *
 * This library allows the manipulation of arrays, and more specifically
 * the manipulation of two-dimensional arrays implemented in one-dimensional
 * arrays.
 */

#include "headers/array.h"

long* array_create(size_t size) {
    assert(size > 0);

    long* array;

    array = (long*)malloc(size * sizeof(long));

    if(array == NULL)
        return NULL;

    return array;
}

void array_set(long* array, size_t width, size_t line, size_t column, long v) {
    assert(array != NULL);
    assert(width > 0);

    array[get_index(width, line, column)] = v;
}

long array_get(long* array, size_t width, size_t line, size_t column) {
    assert(array != NULL);
    assert(width > 0);

    return array[get_index(width, line, column)];
}

void array_free(long* array) {
    assert(array != NULL);

    free(array);
    array = NULL;
}

void array_print(long* array, size_t size) {
    assert(array != NULL);
    assert(size > 0);

    size_t i;

    for(i = 0; i < size; i++)
        printf("%ld ", array[i]);

    printf("\n");
}

size_t get_size(size_t line, size_t column) {
    assert(line > 0);
    assert(column > 0);

    return line * column;
}

size_t get_index(size_t width, size_t line, size_t column) {
    assert(width > 0);

    return width * line + column;
}

#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_array_t.h>

/** Prints info of the wait array. */
void sync_array_print_info(FILE *file, /*!< in: file where to print */
                                  sync_array_t *arr); /*!< in: wait array */

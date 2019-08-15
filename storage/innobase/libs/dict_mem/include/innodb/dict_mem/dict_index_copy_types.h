#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;
struct dict_index_t;

/** Copies types of fields contained in index to tuple. */
void dict_index_copy_types(dtuple_t *tuple,           /*!< in/out: data tuple */
                           const dict_index_t *index, /*!< in: index */
                           ulint n_fields);           /*!< in: number of
                                                      field types to copy */

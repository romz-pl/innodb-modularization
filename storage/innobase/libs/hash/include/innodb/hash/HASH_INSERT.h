#pragma once

#include <innodb/univ/univ.h>

#include <innodb/hash/HASH_ASSERT_OWN.h>
#include <innodb/hash/hash_cell_t.h>
#include <innodb/hash/hash_get_nth_cell.h>
#include <innodb/hash/hash_calc_hash.h>

/** Inserts a struct to a hash table. */

#define HASH_INSERT(TYPE, NAME, TABLE, FOLD, DATA)                    \
  do {                                                                \
    hash_cell_t *cell3333;                                            \
    TYPE *struct3333;                                                 \
                                                                      \
    HASH_ASSERT_OWN(TABLE, FOLD)                                      \
                                                                      \
    (DATA)->NAME = NULL;                                              \
                                                                      \
    cell3333 = hash_get_nth_cell(TABLE, hash_calc_hash(FOLD, TABLE)); \
                                                                      \
    if (cell3333->node == NULL) {                                     \
      cell3333->node = DATA;                                          \
    } else {                                                          \
      struct3333 = (TYPE *)cell3333->node;                            \
                                                                      \
      while (struct3333->NAME != NULL) {                              \
        struct3333 = (TYPE *)struct3333->NAME;                        \
      }                                                               \
                                                                      \
      struct3333->NAME = DATA;                                        \
    }                                                                 \
  } while (0)

#pragma once

#include <innodb/univ/univ.h>

#include <innodb/hash/hash_get_nth_cell.h>
#include <innodb/hash/HASH_ASSERT_VALID.h>
#include <innodb/hash/hash_cell_t.h>
#include <innodb/hash/hash_calc_hash.h>

#define HASH_DELETE(TYPE, NAME, TABLE, FOLD, DATA)                    \
  do {                                                                \
    hash_cell_t *cell3333;                                            \
    TYPE *struct3333;                                                 \
                                                                      \
    HASH_ASSERT_OWN(TABLE, FOLD)                                      \
                                                                      \
    cell3333 = hash_get_nth_cell(TABLE, hash_calc_hash(FOLD, TABLE)); \
                                                                      \
    if (cell3333->node == DATA) {                                     \
      HASH_ASSERT_VALID(DATA->NAME);                                  \
      cell3333->node = DATA->NAME;                                    \
    } else {                                                          \
      struct3333 = (TYPE *)cell3333->node;                            \
                                                                      \
      while (struct3333->NAME != DATA) {                              \
        struct3333 = (TYPE *)struct3333->NAME;                        \
        ut_a(struct3333);                                             \
      }                                                               \
                                                                      \
      struct3333->NAME = DATA->NAME;                                  \
    }                                                                 \
    HASH_INVALIDATE(DATA, NAME);                                      \
  } while (0)

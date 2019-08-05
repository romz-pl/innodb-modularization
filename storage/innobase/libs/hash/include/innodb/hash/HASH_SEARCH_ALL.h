#pragma once

#include <innodb/univ/univ.h>

#include <innodb/hash/HASH_GET_FIRST.h>
#include <innodb/hash/HASH_ASSERT_VALID.h>
#include <innodb/hash/HASH_GET_NEXT.h>

/** Looks for an item in all hash buckets. */
#define HASH_SEARCH_ALL(NAME, TABLE, TYPE, DATA, ASSERTION, TEST) \
  do {                                                            \
    ulint i3333;                                                  \
                                                                  \
    for (i3333 = (TABLE)->n_cells; i3333--;) {                    \
      (DATA) = (TYPE)HASH_GET_FIRST(TABLE, i3333);                \
                                                                  \
      while ((DATA) != NULL) {                                    \
        HASH_ASSERT_VALID(DATA);                                  \
        ASSERTION;                                                \
                                                                  \
        if (TEST) {                                               \
          break;                                                  \
        }                                                         \
                                                                  \
        (DATA) = (TYPE)HASH_GET_NEXT(NAME, DATA);                 \
      }                                                           \
                                                                  \
      if ((DATA) != NULL) {                                       \
        break;                                                    \
      }                                                           \
    }                                                             \
  } while (0)

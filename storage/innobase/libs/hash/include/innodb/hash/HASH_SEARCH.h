#pragma once

#include <innodb/univ/univ.h>

#include <innodb/hash/HASH_ASSERT_OWN.h>
#include <innodb/hash/HASH_GET_FIRST.h>
#include <innodb/hash/hash_calc_hash.h>
#include <innodb/hash/HASH_ASSERT_VALID.h>
#include <innodb/hash/HASH_GET_NEXT.h>

/** Looks for a struct in a hash table. */
#define HASH_SEARCH(NAME, TABLE, FOLD, TYPE, DATA, ASSERTION, TEST)    \
  {                                                                    \
    HASH_ASSERT_OWN(TABLE, FOLD)                                       \
                                                                       \
    (DATA) = (TYPE)HASH_GET_FIRST(TABLE, hash_calc_hash(FOLD, TABLE)); \
    HASH_ASSERT_VALID(DATA);                                           \
                                                                       \
    while ((DATA) != NULL) {                                           \
      ASSERTION;                                                       \
      if (TEST) {                                                      \
        break;                                                         \
      } else {                                                         \
        HASH_ASSERT_VALID(HASH_GET_NEXT(NAME, DATA));                  \
        (DATA) = (TYPE)HASH_GET_NEXT(NAME, DATA);                      \
      }                                                                \
    }                                                                  \
  }
